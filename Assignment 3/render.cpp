#include "render.h"
#include <random>

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}
int variant = 0;
long long Integrator::render()
{
    int arealights = 0;
    std::vector<Light> aLights;
    for(Light &light: this->scene.lights){
        if (light.type == AREA_LIGHT)
        {
            arealights++;
            aLights.push_back(light);
        }
        
    }
    std::cout << this->spp << "\n";
    auto startTime = std::chrono::high_resolution_clock::now();

    if(variant == 3){

        for (int x = 0; x < this->scene.imageResolution.x; x++)
        {
            for (int y = 0; y < this->scene.imageResolution.y; y++)
            {
                Vector3f result(0, 0, 0);
                for(int i=0; i<this->spp; i++){
                    Ray cameraRay = this->scene.camera.generateRay(x, y);
                    Interaction si = this->scene.rayIntersect(cameraRay);
                    Interaction si2 = this->scene.rayEmitterIntersect(cameraRay);
                    if(si.didIntersect){
                    // https://en.cppreference.com/w/cpp/numeric/random/random_device
                        std::random_device rd;  // a seed source for the random number engine
                        std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
                        std::uniform_int_distribution<> distrib(0, this->scene.lights.size() - 1);
                        int idx = distrib(gen);
                        auto light = this->scene.lights[idx];
                        Vector3f radiance; LightSample ls;
                        if(light.type == DIRECTIONAL_LIGHT || light.type == POINT_LIGHT){
                            std::tie(radiance, ls) = light.sample(&si);

                            Ray shadowRay(si.p + 1e-3f * si.n, ls.wo);
                            Interaction siShadow = this->scene.rayIntersect(shadowRay);

                            if (!siShadow.didIntersect || siShadow.t > ls.d)
                            {
                                result += si.bsdf->eval(&si, si.toLocal(ls.wo)) * radiance * std::abs(Dot(si.n, ls.wo));
                            }
                        }
                        else{
                            std::tie(radiance, ls) = light.sample(&si);
                            Ray shadowRay(si.p + 1e-3f * si.n, ls.wo);
                            Interaction siShadow = this->scene.rayIntersect(shadowRay);
                            auto center = light.center, vx = light.vx, vy = light.vy;
                            Vector3f p1 = center + vx + vy;
                            Vector3f p2 = center - vx + vy;
                            Vector3f p3 = center - vx - vy;
                            Vector3f p4 = center + vx - vy;
                            Vector3f cp = Cross(p2 - p1, p4 - p1);
                            auto area = cp.Length();
                            auto cost = std::abs(Dot(light.normal , ls.wo));
                            if (!siShadow.didIntersect || siShadow.t > ls.d)
                            {
                                result += si.bsdf->eval(&si, ls.wo) * radiance * std::abs(Dot(si.n, ls.wo))  * area * cost;
                            }
                        }
                        result += si2.emissiveColor;
                    }
                result /= this->spp;
                result /= this->scene.lights.size();
                this->outputImage.writePixelColor(result, x, y);
                }
            }
            
        }
    }
    else{

        for (int x = 0; x < this->scene.imageResolution.x; x++)
        {
            for (int y = 0; y < this->scene.imageResolution.y; y++)
            {
                Vector3f result(0, 0, 0);
                for (int i = 0; i < this->spp; i++)
                {
                    Ray cameraRay = this->scene.camera.generateRay(x, y);
                    Interaction si = this->scene.rayIntersect(cameraRay);
                    Interaction si2 = this->scene.rayEmitterIntersect(cameraRay);
                    Vector3f subresult(0);
                    if (si.didIntersect)
                    {
                        Vector3f radiance;
                        LightSample ls;
                        for (Light &light : this->scene.lights)
                        {
                            if (light.type == AREA_LIGHT)
                                continue;
                            std::tie(radiance, ls) = light.sample(&si);

                            Ray shadowRay(si.p + 1e-3f * si.n, ls.wo);
                            Interaction siShadow = this->scene.rayIntersect(shadowRay);

                            if (!siShadow.didIntersect || siShadow.t > ls.d)
                            {
                                subresult += si.bsdf->eval(&si, si.toLocal(ls.wo)) * radiance * std::abs(Dot(si.n, ls.wo));
                            }
                        }
                    }
                    if (si.didIntersect && (variant == 0 || variant == 1))
                    {
                        for (Light &light : this->scene.lights)
                        {
                            if (light.type != AREA_LIGHT)
                                continue;

                            // sample directions

                            Vector3f wo;
                            if ((variant == 0))
                            {
                                wo = si.hemisphere();
                            }

                            if (variant == 1)
                            {
                                wo = si.cosine_sample();
                            }
                            Ray shadowRay(si.p + 1e-5f * si.n, Normalize(si.toWorld(wo)));
                            Interaction siShadow = this->scene.rayEmitterIntersect(shadowRay);
                            Interaction siShadow2 = this->scene.rayIntersect(shadowRay);

                            if (siShadow2.didIntersect && siShadow2.t < siShadow.t)
                            {
                                continue;
                            }
                            if (siShadow.didIntersect && siShadow.t < siShadow2.t)
                            {
                                if (variant == 0)
                                {
                                    subresult += si.bsdf->eval(&si, wo) * siShadow.emissiveColor * std::abs(Dot(si.n, wo)) * 2 * M_PI;
                                }
                                if (variant == 1)
                                {
                                    subresult += si.bsdf->eval(&si, wo) * siShadow.emissiveColor*M_PI;
                                }
                            }
                        }
                        subresult/=arealights;
                    }
                    if (si.didIntersect && (variant == 2))
                    {
                        Vector3f radiance;
                        LightSample ls;
                        for (Light &light : this->scene.lights)
                        {
                            if (light.type != AREA_LIGHT)
                                continue;

                            std::tie(radiance, ls) = light.sample(&si);
                            Ray shadowRay(si.p + 1e-3f * si.n, ls.wo);
                            Interaction siShadow = this->scene.rayIntersect(shadowRay);
                            auto center = light.center, vx = light.vx, vy = light.vy;
                            Vector3f p1 = center + vx + vy;
                            Vector3f p2 = center - vx + vy;
                            Vector3f p3 = center - vx - vy;
                            Vector3f p4 = center + vx - vy;
                            Vector3f cp = Cross(p2 - p1, p4 - p1);
                            auto area = cp.Length();
                            auto cost = std::abs(Dot(light.normal , ls.wo));
                            if (!siShadow.didIntersect || siShadow.t > ls.d)
                            {
                                subresult += si.bsdf->eval(&si, ls.wo) * radiance * std::abs(Dot(si.n, ls.wo))  * area * cost;
                            }
                        }
                    }
                    
                    result += (subresult + si2.emissiveColor);
                }

                result /= this->spp;
                this->outputImage.writePixelColor(result, x, y);
            }
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        std::cerr << "Usage: ./render <scene_config> <out_path> <num_samples> <sampling_strategy>\n";
        return 1;
    }
    Scene scene(argv[1]);
    for (auto light : scene.lights)
    {
        if (light.type == AREA_LIGHT)
        {
            light.normal.Print();
        }
    }
    Integrator rayTracer(scene);
    int spp = atoi(argv[3]);
    variant = atoi(argv[4]);
    rayTracer.spp = spp;
    auto renderTime = rayTracer.render();

    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
