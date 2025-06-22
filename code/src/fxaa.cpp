/*
原创性：参考已有代码
https://catlikecoding.com/unity/tutorials/advanced-rendering/fxaa/
*/

#include "fxaa.hpp"
#include <vector>

namespace {
	float smoothstep(float t) {
		if (t > 1) return 1;
		return t * t * (3 - 2 * t);
	}
}

void fxaa(Image &image) {
	const float absoluteThreshold = 0.0833;
	const float relativeThreshold = 0.166;
	const float gradientThreshold = 0.25;
	const int searchSteps = 10;
	const int guessSteps = 8;

	image.clamp();
	int W = image.Width(), H = image.Height();
	Image image0 = image;
	std::vector <std::vector <float> > L(W, std::vector <float> (H));

	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++) {
			L[y][x] =
				0.213 * image.GetPixel(x, y)[0] +
				0.715 * image.GetPixel(x, y)[1] +
				0.072 * image.GetPixel(x, y)[2];
		}
	for (int y = 1; y + 1 < H; y++)
		for (int x = 1; x + 1 < W; x++) {
			float o = L[y][x], u = L[y - 1][x], d = L[y + 1][x], l = L[y][x - 1], r = L[y][x + 1];
			float ul = L[y - 1][x - 1], ur = L[y - 1][x + 1], dl = L[y + 1][x - 1], dr = L[y + 1][x + 1];
			float maxL = std::max(o, std::max(std::max(u, d), std::max(l, r)));
			float minL = std::min(o, std::min(std::min(u, d), std::min(l, r)));
			if (maxL - minL < std::max(absoluteThreshold, relativeThreshold * maxL)) continue;
			float horizontal = fabsf(u + d - 2 * o) + fabsf(ul + dl - 2 * l) + fabsf(ur + dr - 2 * r);
			float vertical = fabsf(l + r - 2 * o) + fabsf(ul + ur - 2 * u) + fabsf(dl + dr - 2 * d);
			int dx = 0, dy = 0;
			if (horizontal > vertical)
				dy = (fabsf(u - o) > fabsf(d - o) ? -1 : 1);
			else
				dx = (fabsf(l - o) > fabsf(r - o) ? -1 : 1);
			float filter = (2 * (u + d + l + r) + 1 * (ul + ur + dl + dr)) / 12;
			filter = fabsf(filter - o) / (maxL - minL);
			float pixelBlend = smoothstep(filter);
			pixelBlend *= pixelBlend;
			int sx = (dx == 0), sy = (dy == 0);
			float edgeL = (L[y][x] + L[y + dy][x + dx]) / 2, edgeP = edgeL, edgeN = edgeL;
			int Pstep = guessSteps, Nstep = guessSteps;
			for (int i = 1, x0 = x, y0 = y; i <= searchSteps; i++) {
				x0 += sx, y0 += sy;
				if (x0 == 0 || x0 + 1 == W || y0 == 0 || y0 + 1 == W) {
					Pstep = i - 1; break;
				}
				edgeP = (L[y0][x0] + L[y0 + dy][x0 + dx]) / 2;
				if (fabs(edgeP - edgeL) > edgeL * gradientThreshold) {
					Pstep = i; break;
				}
			}
			for (int i = 1, x0 = x, y0 = y; i <= searchSteps; i++) {
				x0 -= sx, y0 -= sy;
				if (x0 == 0 || x0 + 1 == W || y0 == 0 || y0 + 1 == W) {
					Nstep = i - 1; break;
				}
				edgeN = (L[y0][x0] + L[y0 + dy][x0 + dx]) / 2;
				if (fabs(edgeN - edgeL) > edgeL * gradientThreshold) {
					Nstep = i; break;
				}
			}
			float edgeBlend = 0;
			if (Pstep < Nstep && (L[y + dy][x + dx] > L[y][x]) == (edgeP > edgeL))
				edgeBlend = 0.5 - (float)Pstep / (Pstep + Nstep);
			if (Nstep < Pstep && (L[y + dy][x + dx] > L[y][x]) == (edgeN > edgeL))
				edgeBlend = 0.5 - (float)Nstep / (Pstep + Nstep);

			float blend = std::max(pixelBlend, edgeBlend);
			image.SetPixel(x, y,
				image0.GetPixel(x, y) * (1 - blend) +
				image0.GetPixel(x + dx, y + dy) * blend);
		}
}
