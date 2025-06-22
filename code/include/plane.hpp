#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// function: ax+by+cz=d

class Plane : public Object3D {
public:
    Plane() {

    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->normal = normal;
        this->d = d;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float t = Vector3f::dot(r.getDirection(), normal);
        int sign = (t > 0 ? 1 : -1);
        if (fabsf(t) < 1e-8) return false;
        t = (this->d - Vector3f::dot(r.getOrigin(), normal)) / t;
        if (t < tmin || t > h.getT()) return false;
        h = Hit(t, material, -normal * sign, material->getColor(), true, Vector3f::ZERO);
        return true;
    }

private:
    Vector3f normal;
    float d;
};

#endif //PLANE_H
		

