/*
原创性：独立实现
*/

#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>
#include <numeric>

int Mesh::intersect_tid(const Ray &r, Hit &h, float tmin) {
    int tid = -1;
    if (useBVH) BVHintersect(r, h, tmin, 0, tid);
    else {
        for (int triId = 0; triId < (int)triangles.size(); ++triId)
            if (triangles[triId].intersect(r, h, tmin)) tid = triId;
    }
    return tid;
}

bool Mesh::intersect(const Ray &r, Hit &h, float tmin) {
    int tid = intersect_tid(r, h, tmin);
    if (tid != -1 && (useVT || useVN)) {
        Vector3f weight = triangles[tid].getWeight(r, h);
        if (useVT && h.getIsFront())
            h.setColor(material->getColor(weight[0] * vt[vt_id[tid][0]] +
                weight[1] * vt[vt_id[tid][1]] + weight[2] * vt[vt_id[tid][2]]));
        if (useVN) {
            h.setNormal(weight[0] * vn[vn_id[tid][0]] +
                weight[1] * vn[vn_id[tid][1]] + weight[2] * vn[vn_id[tid][2]]);
        }
    }
    return tid != -1;
}

Mesh::Mesh(const char *filename, Material *material) : Object3D(material) {
    std::ifstream f;
    f.open(filename);
    if (!f.is_open()) {
        std::cout << "Cannot open " << filename << "\n";
        return;
    }
    std::string line;
    std::string vTok("v");
    std::string fTok("f");
    std::string vtTok("vt");
    std::string vnTok("vn");
    char bslash = '/', space = ' ';
    std::string tok;
    int texID;
    useBVH = useVT = useVN = false;
    static int A[12];
    while (true) {
        std::getline(f, line);
        if (f.eof()) {
            break;
        }
        if (line.size() < 3) {
            continue;
        }
        if (line.at(0) == '#') {
            continue;
        }
        std::stringstream ss(line);
        ss >> tok;
        if (tok == vTok) {
            Vector3f v;
            ss >> v[0] >> v[1] >> v[2];
            add_v(v);
        } else if (tok == vtTok) {
            Vector2f vt;
            ss >> vt[0];
            ss >> vt[1];
            add_vt(vt);
        } else if (tok == vnTok) {
            Vector3f vn;
            ss >> vn[0] >> vn[1] >> vn[2];
            add_vn(vn);
        } else if (tok == fTok) {
            if (line.find(bslash) != std::string::npos) {
                int cnt = std::count(line.begin(), line.end(), bslash) / 2;
                std::replace(line.begin(), line.end(), bslash, space);
                std::stringstream facess(line);
                facess >> tok;
                for (int i = 0; i < cnt * 3; i++) facess >> A[i], A[i]--;
                add_vid(A[0], A[3], A[6]);
                add_vtid(A[1], A[4], A[7]);
                add_vnid(A[2], A[5], A[8]);
                if (cnt == 4) {
                    add_vid(A[6], A[9], A[0]);
                    add_vtid(A[7], A[10], A[1]);
                    add_vnid(A[8], A[11], A[2]);
                }
            } else {
                ss >> A[0] >> A[1] >> A[2];
                add_vid(A[0] - 1, A[1] - 1, A[2] - 1);
            }
        } 
    }
    generate();

    f.close();
}

void Mesh::generate() {
    printf("mesh: V = %d F = %d useVT = %d useVN = %d\n",
        (int)v.size(), (int)v_id.size(), useVT, useVN);
    
    triangles.reserve(v_id.size());
    for (int triId = 0; triId < (int) v_id.size(); ++triId)
        triangles.emplace_back(v[v_id[triId][0]], v[v_id[triId][1]], v[v_id[triId][2]], material);
}

void Mesh::buildBVH() {
    useBVH = true;
    bvhId.resize(triangles.size());
    std::iota(bvhId.begin(), bvhId.end(), 0);
    bvhTree.emplace_back(0, 0, bvhId.size());
    for (int i = 0; i < bvhTree.size(); i++) {
        int l = bvhTree[i].idl, r = bvhTree[i].idr, m = (l + r) / 2;
        for (int j = l; j < r; j++)
            bvhTree[i].volume.merge(triangles[bvhId[j]]);
        if (r - l <= 3) {
            bvhTree[i].cutd = -1;
            continue;
        }
        bvhTree[i].cutd = bvhTree[i].volume.getMaxD();
        int d = bvhTree[i].cutd;

        std::nth_element(bvhId.begin() + l, bvhId.begin() + m, bvhId.begin() + r, [&] (int u, int v) {
            return triangles[u].dmin(d) < triangles[v].dmin(d);
        });
        bvhTree[i].son[0] = bvhTree.size();
        bvhTree.emplace_back(0, l, m);
        bvhTree[i].son[1] = bvhTree.size();
        bvhTree.emplace_back(0, m, r);
    }
}

void Mesh::BVHintersect(const Ray &r, Hit &h, float tmin, int p, int &tid) {
    int d = bvhTree[p].cutd;
    float t = bvhTree[p].volume.intersect(r);
    if (t == -1 || t > h.getT()) return;
    bool result = false;
    if (d == -1) {
        for (int i = bvhTree[p].idl; i < bvhTree[p].idr; i++)
            if (triangles[bvhId[i]].intersect(r, h, tmin)) tid = bvhId[i];
    }
    else {
        int k = (r.getDirection()[d] < 0);
        BVHintersect(r, h, tmin, bvhTree[p].son[k], tid);
        BVHintersect(r, h, tmin, bvhTree[p].son[k ^ 1], tid);
    }
}
