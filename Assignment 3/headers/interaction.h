#pragma once

#include "vec.h"

// Forward declaration of BSDF class
class BSDF;

struct Interaction
{
    // Position of interaction
    Vector3f p;
    // Normal of the surface at interaction
    Vector3f n;
    // The uv co-ordinates at the intersection point
    Vector2f uv;
    // The viewing direction in local shading frame
    Vector3f wi;
    // Distance of intersection point from origin of the ray
    float t = 1e30f;
    // Used for light intersection, holds the radiance emitted by the emitter.
    Vector3f emissiveColor = Vector3f(0.f, 0.f, 0.f);
    // BSDF at the shading point
    BSDF *bsdf;
    // Vectors defining the orthonormal basis
    Vector3f a, b, c;
    // Probability of sampled direction
    float prob;

    bool didIntersect = false;

    Vector3f toWorld(Vector3f w)
    {
        // TODO: Implement this
        // this->a.Print();
        auto x = Vector3f(a.x, b.x, c.x), y = Vector3f(a.y, b.y, c.y), z = Vector3f(a.z, b.z, c.z); // transpose
        auto gx = Dot(w, x), gy = Dot(w, y), gz = Dot(w, z);                                        // global cordinates
        return Vector3f(gx, gy, gz);
    }

    Vector3f toLocal(Vector3f w)
    {
        // TODO: Implement this
        auto lx = Dot(w, a), ly = Dot(w, b), lz = Dot(w, c); // local cordinates
        return Vector3f(lx, ly, lz);
    }

    Vector3f hemisphere()
    {
        auto z1 = next_float(), z2 = next_float(); // zeta values
        float z = z1;
        float r = std::sqrt(std::max((float)0, (float)1. - z * z));
        float phi = 2 * M_PI * z2;
        
        auto sample = (Vector3f(r * cos(phi), r * sin(phi), z));
        return sample;
    }

   Vector3f cosine_sample(){
        auto z1 = next_float(), z2 = next_float(); // zeta values
        float cosTheta = sqrt(1.0f - z1); 
        float sinTheta = sqrt(z1);
        float phi = 2 * M_PI * z2;
        
        auto sample = Vector3f(cosTheta * cos(phi), cosTheta * sin(phi), sinTheta);
        return sample;
    }


    void setONB()
    {
        Vector3f v = Vector3f(0);
        int minidx = 0;
        for(int i=1; i<3; i++) if(std::abs(n[i]) < std::abs(n[minidx])) minidx = i;
        v[(minidx + 2)%3] = -n[(minidx + 1)%3];
        v[(minidx + 1)%3] = n[(minidx + 2)%3];
        auto v1 = this->n;
        auto v2 = Normalize(v);
        auto v3 = Normalize(Cross(v1, v2));
        this->a = v3, this->b = v2, this->c = v1;
    }
};