
#include "TextureLoader.h"

#include "SDL_image.h"

#include "Texture.h"

TextureLoader::TextureLoader()
{
    IMG_Init(IMG_INIT_PNG);
}

Texture* TextureLoader::LoadFromFile(const char* filename)
{
    auto* sdlSurface = IMG_Load(filename);
    auto* texture = new Texture(sdlSurface->w, sdlSurface->h);

    // copy data into native format
    Pixel* dstData = texture->GetData();
    for (int y = 0; y < texture->GetHeight(); y++)
    {
        // Invert texture in Y so that 0 = bottom
        int src_y = sdlSurface->h - y - 1;
        uint8* srcData = (uint8*)(sdlSurface->pixels) + src_y * sdlSurface->pitch;

        for (int x = 0; x < texture->GetWidth(); x++)
        {
            Uint32 srcPixel = *((Uint32*)(srcData + sdlSurface->format->BytesPerPixel * x));
            Pixel& dstPixel = dstData[x];
            SDL_GetRGBA(srcPixel, sdlSurface->format, &dstPixel.R, &dstPixel.G, &dstPixel.B, &dstPixel.A);
        }

        dstData += texture->GetWidth();
    }

    return texture;
}
