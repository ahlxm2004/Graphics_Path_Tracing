/*
原创性：独立实现
*/

#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <iostream>
#include <vecmath.h>
#include <random>

#include "ray.hpp"
#include "hit.hpp"
#include "texture.hpp"
#include "direction.hpp"

class Material {
public:
    virtual ~Material() = default;
    virtual Vector3f getColor() = 0;
    virtual Vector3f getColor(float u, float v) = 0;
    virtual Vector3f getColor(Vector2f u) {
        return getColor(u[0], u[1]);
    }
    bool useTexture() {
        return texture != nullptr;
    }

protected:
    Texture *texture;
};

class PhongMaterial : public Material {
public:
    explicit PhongMaterial(
        const Vector3f &a_color, const Vector3f &d_color,
        const Vector3f &s_color = Vector3f::ZERO, float s = 0) :
        ambientColor(a_color), diffuseColor(d_color), specularColor(s_color), shininess(s) {
    }

    Vector3f getDiffuseColor() const {
        return diffuseColor;
    }

    Vector3f getAmbientColor() const {
        return ambientColor;
    }

    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor) {
        Vector3f shaded = Vector3f::ZERO;
        float t = Vector3f::dot(dirToLight, hit.getNormal());
        if (t > 0) shaded += t * diffuseColor;
        Vector3f R = 2 * t * hit.getNormal() - dirToLight;
        t = Vector3f::dot(-ray.getDirection().normalized(), R);
        if (t > 0) shaded += powf(t, shininess) * specularColor;
        return shaded * lightColor;
    }

    Vector3f getColor() {
        return diffuseColor;
    }

    Vector3f getColor(float u, float v) {
        return diffuseColor;
    }

private:
    Vector3f ambientColor;
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
};

class ReflectiveMaterial : public Material {
public:
    explicit ReflectiveMaterial(float r) : rate(r) {}

    Vector3f getColor() {
        return Vector3f(rate);
    }

    Vector3f getColor(float u, float v) {
        return Vector3f(rate);
    }

private:
    float rate;
};

class RefractiveMaterial : public Material {
public:
    explicit RefractiveMaterial(float n0, float r) : n(n0), rate(r) {}

    float getN() {
        return n;
    }

    Vector3f getColor() {
        return Vector3f(rate);
    }

    Vector3f getColor(float u, float v) {
        return Vector3f(rate);
    }

private:
    float n, rate;
};

class FresnelMaterial : public Material {
public:
    explicit FresnelMaterial(float n_, float r) : n(n_), rate(r) {
        f0 = (1 - n) / (1 + n);
        f0 *= f0;
    }

    float reflectProb(float dot) {
        dot = 1 - dot;
        float dot2 = dot * dot;
        return f0 + (1 - f0) * dot * dot2 * dot2;
    }

    float getN() {
        return n;
    }

    Vector3f getColor() {
        return Vector3f(rate);
    }

    Vector3f getColor(float u, float v) {
        return Vector3f(rate);
    }

private:
    float n, f0, rate;
};

class LightMaterial : public Material {
public:
    explicit LightMaterial() {}

    Vector3f getColor() {
        return Vector3f(1);
    }

    Vector3f getColor(float u, float v) {
        return Vector3f(1);
    }

    static LightMaterial material;
};

inline LightMaterial LightMaterial::material = LightMaterial();

class BRDFMaterial : public Material {
public:
    virtual Vector3f getBRDF(Vector3f incident, Vector3f normal, Vector3f reflect, Vector3f tangent) = 0;

    virtual Vector3f sampling(Vector3f incident, Vector3f normal, std::mt19937_64 &rnd) = 0;

    virtual float samplingPDF(Vector3f incident, Vector3f normal, Vector3f reflect) = 0;
    
    Vector3f getColor() {
        return color;
    }

    Vector3f getColor(float u, float v) {
        return texture->getColor(u, v);
    }

protected:
    Vector3f color;
};

class PhongBRDFMaterial : public BRDFMaterial {
public:
    explicit PhongBRDFMaterial(float rho_d_, float rho_s_, float shininess_,
        Vector3f color_, Texture *texture_) : shininess(shininess_) {
        rho_d = rho_d_, rho_s = rho_s_;
        rho_d0 = rho_d / M_PI;
        rho_s0 = rho_s * ((shininess + 2) * (shininess + 4)) /
            ((8 * M_PI) * (pow(2, -shininess / 2) + shininess));
        color = color_;
        texture = texture_;
    }
    
    Vector3f getBRDF(Vector3f incident, Vector3f normal, Vector3f reflect, Vector3f tangent) {
        float dot = Vector3f::dot(normal, (-incident + reflect).normalized());
        return Vector3f(rho_d0 + rho_s0 * powf(dot, shininess));
    }

    Vector3f sampling(Vector3f incident, Vector3f normal, std::mt19937_64 &rnd) {
        std::uniform_real_distribution <float> gen(0, 1);
        if (gen(rnd) * (rho_d + rho_s) < rho_d)
            return rotate(uniformHemisphere(rnd), normal);
        float cos_theta = powf(gen(rnd), 1 / (shininess + 1));
        float sin_theta = sqrtf(1 - cos_theta * cos_theta);
        float phi = 2 * M_PI * gen(rnd);
        Vector3f h = rotate(Vector3f(sin_theta * cos(phi),
            sin_theta * sin(phi), cos_theta), normal);
        return getReflectDir(incident, h);
    }

