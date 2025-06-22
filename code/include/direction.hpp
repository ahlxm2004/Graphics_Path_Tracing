#ifndef DIRECTION_H
#define DIRECTION_H

#include <vecmath.h>
#include <random>

Vector3f getReflectDir(Vector3f incident, Vector3f normal);

void getRefractDir(Vector3f incident, Vector3f normal, bool isFront, float n, Vector3f &direction, float &weight);

Vector3f uniformHemisphere(std::mt19937_64 &rnd); // x^2 + y^2 + z^2 = 1, z > 0

Vector3f cosWeightedHemisphere(std::mt19937_64 &rnd);

Vector3f rotate(Vector3f base, Vector3f normal);

#endif // DIRECTION_H
