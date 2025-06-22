/*
原创性：独立实现
*/

#include "direction.hpp"

Vector3f getReflectDir(Vector3f incident, Vector3f normal) {
    float dot = Vector3f::dot(incident, normal);
    return incident - 2 * dot * normal;
}

void getRefractDir(Vector3f incident, Vector3f normal, bool isFront, float n, Vector3f &direction, float &weight) {
    float dot = Vector3f::dot(incident, normal);
    if (isFront) n = 1 / n;
    float sin_angle = sqrt(1 - dot * dot);
    float sin_angle_n = sin_angle * n;
    if (sin_angle_n < 1) {
        float cos_angle_n = sqrt(1 - sin_angle_n * sin_angle_n);
        weight = fabsf(dot / cos_angle_n);
        direction = -cos_angle_n * normal + sin_angle_n * (incident - dot * normal) / sin_angle;
    }
    else weight = 0;
}

Vector3f uniformHemisphere(std::mt19937_64 &rnd) {
    std::uniform_real_distribution <float> gen(0, 1);
    float u = gen(rnd) * (2 * M_PI), v = gen(rnd), w = sqrtf(1 - v * v);
    return Vector3f(w * cosf(u), w * sinf(u), v);
}

Vector3f cosWeightedHemisphere(std::mt19937_64 &rnd) {
    std::uniform_real_distribution <float> gen(0, 1);
    float u = sqrtf(gen(rnd)), v = gen(rnd) * (2 * M_PI);
    return Vector3f(u * cosf(v), u * sinf(v), sqrtf(1 - u * u));
}

Vector3f rotate(Vector3f base, Vector3f normal) {
    Vector3f U = (fabsf(normal.x()) > 0.5 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0));
    U = Vector3f::cross(U, normal).normalized();
    Vector3f V = Vector3f::cross(U, normal);
    return U * base.x() + V * base.y() + normal * base.z();
}
