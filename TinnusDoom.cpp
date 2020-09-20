// TinnusDoom.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <tuple>
#include <algorithm>

#include "SDL.h"
#include "SDL_image.h"

#include "Vectors.h"

using namespace std;

struct Pixel
{
    Uint8 R, G, B, A;
};

const int frameWidth = 960; // 240;
const int frameHeight = 600;
Pixel framebuffer[frameWidth * frameHeight];

const int windowWidth = 960;
const int windowHeight = 600;

SDL_Surface* wallTexture;

void sample_texture(SDL_Surface* texture, float u, float v, Pixel& result)
{
    //result.R = result.A = 255;
    //result.G = result.B = 0;
    //return;

    // This is a convoluted
    int tex_x = (u * texture->w);
    int tex_y = (1 - v) * texture->h;

    // This is a convoluted, but fast, way to make the UV repeat.
    tex_x = (tex_x + 0x1000) & (texture->w - 1);
    tex_y = (tex_y + 0x1000) & (texture->h - 1);

    // TODO: optimize this
    auto* linePtr = ((Uint8*)texture->pixels + tex_y * texture->pitch);
    auto pixel = *((Uint32*)(linePtr + wallTexture->format->BytesPerPixel * tex_x));
    SDL_GetRGBA(pixel, wallTexture->format, &result.R, &result.G, &result.B, &result.A);
}

struct Wall
{
    Vec2 A, B;
    float MinHeight, MaxHeight;
    SDL_Surface* Texture;
    float Length;

    //Wall() 
    //: Texture(nullptr), MinHeight(0), MaxHeight(0) {}

    Wall(Vec2 a, Vec2 b, float minHeight, float maxHeight, SDL_Surface* texture)
        : A(a), B(b), Texture(texture), MinHeight(minHeight), MaxHeight(maxHeight)
    { 
        Length = (A - B).Length();
    }
};

//Wall wall1;
vector<Wall> mapWalls;
vector<Wall> transformedWalls;

Vec2 cameraPos(0, 0);
Vec2 cameraDirection(0, 1);
float cameraHeight = 0.6f; // 1.6f;

float cameraAngle = 0;

#define PI 3.14159f
float cameraFovH = 60 * PI / 180; // 90 deg

bool wall_intersect(Vec2 origin, Vec2 direction, const Wall& wall, float& outPosition)
{
    Vec2 q = wall.A;
    Vec2 s = (wall.B - wall.A);
    Vec2 p = origin;
    Vec2 r = direction;

    float num = Vec2::Cross(q - p, r);
    float denom = Vec2::Cross(r, s);

    if (denom == 0) return false;
    float result = num / denom;
    if (result > 0 && result < 1)
    {
        outPosition = result;
        return true;
    }

    return false;
}

bool wall_intersect_from_origin(Vec2 direction, const Wall& wall, float& outPosition)
{
    Vec2 q = wall.A;
    Vec2 s = (wall.B - wall.A);
    Vec2 r = direction;

    float denom = Vec2::Cross(r, s);
    if (denom == 0) return false;

    float num = Vec2::Cross(q, r);
    float result = num / denom;
    if (result > 0 && result < 1)
    {
        outPosition = result;
        return true;
    }

    return false;
}

Vec2 transform_point(const Vec2& point, const Vec2& forward) // NOTE: assumes forward is normalized!
{
    Vec2 right = Vec2(forward.Y, -forward.X);

    Vec2 result;
    result.X = Vec2::Dot(point, right);
    result.Y = Vec2::Dot(point, forward);
    return result;
}

