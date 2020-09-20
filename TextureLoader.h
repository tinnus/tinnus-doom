
#pragma once

class Texture;

class TextureLoader
{
public:
	TextureLoader();
	Texture* LoadFromFile(const char* filename);
};

