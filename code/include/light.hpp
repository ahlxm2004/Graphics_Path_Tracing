/*
原创性：独立实现
*/

#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>
#include <random>
#include "object3d.hpp"

class Light {
public:
    Light() = default;

    virtual void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col, float &dist) const = 0;
};


class DirectionalLight : public Light {
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c) {
        direction = d.normalized();
        color = c;
    }

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col, float &dist) const override {
        dir = -direction;
        col = color;
        dist = 1e9;
    }

private:

    Vector3f direction;
    Vector3f color;

};

class PointLight : public Light {
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c) {
        position = p;
        color = c;
    }

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col, float &dist) const override {
        dir = position - p;
        dist = dir.length();
        dir = dir / dist;
        col = color;
    }

private:

    Vector3f position;
    Vector3f color;

};

class AreaLight : public Light {
public:
    virtual Vector3f sampling(std::mt19937_64 &rnd) = 0;
    virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;
    virtual Vector3f getColor() = 0;
    virtual float area() = 0;

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col, float &dist) const override {

    }
};

/*
for RectLight and CircleLight:
    abs(w) = 1 : X = z Y = [x1, x2] Z = [y1, y2]
    abs(w) = 2 : Y = z Z = [x1, x2] X = [y1, y2]
    abs(w) = 3 : Z = z X = [x1, x2] Y = [y1, y2]
*/

class RectLight : public AreaLight {
public:
    RectLight() = delete;

    RectLight(int w_, float z_, float x1_, float y1_, float x2_, float y2_, Vector3f _color) :
        w(w_), z(z_), x1(x1_), y1(y1_), x2(x2_), y2(y2_), color(_color) {
        if (abs(w) == 1) normal = Vector3f(w / 1, 0, 0);
        else if (abs(w) == 2) normal = Vector3f(0, w / 2, 0);
        else normal = Vector3f(0, 0, w / 3);
    }

    Vector3f getColor() {
        return color;
    }

    float area() {
        return (x2 - x1) * (y2 - y1);
    }

    Vector3f sampling(std::mt19937_64 &rnd) {
        std::uniform_real_distribution <float> dist(0, 1);
        float x = dist(rnd) * (x2 - x1) + x1;
        float y = dist(rnd) * (y2 - y1) + y1;
        if (abs(w) == 1) return Vector3f(z, x, y);
        else if (abs(w) == 2) return Vector3f(y, z, x);
        else return Vector3f(x, y, z);
    }

    bool intersect(const Ray &r, Hit &h, float tmin) {
        int u = abs(w) - 1, v = (u + 1) % 3, w = (v + 1) % 3;
        if (fabsf(r.getDirection()[u]) < 1e-8) return false;
        float t = (z - r.getOrigin()[u]) / r.getDirection()[u];
        if (t < tmin || t > h.getT()) return false;
        float x = r.getOrigin()[v] + t * r.getDirection()[v];
        float y = r.getOrigin()[w] + t * r.getDirection()[w];
        if (x < x1 || x > x2 || y < y1 || y > y2) return false;
        if (Vector3f::dot(r.getDirection(), normal) < 0)
            h.set(t, &LightMaterial::material, normal, color, true, Vector3f::ZERO);
        else
            h.set(t, &LightMaterial::material, normal, Vector3f::ZERO, false, Vector3f::ZERO);
        return true;
    }

private:
    int w;
    float z, x1, y1, x2, y2;
    Vector3f normal, color;
};

class CircleLight : public AreaLight {
public:
    CircleLight() = delete;

    CircleLight(int w_, float z_, float x_, float y_, float radius_, Vector3f _color) :
        w(w_), z(z_), x(x_), y(y_), radius(radius_), color(_color) {
        if (abs(w) == 1) normal = Vector3f(w / 1, 0, 0);
        else if (abs(w) == 2) normal = Vector3f(0, w / 2, 0);
        else normal = Vector3f(0, 0, w / 3);
    }

    Vector3f getColor() {
        return color;
    }

    float area() {
        return M_PI * radius * radius;
    }

    Vector3f sampling(std::mt19937_64 &rnd) {
        std::uniform_real_distribution <float> dist(0, 1);
        float u = sqrt(dist(rnd)) * radius, v = dist(rnd) * (2 * M_PI);
        float x0 = x + u * cos(v);
        float y0 = y + u * sin(v);
        if (abs(w) == 1) return Vector3f(z, x0, y0);
        else if (abs(w) == 2) return Vector3f(y0, z, x0);
        else return Vector3f(x0, y0, z);
    }

    bool intersect(const Ray &r, Hit &h, float tmin) {
        int u = abs(w) - 1, v = (u + 1) % 3, w = (v + 1) % 3;
        if (fabsf(r.getDirection()[u]) < 1e-8) return false;
        float t = (z - r.getOrigin()[u]) / r.getDirection()[u];
        if (t < tmin || t > h.getT()) return false;
        float x0 = r.getOrigin()[v] + t * r.getDirection()[v] - x;
        float y0 = r.getOrigin()[w] + t * r.getDirection()[w] - y;
        if (x0 * x0 + y0 * y0 > radius * radius) return false;
        if (Vector3f::dot(r.getDirection(), normal) < 0)
            h.set(t, &LightMaterial::material, normal, color, true, Vector3f::ZERO);
        else
            h.set(t, &LightMaterial::material, normal, Vector3f::ZERO, false, Vector3f::ZERO);
        return true;
    }

private:
    int w;
    float z, x, y, radius;
    Vector3f normal, color;
};

#endif // LIGHT_H
