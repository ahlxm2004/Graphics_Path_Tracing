/*
原创性：独立实现
*/

#include "tracing_MC.hpp"
#include "group.hpp"
#include "camera.hpp"

namespace {
    bool gamble(std::mt19937_64 &rnd, float rrProb) {
        std::uniform_real_distribution <float> gen(0, 1);
        return gen(rnd) < rrProb;
    }

    Vector3f sampling(Vector3f incident, Vector3f normal, std::mt19937_64 &rnd, BRDFMaterial *material, int sampling) {
        if (sampling == 0 || sampling == 1) return rotate(uniformHemisphere(rnd), normal);
        if (sampling == 2) return rotate(cosWeightedHemisphere(rnd), normal);
        if (sampling == 3) return material->sampling(incident, normal, rnd);
        if (sampling == 4) {
            if (gamble(rnd, 0.5))
                return rotate(cosWeightedHemisphere(rnd), normal);
            else
                return material->sampling(incident, normal, rnd);
        }
        return Vector3f(0);   
    }

    float getPDF(Vector3f incident, Vector3f normal, Vector3f reflect, BRDFMaterial *material, int sampling) {
        float dot = Vector3f::dot(normal, reflect);
        if (dot < 0) return 0;
        if (sampling == 0 || sampling == 1) return 1 / (2 * M_PI);
        if (sampling == 2) return dot / M_PI;
        if (sampling == 3) return material->samplingPDF(incident, normal, reflect);
        if (sampling == 4) return (dot / M_PI + material->samplingPDF(incident, normal, reflect)) / 2;
        return 0;
    }
}

