/*
原创性：独立实现
*/

#include "tracing_Whitted.hpp"
#include "hit.hpp"
#include "group.hpp"
#include "light.hpp"
#include "camera.hpp"

Vector3f tracingWhitted(Ray ray, SceneParser &Parser, Vector3f rate) {
    float tmin = Parser.getTmin();
    if (rate.squaredLength() < 1e-6) return Vector3f::ZERO;
    Vector3f finalColor = Parser.getBackgroundColor();
    Group *baseGroup = Parser.getGroup();
    Hit hit;
    if (baseGroup->intersect(ray, hit, tmin)) {
        Vector3f point = ray.pointAtParameter(hit.getT());
        if (dynamic_cast <PhongMaterial *> (hit.getMaterial())) {
            PhongMaterial *material = dynamic_cast <PhongMaterial *> (hit.getMaterial());
            finalColor = finalColor * (material->getAmbientColor());
            for (int l = 0; l < Parser.getNumLights(); l++) {
                Light *light = Parser.getLight(l);
                Vector3f L, lightColor;
                float dist; Hit hit2;
                light->getIllumination(point, L, lightColor, dist);
                if (!baseGroup->intersect(Ray(point, L), hit2, tmin) || hit2.getT() > dist)
                    finalColor += material->Shade(ray, hit, L, lightColor);
            }
        }
        else if (dynamic_cast <ReflectiveMaterial *> (hit.getMaterial())) {
            Vector3f direction = getReflectDir(ray.getDirection(), hit.getNormal());
            finalColor += tracingWhitted(Ray(point, direction), Parser, rate * hit.getColor());
        }
        else if (dynamic_cast <RefractiveMaterial *> (hit.getMaterial())) {
            RefractiveMaterial *material = dynamic_cast <RefractiveMaterial *> (hit.getMaterial());
            float weight = 0; Vector3f direction;
            getRefractDir(ray.getDirection(), hit.getNormal(), hit.getIsFront(), material->getN(), direction, weight);
            if (weight == 0)
                weight = 1, direction = getReflectDir(ray.getDirection(), hit.getNormal());
            else
                weight = 1 / weight;
            finalColor += weight * tracingWhitted(Ray(point, direction), Parser, rate * hit.getColor());
        }
    }
    return finalColor * rate;
}

Vector3f tracingWhitted(int x, int y, SceneParser &Parser) {
    Camera* camera = Parser.getCamera();
    return tracingWhitted(camera->generateRay(Vector2f(x, y)), Parser, Vector3f(1));
}
