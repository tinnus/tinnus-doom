
#pragma once

#include "Vectors.h"
#include "Renderer.h"

// NOTE: native pixel format is the same as framebuffer (ABGR)
class Texture
{
public:
	Texture(int width, int height);

	int GetWidth() const;
	int GetHeight() const;

	Pixel* GetData() const;
	void GetPixel(int x, int y, Pixel& result) const;
	void Sample(float u, float v, Pixel& result);

private:
	int _width, _height;
	Pixel* _textureData;
};