Vector3f tracingMC(Ray ray, SceneParser &Parser, std::mt19937_64 &rnd, AreaLight *&pLight) {
    pLight = nullptr;
    Group *baseGroup = Parser.getGroup();
    Hit hit;
    float tmin = Parser.getTmin();
    bool hitted = baseGroup->intersect(ray, hit, tmin);
    for (int l = 0; l < Parser.getNumLights(); l++) {
        AreaLight *light = dynamic_cast <AreaLight *> (Parser.getLight(l));
        if (light && light->intersect(ray, hit, tmin)) {
            hitted = true;
            if (hit.getIsFront()) pLight = light;
        }
    }

    if (!hitted) return Vector3f(0);
    if (hit.getMaterial() == &LightMaterial::material) return hit.getColor();
    if (gamble(rnd, Parser.getrrProb())) return Vector3f(0);

    Vector3f finalColor(0);
    Vector3f color = hit.getColor();
    Vector3f point = ray.pointAtParameter(hit.getT());
    AreaLight *pLight2 = nullptr;

    if (dynamic_cast <ReflectiveMaterial *> (hit.getMaterial())) {
        Vector3f direction = getReflectDir(ray.getDirection(), hit.getNormal());
        finalColor = color * tracingMC(Ray(point, direction), Parser, rnd, pLight2);
    }
    else if (dynamic_cast <RefractiveMaterial *> (hit.getMaterial())) {
        float n = dynamic_cast <RefractiveMaterial *> (hit.getMaterial())->getN();
        Vector3f direction; float weight;
        getRefractDir(ray.getDirection(), hit.getNormal(),
            hit.getIsFront(), n, direction, weight);
        if (weight == 0)
            weight = 1, direction = getReflectDir(ray.getDirection(), hit.getNormal());
        else
            weight = 1 / weight;
        finalColor = weight * color * tracingMC(Ray(point, direction), Parser, rnd, pLight2);
    }
    else if (dynamic_cast <FresnelMaterial *> (hit.getMaterial())) {
        FresnelMaterial *material = dynamic_cast <FresnelMaterial *> (hit.getMaterial());
        Vector3f direction; float weight;
        getRefractDir(ray.getDirection(), hit.getNormal(),
            hit.getIsFront(), material->getN(), direction, weight);
        if (weight == 0)
            weight = 1, direction = getReflectDir(ray.getDirection(), hit.getNormal());
        else {
            float prob = material->reflectProb(-Vector3f::dot(hit.getNormal(),
                hit.getIsFront() ? ray.getDirection() : direction));
            if (gamble(rnd, prob))
                weight = 1, direction = getReflectDir(ray.getDirection(), hit.getNormal());
            else
                weight = 1 / weight;
        }
        finalColor = weight * color * tracingMC(Ray(point, direction), Parser, rnd, pLight2);
    }
    else if (dynamic_cast <BRDFMaterial *> (hit.getMaterial())) {
        BRDFMaterial *material = dynamic_cast <BRDFMaterial *> (hit.getMaterial());
        Vector3f reflect, next1, next2, constant(0);
        float weight1 = 1, p1, p2;

        reflect = sampling(ray.getDirection(), hit.getNormal(), rnd, material, Parser.getSampling());
        p1 = getPDF(ray.getDirection(), hit.getNormal(), reflect, material, Parser.getSampling());

        if (Vector3f::dot(hit.getNormal(), reflect) < 0) next1 = Vector3f(0);
        else {
            next1 = material->getBRDF(-reflect, hit.getNormal(), -ray.getDirection(),
                hit.getTangent()) * tracingMC(Ray(point, reflect), Parser, rnd, pLight2) *
                Vector3f::dot(hit.getNormal(), reflect);
            weight1 = 1 / p1;
        }
        
        if (Parser.getSampling() != 0) {
            if (pLight2) {
                Hit hit2; pLight2->intersect(Ray(point, reflect), hit2, tmin);
                p2 = hit2.getT() * hit2.getT() /
                    (-Vector3f::dot(hit2.getNormal(), reflect) * pLight2->area());
                weight1 = 1 / (p1 + p2);
            }
            for (int l = 0; l < Parser.getNumLights(); l++) {
                AreaLight *light = dynamic_cast <AreaLight *> (Parser.getLight(l));
                if (!light) continue;
                Vector3f ppp = light->sampling(rnd);
                Ray ray2(point, (ppp - point).normalized());
                Hit hit2;
                if (Vector3f::dot(ray2.getDirection(), hit.getNormal()) < 0) continue;
                if (!light->intersect(ray2, hit2, tmin) || !hit2.getIsFront()) continue;
                bool shaded = baseGroup->intersect(ray2, hit2, tmin);
                for (int l0 = 0; l0 < Parser.getNumLights() && !shaded; l0++) {
                    if (l == l0) continue;
                    AreaLight *light2 = dynamic_cast <AreaLight *> (Parser.getLight(l0));
                    if (light2 && light2->intersect(ray2, hit2, tmin)) shaded = true;
                }
                if (shaded) continue;
                p1 = getPDF(ray.getDirection(), hit.getNormal(),
                    ray2.getDirection(), material, Parser.getSampling());
                p2 = hit2.getT() * hit2.getT() /
                    (-Vector3f::dot(hit2.getNormal(), ray2.getDirection()) * light->area());
                next2 = material->getBRDF(-ray2.getDirection(), hit.getNormal(), -ray.getDirection(),
                    hit.getTangent()) * hit2.getColor() *
                    Vector3f::dot(hit.getNormal(), ray2.getDirection());
                constant += next2 / (p1 + p2);
            }
        }
        finalColor += color * (weight1 * next1 + constant);
    }
    return finalColor / (1 - Parser.getrrProb());
}

Vector3f tracingMC(int x, int y, SceneParser &Parser) {
    int SPP = Parser.getSPP();
    Camera* camera = Parser.getCamera();
    std::mt19937_64 rnd(y * camera->getWidth() + x);
    AreaLight *pLight;
    Vector2f p0(x, y);
    double u = 0, v = 0, w = 0;
    for (int k = 0; k < SPP; k++) {
        Vector2f p = p0;
        if (Parser.getAntialias() & 1) {
            p[0] += (k + 0.5) / SPP;
            int i = k; float w = 1;
            while (i) {
                w /= 2;
                if (i & 1) p[1] += w;
                i /= 2;
            }
            if (p[0] >= x + 1) p[0] -= 1;
            if (p[1] >= y + 1) p[1] -= 1;
            p[0] -= 0.5, p[1] -= 0.5;
        }
        Vector3f t = tracingMC(camera->generateRay(p), Parser, rnd, pLight);
        u += t[0], v += t[1], w += t[2];
    }
    return Vector3f(u / SPP, v / SPP, w / SPP);
}
