#include "scene.h"
#include "light.h"

void Scene::parseLights( nlohmann::json sceneConfig){
    /*Function to load lights*/
    // nlohmann::json sceneConfig = this->sceneConfig;/
    try {
        auto point_lights = sceneConfig["pointLights"];

        for (auto & curr : point_lights){
            Vector3f location = Vector3f(curr["location"][0], curr["location"][1], curr["location"][2]);
            Vector3f radiance = Vector3f(curr["radiance"][0], curr["radiance"][1], curr["radiance"][2]);
            Vector3f direction = Vector3f(0.0f);
            Light light(POINT_LIGHT, radiance, location, direction);
            this->lights.push_back(light);
        }
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "point light error." << std::endl;
        exit(1);
    }
    try {
        auto direction_lights = sceneConfig["directionalLights"];

        for (auto & curr : direction_lights){
            Vector3f direction = Vector3f(curr["direction"][0], curr["direction"][1], curr["direction"][2]);
            Vector3f radiance = Vector3f(curr["radiance"][0], curr["radiance"][1], curr["radiance"][2]);
            Vector3f location = Vector3f(0.0f);
            Light light(DIRECTIONAL_LIGHT, radiance, location, direction);
            this->lights.push_back(light);
        }
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "direction light error." << std::endl;
        exit(1);
    }

}