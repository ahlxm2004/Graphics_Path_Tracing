#ifndef TEXTURE_H
#define TEXTURE_H

#include <vecmath.h>
#include "image.hpp"

class Texture {
public:
	void set(const char *filename);
	Vector3f getColor(float u, float v);
	void gammaCorrection(float gamma);

private:
	Image *image;
	int width, height;
};

#endif // TEXTURE_H
