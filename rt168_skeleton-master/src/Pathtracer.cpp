#include <algorithm>

#include <glm/glm.hpp>

#include "Integrator.h"

#include <iostream>
#include <stdexcept>

const float pi = 3.1415926535;
inline float random_double() 
{
    return rand() / (RAND_MAX + 1.0);
}

inline float random_float() 
{
    //  random in [-1, 1].
    return 2.0f * rand() / static_cast<float>(RAND_MAX) - 1.0f;
}

glm::vec3 ggxBRDF(glm::vec3 inRay, glm::vec3 outRay, glm::vec3 hitNormal, material_t hitMaterial)
{
    if(glm::dot(inRay, hitNormal) <= 0.0f || glm::dot(outRay, hitNormal)<= 0.0f)
    {
        return glm::vec3(0.0f);
    }
    inRay = glm::normalize(inRay);
    outRay = glm::normalize(outRay);
    glm::vec3 halfVecH = glm::normalize(inRay + outRay);
    // float theta_h = glm::acos(glm::dot(halfVecH, hitNormal));
    float dotProduct = glm::dot(halfVecH, hitNormal);
    dotProduct = glm::clamp(dotProduct, -1.0f, 1.0f);
    float theta_h = glm::acos(dotProduct);
    float term = glm::pow(hitMaterial.roughness, 2) + glm::pow(glm::tan(theta_h), 2);
    //microfacet distribution function
    float D = glm::pow(hitMaterial.roughness, 2.0f)/(pi * glm::pow(glm::cos(theta_h), 4) * glm::pow(term, 2));
    

    float theta_i = glm::acos(glm::dot(inRay, hitNormal));
    float theta_o = glm::acos(glm::dot(outRay, hitNormal));

    float G_i = 2.0f / (1.0f + glm::sqrt(1.0f + glm::pow(hitMaterial.roughness, 2.0f) * glm::pow(glm::tan(theta_i), 2.0f)));
    float G_o = 2.0f / (1.0f + glm::sqrt(1.0f + glm::pow(hitMaterial.roughness, 2.0f) * glm::pow(glm::tan(theta_o), 2.0f)));

    glm::vec3 Fresnel = hitMaterial.specular + (1.0f - hitMaterial.specular) * glm::pow((1.0f - glm::dot(inRay, halfVecH)), 5.0f);
    glm::vec3 ggxBRDF = Fresnel * G_i * G_o * D /(4.0f * glm::dot(inRay, hitNormal) * glm::dot(outRay, hitNormal));

    glm::vec3 brdf = hitMaterial.diffuse/pi + ggxBRDF;
    
    
    return brdf;
}

