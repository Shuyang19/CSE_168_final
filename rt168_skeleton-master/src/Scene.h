#pragma once

#include <string>
#include <vector>
#include <limits>

#include <glm/glm.hpp>
#include <embree3/rtcore.h>

struct camera_t {
    glm::vec3 origin;
    glm::vec3 imagePlaneTopLeft;
    glm::vec3 pixelRight;
    glm::vec3 pixelDown;
};

struct material_t {
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    glm::vec3 emission;
    glm::vec3 ambient;

    bool if_light;
    glm::vec3 is_light_intensity;
    std::string brdf;
    float roughness;
    float Area;
    glm::vec3 lightsurNormal;
};
struct quadLight_t {
    glm::vec3 quadLightA;
    glm::vec3 quadLightAB;
    glm::vec3 quadLightAC;
    glm::vec3 intensity;
};

struct directionalLight_t {
    glm::vec3 toLight;
    glm::vec3 brightness;
};

struct pointLight_t {
    glm::vec3 point;
    glm::vec3 brightness;
    glm::vec3 attenuation;
};

class Scene {

public:

    glm::uvec2 imageSize;
    int maxDepth;
    std::string outputFileName;
    camera_t camera;
    std::vector<glm::mat3> sphereNormalTransforms;
    std::vector<material_t> sphereMaterials;
    std::vector<material_t> triMaterials;
    std::vector<directionalLight_t> directionalLights;
    std::vector<quadLight_t> quadLights;
    std::vector<pointLight_t> pointLights;
    RTCScene embreeScene;

    // // added integrator_type 
    // enum integrator_type {RayTracer, Analyticdirect, dirct} ;
    // integrator_type inte_type; 
    /*
        0 : raytracer 
        1 : direct  
        2 : analyticdirect 
    */
    int inte_type;
    int lightsample; 

    /*
        0: off
        1: on
    */
    bool lightstratify;

    int spp;
    std::string NEE;
    bool RR; 
    std::string importancesampling;

    // std::string brdf;
    // float roughness;
    float gamma;



    bool castRay(
        glm::vec3 origin,
        glm::vec3 direction,
        glm::vec3* hitPosition,
        glm::vec3* hitNormal,
        material_t* hitMaterial) const;

    bool castOcclusionRay(
        glm::vec3 origin,
        glm::vec3 direction,
        float maxDistance = std::numeric_limits<float>::infinity()) const;

};
