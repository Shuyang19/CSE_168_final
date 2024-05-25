#include <algorithm>
#include <glm/glm.hpp>
#include "Integrator.h"
#include <iostream>
#include <stdexcept>

const float pi = 3.1415926535;
inline float random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

glm::vec3 directIntegrator::computeShading(
    glm::vec3 incidentDirection,
    glm::vec3 toLight,
    glm::vec3 normal,
    glm::vec3 lightBrightness,
    const material_t& material)
{
    glm::vec3 diffuseReflectance = material.diffuse * std::max(glm::dot(normal, toLight), 0.0f);
    glm::vec3 halfAngle = glm::normalize(toLight + incidentDirection);
    glm::vec3 specularReflectance =
        material.specular
        * std::pow(std::max(glm::dot(normal, halfAngle), 0.0f), material.shininess);
    return lightBrightness * (diffuseReflectance + specularReflectance);
}



glm::vec3 directIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction)
{
    
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);

    if (hit) {
        if (hitMaterial.if_light == true){
            outputColor = hitMaterial.is_light_intensity;

        }else{

            /* for point light */
            for (const pointLight_t light: _scene->pointLights){
                // std::cout << "yes here " << std::endl;
                glm::vec3 lightpos = light.point; 
                float R = glm::length(lightpos - hitPosition);

                glm::vec3 sampleIncidentDir = glm::normalize((lightpos - hitPosition)/glm::length(lightpos - hitPosition));
                glm::vec3 reflectedDirection = glm::normalize(direction - 2*dot(direction, hitNormal)*hitNormal);

                glm::vec3 LBRDF = hitMaterial.diffuse/pi + 
                        hitMaterial.specular * ((hitMaterial.shininess + 2)/(2*pi)) * 
                        (float)std::pow(glm::dot(reflectedDirection, sampleIncidentDir), hitMaterial.shininess);
                
                glm::vec3 toLight = lightpos - hitPosition;
                float lightDistance = glm::length(toLight);
                toLight /= lightDistance;
                
                bool occluded = _scene->castOcclusionRay(hitPosition, toLight, lightDistance);
                if(!occluded){
                    // std::cout << "output color " << outputColor.x << " " << outputColor.y << " " << outputColor.z << std::endl; 
                    outputColor += light.brightness * LBRDF * glm::dot(sampleIncidentDir,hitNormal) /(R*R);  
                    // std::cout << "output color after:" << outputColor.x << " " << outputColor.y << " " << outputColor.z << std::endl;       
                }

            }

            for (const directionalLight_t light: _scene->directionalLights)
            {
                bool occluded = _scene->castOcclusionRay(hitPosition, -light.toLight);
                glm::vec3 direction = glm::normalize(light.toLight);
                //glm::vec3 direction = light.toLight;

                glm::vec3 reflectedDirection = glm::normalize(direction - 2*dot(direction, hitNormal)*hitNormal);
                glm::vec3 BRDF = hitMaterial.diffuse/pi + hitMaterial.specular * ((hitMaterial.shininess + 2)/(2*pi)) * (float)std::pow(glm::max(glm::dot(reflectedDirection, direction), 0.0f), hitMaterial.shininess);
                //glm::vec3 E = (float)2 * pi * light.brightness;
                glm::vec3 E = (float)2 * pi* light.brightness;
                if(!occluded)
                {
                    outputColor += E * BRDF * glm::max(glm::dot(hitNormal, -direction), 0.0f);
                }
            }

            for (const quadLight_t light: _scene->quadLights) 
            {
                glm::vec3 vertexA = light.quadLightA;

                float N = glm::sqrt(_scene->lightsample);

                glm::vec3 lightsurNormal = glm::normalize(glm::cross(light.quadLightAB, light.quadLightAC));
                float Area = glm::length(glm::cross(light.quadLightAB, light.quadLightAC));

                glm::vec3 temp = glm::vec3(0.0f,0.0f,0.0f);

                for(int i = 0; i < N; i++)
                {
                    for(int j = 0; j < N; j++)
                    {
                        glm::vec3 startpoint;
                        glm::vec3 randompoint;

                        float u1 = random_double();
                        float u2 = random_double();
                       
                        /* for reduce variance */
                        /*
                            glm::vec3 random_point_for_V;
                            startpoint = vertexA + light.quadLightAB * (float)i/(float)N + light.quadLightAC * (float)j/(float)N;
                            randompoint = startpoint +  1/(N*2) * light.quadLightAB +  1/(N*2) * light.quadLightAC;
                            random_point_for_V = vertexA + u1 * light.quadLightAB + u2* light.quadLightAC;
                            
                        */

                        /* for normal sampling */ 
                        if(_scene->lightstratify == true)
                        {
                            startpoint = vertexA + light.quadLightAB * (float)i/(float)N + light.quadLightAC * (float)j/(float)N;
                            randompoint = startpoint + u1 * 1/N * light.quadLightAB + u2 * 1/N * light.quadLightAC;
                                                   }
                        else
                        {
                            randompoint = vertexA + u1 * light.quadLightAB + u2* light.quadLightAC;
                        }              
                        
                        
                        glm::vec3 sampleIncidentDir = glm::normalize((randompoint - hitPosition)/glm::length(randompoint - hitPosition));

                        glm::vec3 reflectedDirection = glm::normalize(direction - 2*dot(direction, hitNormal)*hitNormal);

                        glm::vec3 LBRDF = hitMaterial.diffuse/pi + hitMaterial.specular * ((hitMaterial.shininess + 2)/(2*pi)) * (float)std::pow(glm::dot(reflectedDirection, sampleIncidentDir), hitMaterial.shininess);
                        
                        // for reduce variance 
                        // glm::vec3 toLight  = random_point_for_V -hitPosition;

                        glm::vec3 toLight = randompoint - hitPosition;
                        float lightDistance = glm::length(toLight);
                        toLight /= lightDistance;
                        float R = glm::length(randompoint - hitPosition);
                        // std::cout << R << std::endl;
                        
                        bool occluded = _scene->castOcclusionRay(hitPosition, toLight, lightDistance);
                        if(!occluded)
                        {
                            temp += LBRDF * glm::dot(sampleIncidentDir,hitNormal) * (glm:: dot(lightsurNormal, sampleIncidentDir))/(R*R);
                        
                            
                        }
                        
                    }
                }

                outputColor = outputColor + light.intensity * Area* temp/(float)_scene->lightsample;
            
            }
        }
       
    }

    

    return outputColor;
}

