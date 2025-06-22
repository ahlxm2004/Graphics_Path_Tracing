/*
原创性：独立实现
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>

#include "scene_parser.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "material.hpp"
#include "object3d.hpp"
#include "group.hpp"
#include "mesh.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "triangle.hpp"
#include "curve.hpp"
#include "revsurface.hpp"
#include "transform.hpp"

#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

SceneParser::SceneParser(const char *filename) {

    // initialize some reasonable default values
    group = nullptr;
    camera = nullptr;
    num_lights = 0;
    lights = nullptr;
    num_textures = 0;
    textures = nullptr;
    num_materials = 0;
    materials = nullptr;
    current_material = nullptr;
    omp_threads = 1;
    antialias = 0;
    gamma = 1;
    background_color = Vector3f::ZERO;
    tmin = 1e-4;

    // parse the file
    assert(filename != nullptr);
    const char *ext = &filename[strlen(filename) - 4];

    if (strcmp(ext, ".txt") != 0) {
        printf("wrong file name extension\n");
        exit(0);
    }
    file = fopen(filename, "r");

    if (file == nullptr) {
        printf("cannot open scene file\n");
        exit(0);
    }
    parseFile();
    fclose(file);
    file = nullptr;

    if (num_lights == 0) {
        printf("WARNING:    No lights specified\n");
    }
}

SceneParser::~SceneParser() {

    delete group;
    delete camera;

    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
    for (i = 0; i < num_lights; i++) {
        delete lights[i];
    }
    delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        } else if (!strcmp(token, "Model")) {
            parseModel();
        } else if (!strcmp(token, "Lights")) {
            parseLights();
        } else if (!strcmp(token, "Textures")) {
            parseTextures();
        } else if (!strcmp(token, "Materials")) {
            parseMaterials();
        } else if (!strcmp(token, "Group")) {
            group = parseGroup();
        } else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "up"));
    Vector3f up = readVector3f();
    getToken(token);
    assert (!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token);
    assert (!strcmp(token, "width"));
    int width = readInt();
    getToken(token);
    assert (!strcmp(token, "height"));
    int height = readInt();
    getToken(token);
    assert (!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, width, height, angle_radians);
}

void SceneParser::parseModel() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "tracing")) {
            getToken(token);
            if (!strcmp(token, "Whitted")) model = 0;
            else if (!strcmp(token, "Monte-Carlo")) model = 1;
            else {
                printf("Unknown tracing model: '%s'\n", token);
                assert(0);
            }
        } else if (!strcmp(token, "sampling")) {
            getToken(token);
            if (!strcmp(token, "uniform")) sampling = 0;
            else if (!strcmp(token, "NEE-uniform")) sampling = 1;
            else if (!strcmp(token, "NEE-cos-weighted")) sampling = 2;
            else if (!strcmp(token, "NEE-BRDF")) sampling = 3;
            else if (!strcmp(token, "MIS")) sampling = 4;
            else {
                printf("Unknown sampling style: '%s'\n", token);
                assert(0);
            }
        } else if (!strcmp(token, "antialias")) {
            getToken(token);
            assert(!strcmp(token, "{"));
            while (true) {
                getToken(token);
                if (!strcmp(token, "Hammersley")) {
                    getToken(token);
                    if (!strcmp(token, "true"))
                        antialias |= 1;
                } else if (!strcmp(token, "FXAA")) {
                    getToken(token);
                    if (!strcmp(token, "true"))
                        antialias |= 2;
                } else {
                    assert(!strcmp(token, "}"));
                    break;
                }
            }
        } else if (!strcmp(token, "OMP")) {
            omp_threads = readInt();
        } else if (!strcmp(token, "background")) {
            background_color = readVector3f();
        } else if (!strcmp(token, "SPP")) {
            SPP = readInt();
        } else if (!strcmp(token, "gamma")) {
            gamma = readFloat();
        } else if (!strcmp(token, "rrProb")) {
            rrProb = readFloat();
        } else if (!strcmp(token, "tmin")) {
            tmin = readFloat();
        } else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light *[num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (strcmp(token, "DirectionalLight") == 0) {
            lights[count] = parseDirectionalLight();
        } else if (strcmp(token, "PointLight") == 0) {
            lights[count] = parsePointLight();
        } else if (strcmp(token, "RectLight") == 0) {
            lights[count] = parseRectLight();
        } else if (strcmp(token, "CircleLight") == 0) {
            lights[count] = parseCircleLight();
        } else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

Light *SceneParser::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

Light *SceneParser::parsePointLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "position"));
    Vector3f position = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new PointLight(position, color);
}

Light *SceneParser::parseRectLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    int w;
    float z, x1, x2, y1, y2;
    Vector3f color;
    while (true) {
        getToken(token);
        if (!strcmp(token, "normal")) {
            getToken(token);
            assert(strlen(token) == 2 && token[0] >= 'X' && token[0] <= 'Z');
            assert(token[1] == '+' || token[1] == '-');
            w = (token[1] == '+' ? 1 : -1) * (token[0] - 'X' + 1);
        } else if (!strcmp(token, "color")) {
            color = readVector3f();
        } else if (!strcmp(token, "position")) {
            z = readFloat();
            getToken(token);
            assert(strlen(token) == 1 && token[0] == abs(w) % 3 + 'X');
            x1 = readFloat(), x2 = readFloat();
            assert(x1 < x2);
            getToken(token);
            assert(strlen(token) == 1 && token[0] == (abs(w) + 1) % 3 + 'X');
            y1 = readFloat(), y2 = readFloat();
            assert(y1 < y2);
        } else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    return new RectLight(w, z, x1, y1, x2, y2, color);
}

Light *SceneParser::parseCircleLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    int w;
    float z, x, y, radius;
    Vector3f color;
    while (true) {
        getToken(token);
        if (!strcmp(token, "normal")) {
            getToken(token);
            assert(strlen(token) == 2 && token[0] >= 'X' && token[0] <= 'Z');
            assert(token[1] == '+' || token[1] == '-');
            w = (token[1] == '+' ? 1 : -1) * (token[0] - 'X' + 1);
        } else if (!strcmp(token, "color")) {
            color = readVector3f();
        } else if (!strcmp(token, "position")) {
            z = readFloat();
            getToken(token);
            assert(strlen(token) == 1 && token[0] == abs(w) % 3 + 'X');
            x = readFloat();
            getToken(token);
            assert(strlen(token) == 1 && token[0] == (abs(w) + 1) % 3 + 'X');
            y = readFloat();
        } else if (!strcmp(token, "radius")) {
            radius = readFloat();
        } else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    return new CircleLight(w, z, x, y, radius, color);
}

// ====================================================================
// ====================================================================

void SceneParser::parseTextures() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "numTextures"));
    num_textures = readInt();
    textures = new Texture *[num_textures];
    int count = 0;
    while (num_textures > count) {
        getToken(token);
        if (!strcmp(token, "Texture")) {
            getToken(token);
            textures[count] = new Texture();
            textures[count]->set(token);
        }
        else {
            printf("Unknown token in parseTextures: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

void SceneParser::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material *[num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Material") || !strcmp(token, "PhongMaterial")) {
            materials[count] = parsePhongMaterial();
        } else if (!strcmp(token, "ReflectiveMaterial")) {
            materials[count] = parseReflectiveMaterial();
        } else if (!strcmp(token, "RefractiveMaterial")) {
            materials[count] = parseRefractiveMaterial();
        } else if (!strcmp(token, "FresnelMaterial")) {
            materials[count] = parseFresnelMaterial();
        } else if (!strcmp(token, "PhongBRDFMaterial")) {
            materials[count] = parsePhongBRDFMaterial();
        } else if (!strcmp(token, "CookBRDFMaterial")) {
            materials[count] = parseCookBRDFMaterial();
        } else if (!strcmp(token, "WardBRDFMaterial")) {
            materials[count] = parseWardBRDFMaterial();
        } else {
            printf("Unknown token in parseMaterials: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

Material *SceneParser::parsePhongMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    filename[0] = 0;
    Vector3f diffuseColor(1), ambientColor(0), specularColor(0);
    float shininess = 0;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "diffuseColor") == 0) {
            diffuseColor = readVector3f();
        } else if (strcmp(token, "ambientColor") == 0) {
            ambientColor = readVector3f();
        } else if (strcmp(token, "specularColor") == 0) {
            specularColor = readVector3f();
        } else if (strcmp(token, "shininess") == 0) {
            shininess = readFloat();
        } else if (strcmp(token, "texture") == 0) {
            // Optional: read in texture and draw it.
            getToken(filename);
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    auto *answer = new PhongMaterial(ambientColor, diffuseColor, specularColor, shininess);
    return answer;
}

Material *SceneParser::parseReflectiveMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    float rate;
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    if (!strcmp(token, "rate"))
        rate = readFloat(), getToken(token);
    assert(!strcmp(token, "}"));
    return new ReflectiveMaterial(rate);
}

Material *SceneParser::parseRefractiveMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    float n, rate;
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "n"))
            n = readFloat();
        else if (!strcmp(token, "rate"))
            rate = readFloat();
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    return new RefractiveMaterial(n, rate);
}

Material *SceneParser::parseFresnelMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    float n, rate;
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "n"))
            n = readFloat();
        else if (!strcmp(token, "rate"))
            rate = readFloat();
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    return new FresnelMaterial(n, rate);
}

Material *SceneParser::parsePhongBRDFMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    float rho_d, rho_s, shininess;
    Vector3f color;
    Texture *texture = nullptr;
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "rho_d"))
            rho_d = readFloat();
        else if (!strcmp(token, "rho_s"))
            rho_s = readFloat();
        else if (!strcmp(token, "shininess"))
            shininess = readFloat();
        else if (!strcmp(token, "color"))
            color = readVector3f();
        else if (!strcmp(token, "texture"))
            texture = textures[readInt()];
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    return new PhongBRDFMaterial(rho_d, rho_s, shininess, color, texture);
}

Material *SceneParser::parseCookBRDFMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    float rho_d, rho_s, alpha;
    Vector3f F0, color;
    Texture *texture = nullptr;
  
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "rho_d"))
            rho_d = readFloat();
        else if (!strcmp(token, "rho_s"))
            rho_s = readFloat();
        else if (!strcmp(token, "alpha"))
            alpha = readFloat();
        else if (!strcmp(token, "F0"))
            F0 = readVector3f();
        else if (!strcmp(token, "color"))
            color = readVector3f();
        else if (!strcmp(token, "texture"))
            texture = textures[readInt()];
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }

    return new CookBRDFMaterial(rho_d, rho_s, alpha, F0, color, texture);
}

Material *SceneParser::parseWardBRDFMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    float rho_d, rho_s, alpha_x, alpha_y;
    Vector3f tangent0, color;
    Texture *texture = nullptr;
  
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "rho_d"))
            rho_d = readFloat();
        else if (!strcmp(token, "rho_s"))
            rho_s = readFloat();
        else if (!strcmp(token, "alpha_x"))
            alpha_x = readFloat();
        else if (!strcmp(token, "alpha_y"))
            alpha_y = readFloat();
        else if (!strcmp(token, "tangent0"))
            tangent0 = readVector3f();
        else if (!strcmp(token, "color"))
            color = readVector3f();
        else if (!strcmp(token, "texture"))
            texture = textures[readInt()];
        else {
            assert(!strcmp(token, "}"));
            break;
        }
    }

    return new WardBRDFMaterial(rho_d, rho_s, alpha_x, alpha_y, tangent0, color, texture);
}

// ====================================================================
// ====================================================================

Object3D *SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D *answer = nullptr;
    if (!strcmp(token, "Group")) {
        answer = (Object3D *) parseGroup();
    } else if (!strcmp(token, "Sphere")) {
        answer = (Object3D *) parseSphere();
    } else if (!strcmp(token, "Plane")) {
        answer = (Object3D *) parsePlane();
    } else if (!strcmp(token, "Triangle")) {
        answer = (Object3D *) parseTriangle();
    } else if (!strcmp(token, "TriangleMesh")) {
        answer = (Object3D *) parseTriangleMesh();
    } else if (!strcmp(token, "RevSurface")) {
        answer = (Object3D *) parseRevSurface();
    } else if (!strcmp(token, "Transform")) {
        answer = (Object3D *) parseTransform();
    } else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group *SceneParser::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));

    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numObjects"));
    int num_objects = readInt();

    auto *answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert (index >= 0 && index < getNumMaterials());
            current_material = getMaterial(index);
        } else {
            Object3D *object = parseObject(token);
            assert (object != nullptr);
            answer->addObject(count, object);

            count++;
        }
    }
    getToken(token);
    assert (!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere *SceneParser::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Sphere(center, radius, current_material);
}


Plane *SceneParser::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "normal"));
    Vector3f normal = readVector3f();
    getToken(token);
    assert (!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Plane(normal, offset, current_material);
}


Triangle *SceneParser::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "vertex0"));
    Vector3f v0 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "vertex1"));
    Vector3f v1 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "vertex2"));
    Vector3f v2 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Triangle(v0, v1, v2, current_material);
}


Mesh *SceneParser::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "obj_file"));
    getToken(filename);
    const char *ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    Mesh *answer = new Mesh(filename, current_material);
    getToken(token);
    if (!strcmp(token, "use_BVH")) {
        getToken(token);
        if (!strcmp(token, "true"))
            answer->buildBVH();
        getToken(token);
    }
    assert (!strcmp(token, "}"));
    return answer;
}


Curve *SceneParser::parseBezierCurve() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector <Vector3f> controls;
    while (true) {
        getToken(token);
        if (!strcmp(token, "[")) {
            controls.push_back(readVector3f());
            getToken(token);
            assert (!strcmp(token, "]"));
        } else if (!strcmp(token, "}")) {
            break;
        } else {
            printf("Incorrect format for BezierCurve!\n");
            exit(0);
        }
    }
    return new BezierCurve(controls);
}


Curve *SceneParser::parseBsplineCurve() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector <Vector3f> controls;
    while (true) {
        getToken(token);
        if (!strcmp(token, "[")) {
            controls.push_back(readVector3f());
            getToken(token);
            assert (!strcmp(token, "]"));
        } else if (!strcmp(token, "}")) {
            break;
        } else {
            printf("Incorrect format for BsplineCurve!\n");
            exit(0);
        }
    }
    return new BsplineCurve(controls);
}

RevSurface *SceneParser::parseRevSurface() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    Curve *profile;
    int step1, step2;
    bool isNewton = false;
    while (true) {
        getToken(token);
        if (!strcmp(token, "profile")) {
            getToken(token);
            if (!strcmp(token, "BezierCurve")) {
                profile = parseBezierCurve();
            } else if (!strcmp(token, "BsplineCurve")) {
                profile = parseBsplineCurve();
            } else {
                printf("Unknown profile type in parseRevSurface: '%s'\n", token);
                exit(0);
            }
        } else if (!strcmp(token, "step")) {
            step1 = readInt();
            step2 = readInt();
        } else if (!strcmp(token, "newton")) {
            getToken(token);
            if (!strcmp(token, "true"))
                isNewton = true;
        } else {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    return new RevSurface(profile, current_material, step1, step2, isNewton);
}


Transform *SceneParser::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix4f matrix = Matrix4f::identity();
    Object3D *object = nullptr;
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);

    while (true) {
        if (!strcmp(token, "Scale")) {
            Vector3f s = readVector3f();
            matrix = matrix * Matrix4f::scaling(s[0], s[1], s[2]);
        } else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix = matrix * Matrix4f::uniformScaling(s);
        } else if (!strcmp(token, "Translate")) {
            matrix = matrix * Matrix4f::translation(readVector3f());
        } else if (!strcmp(token, "XRotate")) {
            matrix = matrix * Matrix4f::rotateX(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "YRotate")) {
            matrix = matrix * Matrix4f::rotateY(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "ZRotate")) {
            matrix = matrix * Matrix4f::rotateZ(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "Rotate")) {
            getToken(token);
            assert (!strcmp(token, "{"));
            Vector3f axis = readVector3f();
            float degrees = readFloat();
            float radians = DegreesToRadians(degrees);
            matrix = matrix * Matrix4f::rotation(axis, radians);
            getToken(token);
            assert (!strcmp(token, "}"));
        } else if (!strcmp(token, "Matrix4f")) {
            Matrix4f matrix2 = Matrix4f::identity();
            getToken(token);
            assert (!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2(i, j) = v;
                }
            }
            getToken(token);
            assert (!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        } else {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            break;
        }
        getToken(token);
    }

    assert(object != nullptr);
    getToken(token);
    assert (!strcmp(token, "}"));
    return new Transform(matrix, object);
}

// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert (file != nullptr);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vector3f SceneParser::readVector3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        assert (0);
    }
    return Vector3f(x, y, z);
}


float SceneParser::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert (0);
    }
    return answer;
}


int SceneParser::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert (0);
    }
    return answer;
}
