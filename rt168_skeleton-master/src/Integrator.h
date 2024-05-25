#pragma once

#include <glm/glm.hpp>

#include "Scene.h"

class Integrator {

protected:

    Scene* _scene;

public:

    void setScene(Scene* scene)
    {
        _scene = scene;
    }

    virtual glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction) = 0;

};


class RayTracerIntegrator : public Integrator {

private:

    glm::vec3 computeShading(
        glm::vec3 incidentDirection,
        glm::vec3 toLight,
        glm::vec3 normal,
        glm::vec3 lightBrightness,
        const material_t& material);

    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction, int depth);

public:

    virtual glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction);

};

class  Pathtracer : public Integrator {

private:

    //glm::vec3 generateRefRay(glm::vec3 origin, glm::vec3 direction, glm::vec3 hitNormal, material_t hitMaterial);
    //glm::vec3 geneRefRay(glm::vec3 direction, glm::vec3 hitNormal, material_t hitMaterial);
    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction, int depth);
    glm::vec3 traceRay_NEE(glm::vec3 origin, glm::vec3 direction, int curdepth); 
    glm::vec3 compute_omega_i (
        glm::vec3 hitNormal, 
        const material_t& hitMaterial,
        glm::vec3 reflectedDirection, 
        float t);

    float compute_pdf(
        glm::vec3 hitNormal, 
        const material_t& hitMaterial,
        glm::vec3 reflectedDirection, 
        float t, 
        const glm::vec3 omega_i);

public:

    virtual glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction);

};


class  AnalyticdirectIntegrator : public Integrator {

private:

    // glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction, int depth);

public:

    virtual glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction);

};

// not sure 
class  directIntegrator : public Integrator {

private:

    glm::vec3 computeShading(
        glm::vec3 incidentDirection,
        glm::vec3 toLight,
        glm::vec3 normal,
        glm::vec3 lightBrightness,
        const material_t& material);

    // glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction, int depth);

public:

    virtual glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction);

};

