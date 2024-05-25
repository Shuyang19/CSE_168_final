#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <glm/glm.hpp>

#include <iostream>
#include <random>

#include "Scene.h"
#include "Integrator.h"

#include "RenderPool.h"

RenderJob::RenderJob(glm::uvec2 startPixel, glm::uvec2 windowSize)
    : startPixel(startPixel),
      windowSize(windowSize),
      _result(windowSize.x * windowSize.y)
{
}

void RenderJob::render(Scene* scene, Integrator* integrator)
{
    for (size_t wy = 0; wy < windowSize.y; wy++) {
        size_t y = startPixel.y + wy;
        for (size_t wx = 0; wx < windowSize.x; wx++) {
            float spp_trytry  = scene->spp;
            glm::vec3 temp = glm::vec3(0.0f);
            for (int www = 0; www<spp_trytry ; www++ ){
                size_t x = startPixel.x + wx;
                // get random number 
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dis(0.0, 1.0);
                float random_float1 = dis(gen);
                float random_float2 = dis(gen);
                glm::vec3 target =
                    scene->camera.imagePlaneTopLeft
                    + (x + random_float1) * scene->camera.pixelRight
                    + (y + random_float2) * scene->camera.pixelDown;
                glm::vec3 direction = glm::normalize(target - scene->camera.origin);
                temp += integrator->traceRay(scene->camera.origin, direction);
                
            }
            temp = temp/spp_trytry;
            temp.x = glm::pow(temp.x,1/scene->gamma);
            temp.y = glm::pow(temp.y,1/scene->gamma);
            temp.z = glm::pow(temp.z,1/scene->gamma);
            _result[wy * windowSize.x + wx] = temp;
            //std::cout << "Area" <<glm::length(glm::cross(glm::vec3(0, 0, 0.03333), glm::vec3(0.03333, 0, 0)))<< std::endl;

                

        }
    }
}

std::vector<glm::vec3> RenderJob::getResult()
{
    return std::move(_result);
}

RenderPool::RenderPool(Scene* scene, Integrator* integrator, int numThreads, std::vector<RenderJob*>& jobs)
    : _scene(scene), _integrator(integrator), _nextJob(0), _jobQueue(jobs)
{
    for (int i = 0; i < numThreads; i++) {
        _threads.push_back(std::thread(threadMain, this));
    }
}

RenderPool::~RenderPool()
{
    for (std::thread& thread : _threads) {
        thread.join();
    }
}

void RenderPool::getCompletedJobs(std::vector<RenderJob*>& completedJobs)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);

        _condition.wait(lock, [this]{ return _completedJobs.size() > 0; });
        completedJobs = std::move(_completedJobs);
    }
}

void RenderPool::threadMain(RenderPool* pool)
{
    while (true) {

        size_t jobIndex;
        {
            std::unique_lock<std::mutex> lock(pool->_mutex);

            if (pool->_nextJob >= pool->_jobQueue.size()) break;

            jobIndex = pool->_nextJob;
            pool->_nextJob++;
        }

        pool->_jobQueue[jobIndex]->render(pool->_scene, pool->_integrator);

        {
            std::unique_lock<std::mutex> lock(pool->_mutex);

            pool->_completedJobs.push_back(pool->_jobQueue[jobIndex]);
            pool->_condition.notify_all();
        }
    }
}
