# TinnusDoom

This is a just-for-fun personal project to implement a renderer (and, hopefully, later on, a game on top of it) in the same style as the original Doom.

SDL2 is used only for the final blit-to-screen and SDL2_image is used for reading textures from disk. All rendering is done "by hand" to a custom framebuffer in RAM.


## Dependencies

* SDL2
* SDL2_image

Needs the following files in the binary folder to run:
* SDL2.dll
* SDL2_image.dll
* libpng16-16.dll
* zlib1.dll