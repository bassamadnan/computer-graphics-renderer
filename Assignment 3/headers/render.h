#pragma once

#include "scene.h"

struct Integrator {
    Integrator(Scene& scene);

    long long render();
    long long spp;
    Scene scene;
    Texture outputImage;
};