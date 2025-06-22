#include <Vector3f.h>
#include "ray.hpp"
#include "scene_parser.hpp"

Vector3f tracingWhitted(Ray ray, SceneParser &Parser, Vector3f rate);
Vector3f tracingWhitted(int x, int y, SceneParser &Parser);
