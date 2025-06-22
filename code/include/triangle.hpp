/*
原创性：独立实现
*/

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>

class Triangle: public Object3D {

public:

	Triangle() = delete;

	Triangle(const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
		A = a, edge1 = b - a, edge2 = c - a;
		normal = Vector3f::cross(edge1, edge2).normalized();
	}

	bool intersect(const Ray& ray,  Hit& h, float tmin) override {
		Vector3f p = Vector3f::cross(ray.getDirection(), edge2);
		float a = Vector3f::dot(edge1, p);
		if (fabsf(a) < 1e-8) return false;
		float f = 1 / a;
		Vector3f s = ray.getOrigin() - A;
		float u = f * Vector3f::dot(s, p);
		if (u < 0 || u > 1) return false;
		Vector3f q = Vector3f::cross(s, edge1);
		float v = f * Vector3f::dot(ray.getDirection(), q);
		if (v < 0 || u + v > 1) return false;
		float t = f * Vector3f::dot(q, edge2);
		if (t < tmin || t > h.getT()) return false;
		float dot = Vector3f::dot(ray.getDirection(), normal);
		h.set(t, material, dot < 0 ? normal : -normal,
			material->getColor(), dot < 0, Vector3f::ZERO);
		return true;
	}

	Vector3f getWeight(const Ray& ray,  Hit& hit) {
		Vector3f h = Vector3f::cross(ray.getDirection(), edge2);
		float a = Vector3f::dot(edge1, h);
		float f = 1 / a;
		Vector3f s = ray.getOrigin() - A;
		float u = f * Vector3f::dot(s, h);
		Vector3f q = Vector3f::cross(s, edge1);
		float v = f * Vector3f::dot(ray.getDirection(), q);
		return Vector3f(1 - u - v, u, v);
	}

	Vector3f &getNormal() {return normal;}
	
	Vector3f vertices(int x) {
		if (x == 0) return A;
		if (x == 1) return A + edge1;
		return A + edge2;
	}

	float dmin(int x) {
		return A[x] + std::min((float)0, std::min(edge1[x], edge2[x]));
	}

	float dmax(int x) {
		return A[x] + std::max((float)0, std::max(edge1[x], edge2[x]));
	}

private:
	Vector3f normal;
	Vector3f A, edge1, edge2;
};

#endif //TRIANGLE_H
