/*
原创性：独立实现
*/

#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>

#include <algorithm>

class Curve {
public:
    explicit Curve(std::vector <Vector3f> points) : controls(std::move(points)) {}
    std::vector <Vector3f> &getControls() {return controls;}
    virtual std::pair <Vector3f, Vector3f> getPoint(float t) = 0;

protected:
    std::vector<Vector3f> controls;
};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1 (n >= 1) !\n");
            exit(0);
        }
        this->points = points;
    }

    int getSegCount() {return points.size() / 3;}

    std::pair <Vector3f, Vector3f> getPoint(float t) {
        t *= points.size() / 3;
        int p = (int)t;
        if (p < 0) p = 0;
        if (p >= points.size() / 3)
            p = points.size() / 3 - 1;
        t -= p; p *= 3;
        return std::make_pair(
            1 * (1 - t) * (1 - t) * (1 - t) * points[p] +
            3 * t * (1 - t) * (1 - t) * points[p + 1] +
            3 * t * t * (1 - t) * points[p + 2] +
            1 * t * t * t * points[p + 3],
            (-3 * (1 - t) * (1 - t) * points[p] +
            3 * ((1 - t) * (1 - t) - 2 * t * (1 - t)) * points[p + 1] +
            3 * (2 * t - 3 * t * t) * points[p + 2] +
            3 * t * t * points[p + 3]) * (points.size() / 3)
        );
    }

protected:
    std::vector<Vector3f> points;

};

class BsplineCurve : public Curve {
public:
    static const int k = 3;

    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() <= k) {
            printf("Number of control points of BspineCurve must be more than %d!\n", k);
            exit(0);
        }
        this->points = points;
    }

    int getSegCount() {return points.size() - k;}

    std::pair <Vector3f, Vector3f> getPoint(float t) {
        int n = points.size() - 1;
        t = (n - k + 1) * t + k;
        int p0 = (int)(t);
        if (p0 < k) p0 = k;
        if (p0 > n) p0 = n;
        std::vector <float> B(k);
        for (int l = 0; l < k; l++)
            B[l] = (l + 1 == k ? 1. : 0.);
        int s = p0 - k + 1;
        t -= s;
        for (int p = 1; p < k; p++) {
            for (int l = k - p - 1; l < k - 1; l++)
                B[l] = (t - l) / p * B[l] + (l + p - t + 1) / p * B[l + 1];
            B[k - 1] *= (t - (k - 1)) / p;
        }
        Vector3f V(0), T(0);
        for (int l = 0; l < k - 1; l++)
            V += (B[l] * (t - l) + B[l + 1] * (l + k + 1 - t)) * points[l + s],
            T += (B[l] - B[l + 1]) * points[l + s];
        V += B[0] * (k - t) * points[s - 1];
        V += B[k - 1] * (t - (k - 1)) * points[k - 1 + s];
        T -= B[0] * points[p0 - k];
        T += B[k - 1] * points[p0];
        return std::make_pair(V / k, T * (n - k + 1));
    }

protected:
    std::vector<Vector3f> points; 
};

#endif // CURVE_HPP