float ggxPDF(glm::vec3 inRay, glm::vec3 outRay, glm::vec3 hitNormal, material_t hitMaterial)
{
    glm::vec3 halfVecH = glm::normalize(inRay + outRay);
    // float theta_h = glm::acos(glm::dot(halfVecH, hitNormal));
    float dotProduct = glm::dot(halfVecH, hitNormal);
    dotProduct = glm::clamp(dotProduct, -1.0f, 1.0f);
    float theta_h = glm::acos(dotProduct);
    
    float average_ks = (hitMaterial.specular.x + hitMaterial.specular.y + hitMaterial.specular.z)/3.0f;
    float average_kd = (hitMaterial.diffuse.x + hitMaterial.diffuse.y + hitMaterial.diffuse.z)/3.0f;
    float t;
    if(average_ks == 0.0f && average_kd == 0.0f)
    {
        t = 1.0f;
    }
    else
    {
        t = glm::max(0.25f, average_ks/(average_kd + average_ks));
    }
    float term = glm::pow(hitMaterial.roughness, 2) + glm::pow(glm::tan(theta_h), 2);
    //microfacet distribution function
    float D = glm::pow(hitMaterial.roughness, 2.0f)/(pi * glm::pow(glm::cos(theta_h), 4) * glm::pow(term, 2));
    float pdf = (1.0f - t) * glm::dot(hitNormal, inRay)/pi 
                + t * (D * glm::dot(hitNormal, halfVecH))/(4 * glm::dot(halfVecH, inRay));

    return pdf;

}
glm::vec3 ggxOmega_i(material_t hitMaterial, glm::vec3 hitNormal, glm::vec3 direction)
{
    float average_ks = (hitMaterial.specular.x + hitMaterial.specular.y + hitMaterial.specular.z)/3.0f;
    float average_kd = (hitMaterial.diffuse.x + hitMaterial.diffuse.y + hitMaterial.diffuse.z)/3.0f;
    float t;

    if(average_ks == 0.0f && average_kd == 0.0f)
    {
        t = 1.0f;
    }
    else
    {
        t = glm::max(0.25f, average_ks/(average_kd + average_ks));
    }
    //std::cout << t << std::endl;
    
    
    //float r1 = random_double();

    float phi = 2.0f * pi * random_double();
    //float theta;
    glm::vec3 w = glm::normalize(hitNormal);
    glm::vec3 omega_i;
    if(random_double() > t)
    {
        float theta_d = glm::acos(glm::sqrt(random_double()));
        glm::vec3 sample_S = glm::vec3(glm::cos(phi)* glm::sin(theta_d),
                            glm::sin(phi) * glm::sin(theta_d),
                            glm::cos(theta_d));
        glm::vec3 up_vector_a = glm::vec3(0.0f, 1.0f, 0.0f);

        if (glm::abs(glm::dot(up_vector_a, w)) > 0.99f) 
        {
            up_vector_a = glm::vec3(1.0f, 0.0f, 0.0f); // Use an alternative vector
        }

        glm::vec3 u = glm::normalize(glm::cross(up_vector_a, w));
        glm::vec3 v = glm::normalize(glm::cross(w, u));
        omega_i = sample_S.x * u + sample_S.y * v + sample_S.z * w;
    }
    else
    {
        
        float theta_h  = glm::atan((hitMaterial.roughness * glm::sqrt(random_double()))/(glm::sqrt(1 - random_double())));
        glm::vec3 H = glm::vec3(glm::cos(phi)* glm::sin(theta_h),
                            glm::sin(phi) * glm::sin(theta_h),
                            glm::cos(theta_h));

        glm::vec3 up_vector_a = glm::vec3(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(up_vector_a, w)) > 0.99f) 
        {
            up_vector_a = glm::vec3(1.0f, 0.0f, 0.0f); // Use an alternative vector
        }

        glm::vec3 u = glm::normalize(glm::cross(up_vector_a, w));
        glm::vec3 v = glm::normalize(glm::cross(w, u));
        glm::vec3 Half = H.x * u + H.y * v + H.z * w;
        omega_i = glm::reflect(direction, Half);  
        //omega_i = direction - 2.0f * glm::dot(direction, H) * H;
    }
    return omega_i;

}

glm::vec3 Pathtracer::compute_omega_i (glm::vec3 hitNormal, const material_t& hitMaterial, glm::vec3 reflectedDirection, float t)
{
    glm::vec3 w = glm::normalize(hitNormal);

    float theta; 
    
    // choose on different theta depend on different importancesampling 
    if (_scene->importancesampling == "cosine")
    {
        theta = glm::acos(glm::sqrt(random_double()));
    }
    else if (_scene->importancesampling == "brdf")
    {

        if (random_double() <= t)
        {
            theta = glm::acos(glm::pow(random_double(),1.0f/(hitMaterial.shininess+1.0f)));
            w = glm::normalize(reflectedDirection);
        }
        else
        {
            theta = glm::acos(glm::sqrt(random_double()));
        } 
    }
    else
    {
        theta = glm::acos(random_double());
    }

    float phi = 2 * pi * random_double();
    glm::vec3 sample_S = glm::vec3(glm::cos(phi)* glm::sin(theta),
                            glm::sin(phi) * glm::sin(theta),
                            glm::cos(theta));

    
    glm::vec3 up_vector_a = glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::abs(glm::dot(up_vector_a, w)) > 0.99f) 
    {
        up_vector_a = glm::vec3(1.0f, 0.0f, 0.0f); // Use an alternative vector
    }

    glm::vec3 u = glm::normalize(glm::cross(up_vector_a, w));
    glm::vec3 v = glm::normalize(glm::cross(w, u));
    glm::vec3 omega_i = sample_S.x * u + sample_S.y * v + sample_S.z * w;

    return omega_i;
}


