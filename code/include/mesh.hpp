/*
原创性：独立实现
*/

#ifndef MESH_H
#define MESH_H

#include <vector>
#include "object3d.hpp"
#include "triangle.hpp"
#include "Vector2f.h"
#include "Vector3f.h"
#include "ray.hpp"

class Mesh : public Object3D {

public:
    Mesh(Material *material) {
        useVT = useVN = useBVH = false;
        this->material = material;
    }

    Mesh(const char *filename, Material *m);

    struct TriangleIndex {
        TriangleIndex() {
            x[0] = 0; x[1] = 0; x[2] = 0;
        }
        TriangleIndex(int u, int v, int w) {
            x[0] = u; x[1] = v; x[2] = w;
        }
        int &operator[](const int i) { return x[i]; }
        int x[3]{};
    };

    void add_v(Vector3f u) {v.push_back(u);}
    void add_vid(int u, int v, int w) {v_id.emplace_back(u, v, w);}
    void add_vt(Vector2f u) {useVT = true; vt.push_back(u);}
    void add_vt(float u, float v) {add_vt(Vector2f(u, v));}
    void add_vtid(int u, int v, int w) {vt_id.emplace_back(u, v, w);}
    void add_vn(Vector3f u) {useVN = true; vn.push_back(u);}
    void add_vn(float u, float v, float w) {add_vn(Vector3f(u, v, w));}
    void add_vnid(int u, int v, int w) {vn_id.emplace_back(u, v, w);}

    int getTriangleIndex(int t, int x) {return v_id[t][x];}

    Triangle getTriangle(int i) {
        assert(i >= 0 && i < triangles.size());
        return triangles[i];
    }

    void generate();
    void buildBVH();
    int intersect_tid(const Ray &r, Hit &h, float tmin);
    bool intersect(const Ray &r, Hit &h, float tmin) override;

private:

    std::vector <Vector3f> v;
    std::vector <Vector2f> vt;
    std::vector <Vector3f> vn;
    std::vector <TriangleIndex> v_id, vt_id, vn_id;
    std::vector <Triangle> triangles;

    bool useVT, useVN;

    struct volume3d {
        float dmin[3], dmax[3];
        

        volume3d() {
            dmin[0] = dmin[1] = dmin[2] = 1e9;
            dmax[0] = dmax[1] = dmax[2] = -1e9;
        }

        void merge(Vector3f point) {
            for (int i = 0; i < 3; i++) {
                if (point[i] < dmin[i]) dmin[i] = point[i];
                if (point[i] > dmax[i]) dmax[i] = point[i];
            }
        }

        void merge(volume3d b) {
            for (int i = 0; i < 3; i++) {
                if (b.dmin[i] < dmin[i]) dmin[i] = b.dmin[i];
                if (b.dmax[i] > dmax[i]) dmax[i] = b.dmax[i];
            }
        }

        void merge(Triangle t) {
            merge(t.vertices(0));
            merge(t.vertices(1));
            merge(t.vertices(2));
        }

        int getMaxD() {
            int d = 0;
            for (int i = 1; i < 3; i++)
                if (dmax[i] - dmin[i] > dmax[d] - dmin[d]) d = i;
            return d;
        }

        float intersect(const Ray &ray) {
            bool ok = true;
            for (int i = 0; i < 3 && ok; i++)
                ok &= (ray.getOrigin()[i] >= dmin[i] && ray.getOrigin()[i] <= dmax[i]);
            if (ok) return 0;
            for (int d = 0; d < 3; d++) {
                if (ray.getDirection()[d] > 0) {
                    if (ray.getOrigin()[d] > dmax[d]) return -1;
                }
                else {
                    if (ray.getOrigin()[d] < dmin[d]) return -1;
                }
            }
            for (int d = 0; d < 3; d++) {
                float t;
                if (ray.getDirection()[d] > 0) {
                    if (ray.getOrigin()[d] < dmin[d])
                        t = (dmin[d] - ray.getOrigin()[d]) / ray.getDirection()[d];
                    else continue;
                }
                else if (ray.getDirection()[d] < 0) {
                    if (ray.getOrigin()[d] > dmax[d])
                        t = (dmax[d] - ray.getOrigin()[d]) / ray.getDirection()[d];
                    else continue;
                }
                int dx = (d + 1) % 3;
                float x = ray.getOrigin()[dx] + t * ray.getDirection()[dx];
                if (x >= dmin[dx] && x <= dmax[dx]) {
                    dx = (dx + 1) % 3;
                    x = ray.getOrigin()[dx] + t * ray.getDirection()[dx];
                    if (x >= dmin[dx] && x <= dmax[dx]) return t;
                }
            }
            return -1;
        }        
    };

    struct bvhNode {
        int son[2], cutd, idl, idr;
        volume3d volume;
        bvhNode(int cutd_, int idl_, int idr_) :
            cutd(cutd_), idl(idl_), idr(idr_) {
            son[0] = son[1] = -1;
        }
    };

    std::vector <bvhNode> bvhTree;
    std::vector <int> bvhId;

    bool useBVH;
    
    void BVHintersect(const Ray &r, Hit &h, float tmin, int p, int &tid);
};

#endif