    float samplingPDF(Vector3f incident, Vector3f normal, Vector3f reflect) {
        Vector3f h = (-incident + reflect).normalized();
        float dot1 = Vector3f::dot(normal, h);
        if (dot1 < 0) dot1 = -dot1, h = -h;
        float dot2 = Vector3f::dot(h, reflect);
        float result = 0;
        result += rho_d / (rho_d + rho_s) / (2 * M_PI);
        result += rho_s / (rho_d + rho_s) * (shininess + 1) /
            (2 * M_PI) * powf(dot1, shininess) / (4 * dot2);
        return result;
    }

private:
    float rho_d, rho_s, rho_d0, rho_s0, shininess;
};

class CookBRDFMaterial : public BRDFMaterial {
public:
    explicit CookBRDFMaterial(float rho_d_, float rho_s_, float alpha_, Vector3f F0_,
        Vector3f color_, Texture *texture_) : alpha(alpha_), F0(F0_) {
        rho_d0 = rho_d_ / M_PI;
        rho_s0 = rho_s_ / (4 * M_PI);
        alpha2 = alpha * alpha, k = (alpha + 1) * (alpha + 1) / 8;
        texture = texture_;
        color = color_;
    }

    Vector3f getBRDF(Vector3f incident, Vector3f normal, Vector3f reflect, Vector3f tangent) {
        Vector3f h = (-incident + reflect).normalized();
        float ni = -Vector3f::dot(normal, incident);
        float nr = Vector3f::dot(normal, reflect);
        float nh = Vector3f::dot(normal, h);
        float D = nh * nh * (alpha2 - 1) + 1;
        D = alpha2 / (D * D);
        Vector3f F = F0 + (1 - F0) * pow5(1 - nr);
        float G = 1 / ((ni * (1 - k) + k) * (nr * (1 - k) + k));
        return rho_d0 + rho_s0 * D * F * G;
    }

    Vector3f sampling(Vector3f incident, Vector3f normal, std::mt19937_64 &rnd) {
        std::uniform_real_distribution <float> gen(0, 1);
        float theta = atanf(alpha * sqrt(1 / (1 / gen(rnd) - 1)));
        float phi = 2 * M_PI * gen(rnd);
        Vector3f h = rotate(Vector3f(sin(theta) * cos(phi),
            sin(theta) * sin(phi), cos(theta)), normal);
        return getReflectDir(incident, h);
    }

    float samplingPDF(Vector3f incident, Vector3f normal, Vector3f reflect) {
        Vector3f h = (-incident + reflect).normalized();
        float nh = Vector3f::dot(normal, h);
        if (nh < 0) nh = -nh, h = -h;
        float D = nh * nh * (alpha2 - 1) + 1;
        D = alpha2 / (M_PI * D * D);
        return D * nh / (4 * Vector3f::dot(h, reflect));
    }

private:
    float rho_d0, rho_s0, alpha, alpha2, k;
    Vector3f F0;

    inline float pow5(float t) {
        float t2 = t * t;
        return t2 * t2 * t;
    }
};

class WardBRDFMaterial : public BRDFMaterial {
public:
    explicit WardBRDFMaterial(float rho_d_, float rho_s_, float alpha_x_, float alpha_y_,
        Vector3f tangent0_, Vector3f color_, Texture *texture_) : tangent0(tangent0_) {
        rho_d0 = rho_d_ / M_PI;
        rho_s0 = rho_s_ / (4 * M_PI * alpha_x_ * alpha_y_);
        alpha_x2 = alpha_x_ * alpha_x_;
        alpha_y2 = alpha_y_ * alpha_y_;
        texture = texture_;
        color = color_;
    }

    Vector3f getBRDF(Vector3f incident, Vector3f normal, Vector3f reflect, Vector3f tangent) {
        if (tangent0 != Vector3f::ZERO) tangent = tangent0;
        Vector3f h = (-incident + reflect).normalized();
        float ni = -Vector3f::dot(normal, incident);
        float nr = Vector3f::dot(normal, reflect);
        float nh = Vector3f::dot(normal, h);
        Vector3f h0 = (h - nh * normal).normalized();
        float ht = Vector3f::dot(h0, tangent), ht2 = ht * ht;
        float e = (1 / (nh * nh) - 1) * (ht2 / alpha_x2 + (1 - ht2) / alpha_y2);
        return rho_d0 + rho_s0 / sqrtf(ni * nr) * expf(-e);
    }

    Vector3f sampling(Vector3f incident, Vector3f normal, std::mt19937_64 &rnd) {
        return Vector3f(0);
    }

    float samplingPDF(Vector3f incident, Vector3f normal, Vector3f reflect) {
        return 0;
    }

private:
    float rho_d0, rho_s0;
    float alpha_x2, alpha_y2;
    Vector3f tangent0;
};

#endif // MATERIAL_H
