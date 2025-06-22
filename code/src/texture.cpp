/*
原创性：独立实现
*/

#include <vecmath.h>
#include "image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.hpp"

#include <unistd.h>

void Texture::set(const char *filename) {
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	int channels;
	unsigned char *data = stbi_load(filename, &width, &height, &channels, 0);
	image = new Image(width, height);
	for (int y = height - 1, idx = 0; y >= 0; y--)
        for (int x = 0; x < width; x++, idx += channels)
            image->SetPixel(x, y, Vector3f(data[idx], data[idx + 1], data[idx + 2]) / 256);
    printf("%s: %d x %d\n", filename, width, height);
}

Vector3f Texture::getColor(float u, float v) {
	if (u < 0) u = 0;
	if (u > 0.9999) u = 0.9999;
	if (v < 0) v = 0;
	if (v > 0.9999) v = 0.9999;
	u *= width, v *= height;
	int x = (int)u, y = (int)v;
	return image->GetPixel(x, y);
}

void Texture::gammaCorrection(float gamma) {
	image->gammaCorrection(gamma);
}