float Pathtracer::compute_pdf(glm::vec3 hitNormal, const material_t& hitMaterial, glm::vec3 reflectedDirection, float t, const glm::vec3 omega_i) 
{
    float pdf; 
    
    if (_scene->importancesampling == "cosine")
    {
        pdf = glm::dot(hitNormal, omega_i)/pi;
    }
    else if (_scene->importancesampling == "brdf")
    {
        pdf = (1.0f-t)*glm::dot(hitNormal, omega_i)/pi 
            + t * (hitMaterial.shininess + 1)
            *(float)std::pow(glm::dot(reflectedDirection, omega_i), hitMaterial.shininess)/(2.0f*pi);
    }
    else
    {
        pdf = 1/ (2.0f * pi); 
    }

    return pdf;

}



glm::vec3 Pathtracer::traceRay_NEE(glm::vec3 origin, glm::vec3 direction, int curdepth){
    if(curdepth<=0)
    {
        return glm::vec3(0.0f);
    }
    // std::cout<<"depth: " << curdepth << std::endl;

    glm::vec3 outputColor = glm::vec3(0.0f);
    
    glm::vec3 directcolor= glm::vec3(0.0f);

    glm::vec3 indirectcolor = glm::vec3(0.0f);
    glm::vec3 T = glm::vec3(1.0f);
    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);

    if (hit) 
    {   
        // std::cout << "depth: " << depth << std::endl;
        if (hitMaterial.if_light == false)
        {   
            glm::vec3 reflectedDirection = glm::normalize(direction - 2*dot(direction, hitNormal)*hitNormal);
            float average_ks = (hitMaterial.specular.x + hitMaterial.specular.y + hitMaterial.specular.z)/3.0f;
            float average_kd = (hitMaterial.diffuse.x + hitMaterial.diffuse.y + hitMaterial.diffuse.z)/3.0f;
            float t = average_ks/(average_kd + average_ks);
            for (const quadLight_t light: _scene->quadLights) 
            {
                
                //const quadLight_t light = _scene->quadLights[0];
                glm::vec3 vertexA = light.quadLightA;
                glm::vec3 lightsurNormal = glm::normalize(glm::cross(light.quadLightAB, light.quadLightAC));
                float Area = glm::length(glm::cross(light.quadLightAB, light.quadLightAC));

                glm::vec3 temp = glm::vec3(0.0f,0.0f,0.0f);
                glm::vec3 randompoint;


                randompoint = vertexA + random_double() * light.quadLightAB + random_double()* light.quadLightAC;
                    
                glm::vec3 sampleIncidentDir = glm::normalize((randompoint - hitPosition)/glm::length(randompoint - hitPosition));
                glm::vec3 LBRDF;
                float BRDF_pdf;
                if(hitMaterial.brdf=="ggx")
                {
                    LBRDF = ggxBRDF(glm::normalize(sampleIncidentDir),glm::normalize(-direction), hitNormal, hitMaterial);
                    
                    BRDF_pdf = ggxPDF(glm::normalize(sampleIncidentDir),glm::normalize(-direction), hitNormal, hitMaterial);

                }
                else
                {
                    LBRDF = hitMaterial.diffuse/pi + hitMaterial.specular * ((hitMaterial.shininess + 2)/(2*pi)) * (float)std::pow(glm::dot(reflectedDirection, sampleIncidentDir), hitMaterial.shininess);
                    BRDF_pdf = compute_pdf(hitNormal, hitMaterial, reflectedDirection, t, sampleIncidentDir); 
                }
                glm::vec3 toLight = randompoint - hitPosition;
                float R = glm::length(randompoint - hitPosition);
                float pdf_nee;
                if(glm::dot(toLight,lightsurNormal )<0.0f)
                {
                    pdf_nee = 0.0f;
                }
                else
                {
                    pdf_nee = R*R/(Area * glm::abs(glm::dot(lightsurNormal, sampleIncidentDir)));

                }
                float lightDistance = glm::length(toLight);
                toLight /= lightDistance;
                
                //float pdf_nee = R*R/(Area * glm::abs(glm::dot(lightsurNormal, sampleIncidentDir)));
                pdf_nee/=_scene->quadLights.size();

                float weight = glm::pow(pdf_nee, 2.0f)/(glm::pow(pdf_nee, 2.0f) + glm::pow(BRDF_pdf, 2.0f));
                
                bool occluded = _scene->castOcclusionRay(hitPosition, toLight, lightDistance);
                if(!occluded)
                {
                    
                    temp = LBRDF * glm::dot(sampleIncidentDir,hitNormal) * (glm::dot(lightsurNormal, sampleIncidentDir))/(R*R);
                    if(_scene->NEE == "mis")
                    {
                        temp *= weight;
                    }
                    
                }
                 
                directcolor = light.intensity * Area *  temp;
                outputColor += directcolor;
            }
            glm::vec3 omega_i;
            glm::vec3 BRDF;
            float pdf;
            if(hitMaterial.brdf == "ggx")
            {
                omega_i = ggxOmega_i(hitMaterial, hitNormal, glm::normalize(direction));
                BRDF = ggxBRDF(glm::normalize(omega_i),glm::normalize(-direction), hitNormal, hitMaterial);

                pdf = ggxPDF(glm::normalize(omega_i),glm::normalize(-direction), hitNormal, hitMaterial);
            }
            else
            {
                omega_i = compute_omega_i(hitNormal,hitMaterial,reflectedDirection,t);
                BRDF = hitMaterial.diffuse/pi + hitMaterial.specular 
                                * ((hitMaterial.shininess + 2.0f)/(2.0f*pi)) 
                                * (float)std::pow(glm::dot(reflectedDirection, omega_i), hitMaterial.shininess);
                pdf = compute_pdf(hitNormal, hitMaterial, reflectedDirection, t, omega_i); 
            }
            
            T = BRDF/pdf * glm::dot(hitNormal, omega_i);

                
            if(_scene->RR)
            {

                float q = (float)1.0f - glm::min(glm::max(T.x, glm::max(T.y, T.z)), 1.0f);
                
                if (random_double() > q)
                {       
                    T *= 1.0f/(1.0f-q);
                    
                }
                else
                {
                    return outputColor;
                }
            }

            glm::vec3 indirectRadiance = traceRay_NEE(hitPosition, omega_i, curdepth - 1);
            indirectcolor = T * indirectRadiance;
            
            
        }
        outputColor += indirectcolor;
    }

    return outputColor;
}

