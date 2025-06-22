/*
原创性：独立实现
*/

#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

class Sphere : public Object3D {
public:
    Sphere() {
        // unit ball at the center
        center = Vector3f(0., 0., 0.);
        radius = 1;
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        this->center = center;
        this->radius = radius;
        this->squaredRadius = radius * radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        Vector3f o = r.pointAtParameter(tmin);
        Vector3f l = center - o;
        float t = Vector3f::dot(l, r.getDirection());
        float d = l.squaredLength() - t * t;
        bool isFront = (l.squaredLength() > squaredRadius);
        if (isFront) {
            if (t < 0 || d > squaredRadius) return false;
            t -= sqrtf(squaredRadius - d);
        }
        else t += sqrtf(squaredRadius - d);
        t += tmin;
        if (t > h.getT()) return false;
        Vector3f p = r.pointAtParameter(t);
        Vector3f normal = (p - center).normalized();
        h.set(t, material, isFront ? normal : -normal,
            material->useTexture() && isFront ? material->getColor(
            atan2f(p[1] - center[1], p[0] - center[0]) / (2 * M_PI) + 0.5,
            (p[2] - center[2]) / (2 * radius) + 0.5) : material->getColor(),
            isFront, Vector3f::ZERO);
        return true;
    }

private:
    Vector3f center;
    float radius, squaredRadius;
};


#endif