void trace_column(int column, const Vec2& traceDir, float cameraHeight, float cameraTanHalfFov)
{
    // trace into all walls and order according to distance to hit
    static vector<tuple<Wall*, float, float>> orderedWalls;
    orderedWalls.clear();
    for (auto& transformedWall : transformedWalls)
    {
        float wallPosition;
        if (wall_intersect_from_origin(traceDir, transformedWall, wallPosition))
        {
            // get intersection point in 2D (only depth matters)
            float hitDistance = Lerp(transformedWall.A.Y, transformedWall.B.Y, wallPosition);
            orderedWalls.push_back(make_tuple(&transformedWall, hitDistance, wallPosition));
        }
    }
    sort(orderedWalls.begin(), orderedWalls.end(), [](const auto& lhs, const auto& rhs) {
        return get<1>(lhs) < get<1>(rhs);
    });

    //printf("Drawing %d walls for column %d\n", orderedWalls.size(), column);

    // draw walls front-to-back
    for (auto& wallAndHitDist : orderedWalls)
    {
        auto& transformedWall = *get<0>(wallAndHitDist);
        float hitDistance = get<1>(wallAndHitDist);
        float wallPosition = get<2>(wallAndHitDist);

        // figure out min and max screen positions
        float halfHeight = hitDistance * cameraTanHalfFov;
        float wallMaxNormalized = 0.5f + (transformedWall.MaxHeight - cameraHeight) / halfHeight;
        float wallMinNormalized = 0.5f - (cameraHeight - transformedWall.MinHeight) / halfHeight;

        int miny = int(wallMinNormalized * frameHeight);
        int maxy = int(wallMaxNormalized * frameHeight);

        // trace wall column
        float u = wallPosition * transformedWall.Length;
        int traceMinY = fmax(0, miny);
        int traceMaxY = fmin(frameHeight, maxy);

        Pixel* frameBufferPtr = &framebuffer[(frameHeight - traceMaxY - 1) * frameWidth + column];
        float v = InverseLerp(miny, maxy, traceMaxY);
        float vGrad = 1.0f / (maxy - miny);
        
        // NOTE: "up" in the world is actually further into the framebuffer, so we iterate from the top to the bottom,
        // to keep the framebuffer and texture iteration in memory order. This seems to me slightly better for performance.
        for (int y = traceMaxY - 1; y >= traceMinY; y--)
        {
            // flip y horizontally, since the screen is drawn top to bottom and we're counting bottom to top
            Pixel& color = framebuffer[(frameHeight - y - 1) * frameWidth + column];
            //Pixel& color = *frameBufferPtr;
            
            // ignore already drawn pixels
            if (color.A != 0) continue;

            // calculate texture V coordinate
            v = InverseLerp(miny, maxy, y);

            sample_texture(transformedWall.Texture, u, v, color); // *frameBufferPtr);

            // Not using this for now. Bugged for some reason.
            //frameBufferPtr -= frameWidth;
            //v -= vGrad;
        }
    }
}

void draw_walls(const Vec2& cameraPos, const Vec2& cameraDirection, float cameraHeight, float cameraFovH)
{
    const float nearDist = 1;
    const float halfFov = cameraFovH / 2;
    const float nearWidth = 2 * nearDist * tan(halfFov);
    const float xStep = nearWidth / frameWidth;

    const float screenAspect = float(frameWidth) / frameHeight;
    float cameraFovV = 2 * atan(tan(cameraFovH / 2) / screenAspect);
    float halfFovV = cameraFovV / 2;
    float tanHalfFov = tan(halfFov);

    // transform walls
    transformedWalls.clear();
    for (auto& wall : mapWalls)
    {
        Wall transformedWall = wall;
        transformedWall.A -= cameraPos;
        transformedWall.B -= cameraPos;
        transformedWall.A = transform_point(transformedWall.A, cameraDirection);
        transformedWall.B = transform_point(transformedWall.B, cameraDirection);
        transformedWalls.push_back(transformedWall);
    }

    Vec2 nearPoint(-nearWidth / 2, nearDist);
    for (int x = 0; x < frameWidth; x++, nearPoint.X += xStep)
    {
        //Vec2 nearPoint(x * xStep - nearWidth / 2, nearDist); // Moved this to be incremented only
        Vec2 traceDir = nearPoint;

        trace_column(x, traceDir, cameraHeight, tanHalfFov);
    }
}

