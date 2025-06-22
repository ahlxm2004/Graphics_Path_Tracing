#include <random>
#include <Vector3f.h>
#include "ray.hpp"
#include "scene_parser.hpp"
#include "light.hpp"

Vector3f tracingMC(Ray ray, SceneParser &Parser, std::mt19937_64 &rnd, AreaLight *&pLight);
Vector3f tracingMC(int x, int y, SceneParser &Parser);
