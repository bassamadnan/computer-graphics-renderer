#include "render.h"

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}
int variant = 3;

long long Integrator::render()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay);

            if (si.didIntersect){
                // int
                // store uv in si;

                auto f  = si.f, color = Vector3f(0.0f);
                if(variant == 0) f = si.neighbor;
                if(variant == 1) f = si.bilinear;
                if(variant == 2) f = Vector3f(M_1_PI);
                // std::cout<<si.uv.x<<" "<<si.uv.y<<"\n";
                // return 0;
                // f = this->outputImage.nearestNeighbourFetch(Vector2f((si.uv.x), (1 - si.uv.y)));
                // f *= M_1_PI;
                // std::cout<<si.uv.x <<" "<<si.uv.y <<" \n";
                auto z = si.p, n = si.n;
                for(auto& light: this->scene.lights){
                    if(light.type == POINT_LIGHT) continue;
                    auto  d = light.direction, radiance = light.radiance;
                    Ray shadowRay = Ray(z + ((1e-5)*n), Normalize(d));
                    Interaction si2 = this->scene.rayIntersect(shadowRay);
                    if(!(si2.didIntersect))
                        color += (f *radiance) * ((Dot((n), (d))));
                    
                }
                for(auto& light: this->scene.lights){
                    if(light.type == DIRECTIONAL_LIGHT) continue;
                    auto  p = light.location, radiance = light.radiance;
                    Ray shadowRay = Ray( z + ((1e-5)*n), Normalize(p - z));
                    Interaction si2 = this->scene.rayIntersect(shadowRay);
                    bool hit = si2.didIntersect;
                    auto prod = Dot(p-z, p-z);
                    if(prod < 0.000001f) prod = 1e-3;
                    radiance = radiance/prod;
                    if(!(hit) || si2.t >= sqrt(prod))
                        color += ((radiance* (f))*((Dot(n, Normalize(p - z))))) ; // missing term?
                    
                    
                }
                this->outputImage.writePixelColor(color, x, y);
            }
            // if (si.didIntersect)
            //     this->outputImage.writePixelColor(0.5f * (si.n + Vector3f(1.f, 1.f, 1.f)), x, y);
            else
                this->outputImage.writePixelColor(Vector3f(0.f), x, y);
        }
    }
                    // if shadowray does not hit an object -> illuminate it / color it
                    // if shadowray does hit an object -> shadow it / color = 0.0
                    // hint: if hit , point lights can still be visible (color != 0) ?
                    // move in units of p-z, light is at distance of mod p - z
                    // i.e if from point p, a ray towards z is 
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Usage: ./render <scene_config> <out_path> <variant>";
        return 1;
    }
    Scene scene(argv[1]);
    // std::cout<<"point lights\n";
    // for(auto light: scene.lights){
    //     if(light.type == POINT_LIGHT){
    //         light.radiance.Print();
    //         light.location.Print();
    //     }
    // }
    //  std::cout<<"direction lights\n";
    // for(auto light: scene.lights){
    //     if(light.type == DIRECTIONAL_LIGHT){
    //         light.radiance.Print();
    //         light.direction.Print();
    //     }
    // }

    Integrator rayTracer(scene);
    variant = atoi(argv[3]);
    std::cout<<variant<<"\n";
    for(auto &surf : scene.surfaces){
        // std::cout<<surf.diffuseTexture.resolution[0] << " "<<surf.diffuseTexture.resolution[1] <<"\n";
        std::cout<<surf.uvs[0].x <<" "<<surf.uvs[0].y<<"\n";
    }
    auto renderTime = rayTracer.render();
    
    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