void trace_wall(const Wall& wall, const Vec2& cameraPos, const Vec2& cameraDirection, float cameraHeight, float cameraFovH)
{
    const float nearDist = 1;
    const float halfFov = cameraFovH / 2;
    const float nearWidth = 2 * nearDist * tan(halfFov);
    const float xStep = nearWidth / frameWidth;

    const float screenAspect = float(frameWidth) / frameHeight;
    float cameraFovV = 2 * atan(tan(cameraFovH / 2) / screenAspect);
    float halfFovV = cameraFovV / 2;

    Wall transformedWall = wall;
    transformedWall.A -= cameraPos;
    transformedWall.B -= cameraPos;
    transformedWall.A = transform_point(transformedWall.A, cameraDirection);
    transformedWall.B = transform_point(transformedWall.B, cameraDirection);

    Vec2 zero(0, 0);
    for (int x = 0; x < frameWidth; x++)
    {
        Vec2 nearPoint(x * xStep - nearWidth / 2, nearDist);
        Vec2 traceDir = nearPoint;

        float wallPosition;
        if (wall_intersect(zero, traceDir, transformedWall, wallPosition))
        {
            // get intersection point in 2D
            Vec2 hitPoint = Lerp(transformedWall.A, transformedWall.B, wallPosition);
            float hitDistance = hitPoint.Y; // Length();

            // figure out min and max screen positions
            float halfHeight = hitDistance * tan(halfFovV);
            float wallMaxNormalized = 0.5f + (transformedWall.MaxHeight - cameraHeight) / halfHeight;
            float wallMinNormalized = 0.5f - (cameraHeight - transformedWall.MinHeight) / halfHeight;

            int miny = round(wallMinNormalized * frameHeight);
            int maxy = round(wallMaxNormalized * frameHeight);

            // trace wall column
            float u = wallPosition;
            int traceMinY = fmax(0, miny);
            int traceMaxY = fmin(frameHeight, maxy);
            for (int y = traceMinY; y < traceMaxY; y++)
            {
                if (y < 0 || y >= frameHeight) continue;

                // calculate texture V coordinate
                float v = InverseLerp(miny, maxy, y);

                // flip y horizontally, since the screen is drawn top to bottom and we're counting bottom to top
                Pixel& color = framebuffer[(frameHeight - y - 1) * frameWidth + x];
                sample_texture(transformedWall.Texture, u, v, color);
            }
        }
    }
}

void draw_texture_rotated(SDL_Surface* texture, const Vec2& origin, const Vec2& axis1, const Vec2& axis2, const int& x, const int& y)
{
    Vec2 point(x, y);
    point.X /= frameWidth;
    point.Y /= frameHeight;

    Vec2 originToPoint = point - origin;
    float u = Vec2::Dot(originToPoint, axis1) / axis1.SqrLength(); // originToPoint.Length()
    float v = Vec2::Dot(originToPoint, axis2) / axis2.SqrLength();

    if (u < 0 || v < 0 || u > 1 || v > 1) return;

    Pixel& color = framebuffer[y * frameWidth + x];
    sample_texture(texture, u, v, color);
}

void clear()
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

void draw()
{
    draw_walls(cameraPos, cameraDirection, cameraHeight, cameraFovH);

    /*Vec2 wall_00(0.2f, 0.3f);
    Vec2 wall_01(0.3f, 0.9f);
    Vec2 wall_10(0.8f, 0.2f);

    Vec2 origin = wall_00;
    Vec2 axis1 = wall_10 - wall_00;
    Vec2 axis2 = wall_01 - wall_00;*/

    /*for(auto& wall : mapWalls)
    {
        trace_wall(wall, cameraPos, cameraDirection, cameraHeight, cameraFovH);
    }*/

    /*for (int y = 0; y < frameHeight; y++)
    {
        for (int x = 0; x < frameWidth; x++)
        {
            //draw_texture_rotated(wallTexture, origin, axis1, axis2, x, y);
            trace_wall(wall1, cameraPos, cameraDirection, cameraHeight, cameraFovH);
        }
    }*/
}