glm::vec3 Pathtracer::traceRay(glm::vec3 origin, glm::vec3 direction, int curdepth)
{   
    if(curdepth<=0)
    {
        // std::cout << "test" << std::endl;
        return glm::vec3(0.0f);
    }
    // std::cout<<"depth: " << curdepth << std::endl;

    glm::vec3 outputColor = glm::vec3(0.0f);
    
    glm::vec3 directcolor= glm::vec3(0.0f);

    glm::vec3 indirectcolor = glm::vec3(0.0f);
    glm::vec3 T = glm::vec3(1.0f);
    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);


    if (hit) 
    {   
        // std::cout << "depth: " << depth << std::endl;
        if (hitMaterial.if_light == true)
        {
            // if(_scene->NEE == "off")
            // {
                if (glm::dot(direction,hitNormal) < 0.0f)
                {
                    indirectcolor = glm::vec3(0.0f);
                }
                else
                {
                    indirectcolor = hitMaterial.is_light_intensity;
                }
                //std::cout << "?" << std::endl;
                
            // }
        }
        else
        {   
            glm::vec3 reflectedDirection = glm::normalize(direction - 2*dot(direction, hitNormal)*hitNormal);               

            float average_ks = (hitMaterial.specular.x + hitMaterial.specular.y + hitMaterial.specular.z)/3.0f;
            float average_kd = (hitMaterial.diffuse.x + hitMaterial.diffuse.y + hitMaterial.diffuse.z)/3.0f;
            float t = average_ks/(average_kd + average_ks);
            
            glm::vec3 omega_i;
            glm::vec3 BRDF;
            float pdf;

            if(hitMaterial.brdf == "ggx")
            {
                omega_i = ggxOmega_i(hitMaterial, hitNormal,direction);
                BRDF = ggxBRDF(glm::normalize(omega_i),glm::normalize(-direction), hitNormal, hitMaterial);

                pdf = ggxPDF(glm::normalize(omega_i), -direction, hitNormal, hitMaterial);
                
            }
            else
            {
                omega_i = compute_omega_i(hitNormal,hitMaterial,reflectedDirection,t);
                BRDF = hitMaterial.diffuse/pi + hitMaterial.specular 
                                * ((hitMaterial.shininess + 2.0f)/(2.0f*pi)) 
                                * (float)std::pow(glm::dot(reflectedDirection, omega_i), hitMaterial.shininess);
                pdf = compute_pdf(hitNormal, hitMaterial, reflectedDirection, t, omega_i); 
                
            }
            // if(std::isnan(BRDF.x) || std::isnan(pdf))
            // {
            //     std::cout << "nan value!" << std::endl;
            // }

            
            float pdf_light = 0.0f;
            
            glm::vec3 hitPosition2;
            glm::vec3 hitNormal2;
            material_t hitMaterial2;
            bool hit = _scene->castRay(hitPosition, omega_i, &hitPosition2, &hitNormal2, &hitMaterial2);
            //float R = glm::length(hitPosition2 - hitPosition);
            if(hit)
            {
                
                if(hitMaterial2.if_light)
                {
                    glm::vec3 toLight = hitPosition2 - hitPosition;
                    float R = glm::length(toLight);
                    glm::vec3 lightsurNormal = hitMaterial2.lightsurNormal;

                    
                    pdf_light = R * R/(hitMaterial2.Area * glm::abs(glm::dot(lightsurNormal, omega_i)));
                    
                }
                else
                {
                    pdf_light = 0.0f;
                }
            }
            
            pdf_light /= _scene->quadLights.size();

            
        
                

            //std::cout << BRDF_pdf << std::endl;
            //float brdf_pdf = ggxPDF(glm::normalize(omega_i),glm::normalize(-direction), hitNormal, hitMaterial);
            float weight = glm::pow(pdf, 2.0f) / (glm::pow(pdf, 2.0f) + glm::pow(pdf_light, 2.0f));
            // if(weight != 1.0f)
            // {
            //     std::cout <<"weight"<< weight << std::endl;
            // }
            
            
            T = BRDF/pdf * glm::dot(hitNormal, omega_i);
            if(_scene->NEE == "mis")
            {
                T *= weight;
            }
                
            if(_scene->RR)
            {

                float q = (float)1.0f - glm::min(glm::max(T.x, glm::max(T.y, T.z)), 1.0f);
                
                if (random_double() > q)
                {       
                    T *= 1.0f/(1.0f-q);
                    
                }
                else
                {
                    return directcolor;
                }
            }

            glm::vec3 indirectRadiance; 

            indirectRadiance = traceRay(hitPosition, omega_i, curdepth - 1);
            
        
            indirectcolor = T * indirectRadiance;
            
        }

        outputColor += indirectcolor;

    }

    return outputColor;
}

