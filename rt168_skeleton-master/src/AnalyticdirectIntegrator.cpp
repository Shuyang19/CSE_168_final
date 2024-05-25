#include <algorithm>

#include <glm/glm.hpp>

#include "Integrator.h"

#include <iostream>
#include <stdexcept>

const float pi = 3.1415926535;

float Computetheta(glm::vec3 vertex, glm::vec3 nextvertex, glm::vec3 hitpoint)
{
    float theta = glm::dot(glm::normalize(vertex - hitpoint),glm::normalize(nextvertex - hitpoint));

    theta = glm::acos(theta);
    return theta;
}

glm::vec3 ComputeTau(glm::vec3 vertex, glm::vec3 nextvertex, glm::vec3 hitpoint)
{
    glm::vec3 Tau = glm::normalize(glm::cross((vertex - hitpoint), (nextvertex - hitpoint)));
    return Tau;
}



glm::vec3 AnalyticdirectIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction)
{
    
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);
    if (hit) {

        //outputColor += hitMaterial.ambient;
       for (const quadLight_t light : _scene->quadLights) {

            glm::vec3 vertexA = light.quadLightA;
            glm::vec3 vertexB = light.quadLightA + light.quadLightAB;
            glm::vec3 vertexC = light.quadLightA + light.quadLightAC;
            glm::vec3 vertexD = light.quadLightA + light.quadLightAB + light.quadLightAC;

            glm::vec3 f = hitMaterial.diffuse/pi;
            
            glm::vec3 IrradianceVecrtor1 = Computetheta(vertexA, vertexB, hitPosition) * ComputeTau(vertexA, vertexB, hitPosition);
            glm::vec3 IrradianceVecrtor2 = Computetheta(vertexB, vertexD, hitPosition) * ComputeTau(vertexB, vertexD, hitPosition);
            glm::vec3 IrradianceVecrtor3 = Computetheta(vertexD, vertexC, hitPosition) * ComputeTau(vertexD, vertexC, hitPosition);
            glm::vec3 IrradianceVecrtor4 = Computetheta(vertexC, vertexA, hitPosition) * ComputeTau(vertexC, vertexA, hitPosition);


            glm::vec3 IrradianceVecrtor = 0.5f * (IrradianceVecrtor1+ IrradianceVecrtor2 + IrradianceVecrtor3 + IrradianceVecrtor4);
            glm::vec3 Diffusesur = f * light.intensity * glm::dot(IrradianceVecrtor, hitNormal);
            
            outputColor += Diffusesur;
       }
       
    }
    return outputColor;
}