void increment_camera_angle(float delta)
{
    cameraAngle += delta;
    cameraDirection.X = -sin(cameraAngle);
    cameraDirection.Y = cos(cameraAngle);
}

int main(int argc, char** argv)
{
    std::cout << "Hello World!\n";

    SDL_Window* sdlWindow = SDL_CreateWindow("TinnusDoom",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowWidth, windowHeight,
        0);
    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);

    auto* sdlTexture = SDL_CreateTexture(sdlRenderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        frameWidth, frameHeight);

    // load images
    IMG_Init(IMG_INIT_PNG);
    wallTexture = IMG_Load("Assets/wall_64.png");

    printf("Read %d x %d pixels bpp %d pitch %d\n", wallTexture->w, wallTexture->h, wallTexture->format->BytesPerPixel, wallTexture->pitch);

    /*Vec2 test;
    test.X = Fixed::FromFloat(1.23f);
    test.Y = Fixed::FromFloat(31995.005f);
    printf("Fixed point test: %.5f / %.5f internal: %d / %d\n", test.X.ToFloat(), test.Y.ToFloat(), test.X.Value, test.Y.Value);*/

    // Init map
    mapWalls.push_back(Wall(Vec2(-3, 1), Vec2(-1, 3), 0, 0.7f, wallTexture));
    mapWalls.push_back(Wall(Vec2(-1, 3), Vec2(1, 3), 0, 1, wallTexture));
    mapWalls.push_back(Wall(Vec2(1, 3), Vec2(3, 1), 0, 0.7f, wallTexture));
    mapWalls.push_back(Wall(Vec2(-4, 2), Vec2(4, 2), -0.4, 0.3f, wallTexture));

    float cameraRotateSpeed = 120 * PI / 180;
    float cameraMoveSpeed = 3.8f;

    Vec2 moveInput;
    Vec2 rotateInput;
    
    bool quit = false;
    unsigned int lastTime = SDL_GetTicks(), currentTime;
    while (!quit)
    {
        // update timers
        currentTime = SDL_GetTicks();
        float deltaTime = 0.001f * (currentTime - lastTime);
        lastTime = currentTime;

        printf("%8.3f FPS\n", 1/deltaTime);

        SDL_PumpEvents();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if (event.key.repeat) continue;

                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    rotateInput.X++;
                    break;
                case SDLK_RIGHT:
                    rotateInput.X--;
                    break;

                case SDLK_w:
                    moveInput.Y++;
                    break;
                case SDLK_s:
                    moveInput.Y--;
                    break;

                case SDLK_d:
                    moveInput.X++;
                    break;
                case SDLK_a:
                    moveInput.X--;
                    break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    rotateInput.X--;
                    break;
                case SDLK_RIGHT:
                    rotateInput.X++;
                    break;

                case SDLK_w:
                    moveInput.Y--;
                    break;
                case SDLK_s:
                    moveInput.Y++;
                    break;

                case SDLK_d:
                    moveInput.X--;
                    break;
                case SDLK_a:
                    moveInput.X++;
                    break;
                }
                break;
            }
        }

        // update input
        if (rotateInput.X != 0)
        {
            increment_camera_angle(rotateInput.X * cameraRotateSpeed * deltaTime);
        }

        Vec2 cameraRight = Vec2(cameraDirection.Y, -cameraDirection.X);
        if (moveInput.X != 0)
        {
            cameraPos += moveInput.X * cameraRight * cameraMoveSpeed * deltaTime;
        }
        if (moveInput.Y != 0)
        {
            cameraPos += moveInput.Y * cameraDirection * cameraMoveSpeed * deltaTime;
        }

        clear();
        draw();

        //SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 20, 255);
        //SDL_RenderClear(sdlRenderer);

        SDL_UpdateTexture(sdlTexture, nullptr, framebuffer, frameWidth * sizeof(Uint32));
        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);

        SDL_RenderPresent(sdlRenderer);
    }

    return 0;
}