glm::vec3 Pathtracer::traceRay(glm::vec3 origin, glm::vec3 direction)
{
    if(_scene->NEE == "on")
    {
        glm::vec3 hitPosition;
        glm::vec3 hitNormal;
        material_t hitMaterial;
        bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);
        glm::vec3 emission = glm::vec3(0.0f);
        if (hit) {
            if (hitMaterial.if_light == true)
            {
                if (glm::dot(direction,hitNormal)<0){
                    emission = glm::vec3(0.0f);
                }else{
                    emission = hitMaterial.is_light_intensity;
                }
                
            }
        }

        return emission + traceRay_NEE(origin, direction, _scene->maxDepth);
    }
    else if(_scene->NEE == "mis")
    {
        //float NEE_weight = glm::pow(NEE_pdf, 2.0f)/(glm::pow(NEE_pdf, 2.0f) + glm::pow(BRDF_pdf, 2.0f));
        //std::cout << NEE_pdf << std::endl;
        //float BRDF_weight = 1.0f - NEE_weight;
        //return NEE_weight * traceRay_NEE(origin, direction, _scene->maxDepth) + BRDF_weight * traceRay(origin, direction, _scene->maxDepth + 1);
        return traceRay_NEE(origin, direction, _scene->maxDepth) + traceRay(origin, direction, _scene->maxDepth + 1);
    }
    else
    {
        return traceRay(origin, direction, _scene->maxDepth + 1);
    }

    
}