/*
原创性：独立实现
*/

#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "ray.hpp"

class Material;

class Hit {
public:

    // constructors
    Hit() {
        material = nullptr;
        t = 1e38;
    }

    Hit(double _t, Material *m, const Vector3f &n, const Vector3f &c,
        bool f, const Vector3f &tangent) {
        t = _t;
        material = m;
        normal = n;
        color = c;
        this->tangent = tangent;
    }

    Hit(const Hit &h) {
        t = h.t;
        material = h.material;
        normal = h.normal;
        color = h.color;
        tangent = h.tangent;
    }

    // destructor
    ~Hit() = default;

    void set(float t_, Material *m, const Vector3f &n, const Vector3f &c,
        bool f, const Vector3f &tangent) {
        t = t_;
        material = m;
        normal = n.normalized();
        color = c;
        isFront = f;
        this->tangent = tangent;
    }
    
    float getT() const {
        return t;
    }

    Material *getMaterial() const {
        return material;
    }

    const Vector3f &getNormal() const {
        return normal;
    }

    const Vector3f &getColor() const {
        return color;
    }

    const bool &getIsFront() const {
        return isFront;
    }

    const Vector3f &getTangent() const {
        return tangent;
    }

    void setNormal(Vector3f n) {
        normal = n.normalized();
    }

    void setColor(Vector3f c) {
        color = c;
    }

private:
    float t;
    Material *material;
    Vector3f normal, color, tangent;
    bool isFront;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
