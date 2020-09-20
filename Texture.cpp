
#include "Texture.h"

Texture::Texture(int width, int height)
{
	_width = width;
	_height = height;
	_textureData = new Pixel[width * height];
}

int Texture::GetWidth() const
{
	return _width;
}

int Texture::GetHeight() const
{
	return _height;
}

Pixel* Texture::GetData() const
{
	return _textureData;
}

void Texture::GetPixel(int x, int y, Pixel& result) const
{
	result = _textureData[y * _width + x];
}

void Texture::Sample(float u, float v, Pixel& result)
{
    int tex_x = u * _width;
    int tex_y = v * _height;

    // This is a convoluted, but fast, way to make the UV repeat in both directions.
    // Only works if texture size is power of 2!
    tex_x = (tex_x + 0x1000) & (_width - 1);
    tex_y = (tex_y + 0x1000) & (_height - 1);

    GetPixel(tex_x, tex_y, result);
}
