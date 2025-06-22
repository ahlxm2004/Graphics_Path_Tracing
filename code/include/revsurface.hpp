/*
原创性：独立实现
*/

#ifndef REVSURFACE_HPP
#define REVSURFACE_HPP

#include "object3d.hpp"
#include "curve.hpp"
#include <tuple>

class RevSurface : public Object3D {
public:
    static const int newtonSteps = 15;
    static constexpr double eps = 1e-4;

    RevSurface(Curve *pCurve, Material* material, int step1, int step2, bool isNewton)
        : pCurve(pCurve), Object3D(material) {
        this->step1 = step1;
        this->step2 = step2;
        this->isNewton = isNewton;
        for (const auto &cp : pCurve->getControls())
            if (cp.z() != 0.0) {
                printf("Profile of revSurface must be flat on xy plane.\n");
                exit(0);
            }
        buildMesh();
    }

    ~RevSurface() override {
        delete pCurve;
        delete pMesh;
    }

    bool intersect_newton(const Ray &r, Hit &h, float tmin, int tid) {
        int vid = pMesh->getTriangleIndex(tid, 0);
        double t_min = (double)(vid / step2) / step1;
        double t_max = t_min + 1. / step1;
        double t = (t_min + t_max) / 2;
        t_min -= 0.05; t_max += 0.05;
        if (t_min < 0) t_min = 0;
        if (t_max > 1) t_max = 1;

        double ox = r.getOrigin()[0], oy = r.getOrigin()[1], oz = r.getOrigin()[2];
        double dx = r.getDirection()[0], dy = r.getDirection()[1], dz = r.getDirection()[2];
        double dy2 = dy * dy, oxdy = ox * dy, ozdy = oz * dy;
        double lx, ly, Dlx, Dly;
        Vector3f l, Dl;

        for (int step = 0; ; step++) {
            std::tie(l, Dl) = pCurve->getPoint(t);
            lx = l[0], ly = l[1];
            Dlx = Dl[0], Dly = Dl[1];
            
            double u = oxdy + (ly - oy) * dx, v = ozdy + (ly - oy) * dz;
            double f = u * u + v * v - dy2 * lx * lx;

            if (step == newtonSteps) {
                if (fabs(f) > eps || (ly - oy) / dy < tmin) return false;
                Vector3f p = r.pointAtParameter((ly - oy) / dy);
                double Dl = sqrt(Dlx * Dlx + Dly * Dly);
                if (lx < 0) Dlx = -Dlx;
                Dlx /= Dl, Dly /= Dl;
                double n0 = sqrt(p[0] * p[0] + p[2] * p[2]);
                double nx = p[0] / n0, nz = p[2] / n0;
                Vector3f normal(-Dly * nx, Dlx, -Dly * nz);
                Vector3f tangent(Dlx * nx, Dly, Dlx * nz);
                bool isFront = (Vector3f::dot(normal, r.getDirection()) < 0);
                if (!isFront > 0) normal = -normal;
                Vector3f color;
                if (material->useTexture() && isFront) {
                    float v = atan2f(p[0], p[2]) / (2 * M_PI) + 1.25;
                    if (v > 1) v -= 1;
                    color = material->getColor(v, 1 - t);
                }
                else color = material->getColor();
                h.set((ly - oy) / dy, material, normal, color, isFront, tangent);
                return true;
            }

            double Df = 2 * (Dly * (u * dx + v * dz) - dy2 * lx * Dlx);

            t -= f / Df;
            if (t < t_min) t = t_min;
            if (t > t_max) t = t_max;
        }
        return false;
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        if (!isNewton)
            return pMesh->intersect(r, h, tmin);
        else {
            float tmin0 = tmin;
            while (true) {
                Hit h0 = h;
                int tid = pMesh->intersect_tid(r, h0, tmin0);
                if (tid == -1) return false;
                if (intersect_newton(r, h, tmin, tid)) return true;
                tmin0 = h0.getT() + tmin;
            }
        }
    }

    void buildMesh() {
        pMesh = new Mesh(material);
        curvePoints.resize(step1 + 1);
        for (int i = 0; i <= step1; i++)
            curvePoints[i] = pCurve->getPoint((float)i / step1).first;
        for (unsigned int ci = 0; ci <= step1; ++ci) {
            for (unsigned int i = 0; i < step2; ++i) {
                float t = (float) i / step2;
                Quat4f rot;
                rot.setAxisAngle(t * 2 * M_PI, Vector3f::UP);
                Vector3f pnew = Matrix3f::rotation(rot) * curvePoints[ci];
                pMesh->add_v(pnew);
                if (material->useTexture())
                    pMesh->add_vt((float)i / step2, 1 - (float)ci / (curvePoints.size() - 1));
                int i1 = (i + 1 == step2) ? 0 : i + 1;
                if (ci != step1) {
                    pMesh->add_vid(ci * step2 + i, (ci + 1) * step2 + i, ci * step2 + i1);
                    pMesh->add_vid(ci * step2 + i1, (ci + 1) * step2 + i, (ci + 1) * step2 + i1);
                    if (material->useTexture()) {
                        pMesh->add_vtid(ci * step2 + i, (ci + 1) * step2 + i, ci * step2 + i1);
                        pMesh->add_vtid(ci * step2 + i1, (ci + 1) * step2 + i, (ci + 1) * step2 + i1);
                    }
                }
            }
        }
        pMesh->generate();
        pMesh->buildBVH();
    }

private:
    Curve *pCurve;
    Mesh *pMesh;
    int step1, step2;
    bool isNewton;

    std::vector <Vector3f> curvePoints;
};

#endif //REVSURFACE_HPP
