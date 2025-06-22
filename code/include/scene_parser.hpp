/*
原创性：独立实现
*/

#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <cassert>
#include <vecmath.h>

class Camera;
class Light;
class Texture;
class Material;
class Object3D;
class Group;
class Sphere;
class Plane;
class Triangle;
class Mesh;
class Curve;
class RevSurface;
class Transform;

#define MAX_PARSER_TOKEN_LENGTH 1024

class SceneParser {
public:

    SceneParser() = delete;
    SceneParser(const char *filename);

    ~SceneParser();

    Camera *getCamera() const {
        return camera;
    }

    int getModel() {
        return model;
    }

    int getOmpThreads() {
        return omp_threads;
    }

    Vector3f getBackgroundColor() const {
        return background_color;
    }

    int getSampling() {
        return sampling;
    }

    int getAntialias() {
        return antialias;
    }

    int getSPP() {
        return SPP;
    }

    float getGamma() {
        return gamma;
    }

    float getrrProb() {
        return rrProb;
    }

    float getTmin() {
        return tmin;
    }

    int getNumLights() const {
        return num_lights;
    }

    Light *getLight(int i) const {
        assert(i >= 0 && i < num_lights);
        return lights[i];
    }

    int getNumTextures() const {
        return num_textures;
    }

    Texture *getTexture(int i) const {
        assert(i >= 0 && i < num_textures);
        return textures[i];
    }

    int getNumMaterials() const {
        return num_materials;
    }

    Material *getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }

    Group *getGroup() const {
        return group;
    }

private:

    void parseFile();
    void parsePerspectiveCamera();
    void parseModel();
    void parseLights();
    Light *parsePointLight();
    Light *parseDirectionalLight();
    Light *parseRectLight();
    Light *parseCircleLight();
    void parseTextures();
    void parseMaterials();
    Material *parsePhongMaterial();
    Material *parseReflectiveMaterial();
    Material *parseRefractiveMaterial();
    Material *parseFresnelMaterial();
    Material *parsePhongBRDFMaterial();
    Material *parseCookBRDFMaterial();
    Material *parseWardBRDFMaterial();
    Object3D *parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group *parseGroup();
    Sphere *parseSphere();
    Plane *parsePlane();
    Triangle *parseTriangle();
    Mesh *parseTriangleMesh();
    Curve *parseBezierCurve();
    Curve *parseBsplineCurve();
    RevSurface *parseRevSurface();
    Transform *parseTransform();

    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);

    Vector3f readVector3f();

    float readFloat();
    int readInt();

    FILE *file;
    Camera *camera;
    int model; // 0: Whitted 1: Monte Carlo
    int omp_threads;
    Vector3f background_color; // for Whitted
    int sampling; // for Monte Carlo
    /*
    for Monte Carlo:
        0 = uniform
        1 = NEE-uniform
        2 = NEE-cos-weighted
        3 = NEE-BRDF
        4 = MIS
    */
    int antialias;
    /*
        0 = none
        1 = Hammersley (only Monte Carlo)
        2 = FAXX
        3 = Hammersley + FAXX
    */
    int SPP; // for Monte Carlo
    float gamma;
    float rrProb; // for Monte Carlo
    float tmin;
    int num_lights;
    Light **lights;
    int num_textures;
    int num_materials;
    Texture **textures;
    Material **materials;
    Material *current_material;
    Group *group;
};

#endif // SCENE_PARSER_H
