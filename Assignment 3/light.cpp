#include "light.h"

Light::Light(LightType type, nlohmann::json config)
{
    switch (type)
    {
    case LightType::POINT_LIGHT:
        this->position = Vector3f(config["location"][0], config["location"][1], config["location"][2]);
        break;
    case LightType::DIRECTIONAL_LIGHT:
        this->direction = Vector3f(config["direction"][0], config["direction"][1], config["direction"][2]);
        break;
    case LightType::AREA_LIGHT:
        // TODO: Implement this
        this->center = Vector3f(config["center"][0], config["center"][1], config["center"][2]);
        this->vx = Vector3f(config["vx"][0], config["vx"][1], config["vx"][2]);
        this->vy = Vector3f(config["vy"][0], config["vy"][1], config["vy"][2]);
        this->normal = Vector3f(config["normal"][0], config["normal"][1], config["normal"][2]);
        break;
    default:
        std::cout << "WARNING: Invalid light type detected";
        break;
    }

    this->radiance = Vector3f(config["radiance"][0], config["radiance"][1], config["radiance"][2]);
    this->type = type;
}

std::pair<Vector3f, LightSample> Light::sample(Interaction *si)
{
    LightSample ls;
    memset(&ls, 0, sizeof(ls));

    Vector3f radiance;
    switch (type)
    {
    case LightType::POINT_LIGHT:
        ls.wo = (position - si->p);
        ls.d = ls.wo.Length();
        ls.wo = Normalize(ls.wo);
        radiance = (1.f / (ls.d * ls.d)) * this->radiance;
        break;
    case LightType::DIRECTIONAL_LIGHT:
        ls.wo = Normalize(direction);
        ls.d = 1e10;
        radiance = this->radiance;
        break;
    case LightType::AREA_LIGHT:
        // TODO: Implement this
        // ls.wo = Normalize(this->normal);
        // radiance = this->radiance;
        auto r1 = next_float(), r2 = next_float();
        ls.p = center + 2.f*(r1 - 0.5f)*vx + 2.f*(r2 - 0.5f)*vy;
        
        ls.wo = ls.p - si->p;
        ls.d = ls.wo.Length();
        ls.wo = Normalize(ls.wo);
        if(std::abs(si->n[0] - normal[0]) < 1e-6 &&std::abs(si->n[1] - normal[1]) < 1e-6 &&std::abs(si->n[2] - normal[2]) < 1e-6) 
            radiance = Vector3f(0);
        else 
            radiance = (1.f / (ls.d * ls.d)) * this->radiance;
        break;
    }
    return {radiance, ls};
}


Interaction Light::rayPlaneIntersect(Ray ray, Vector3f p, Vector3f n)
{
    Interaction si;

    float dDotN = Dot(ray.d, n);
    if (dDotN != 0.f) {
        float t = -Dot((ray.o - p), n) / dDotN;

        if (t >= 0.f) {
            si.didIntersect = true;
            si.t = t;
            si.n = n;
            si.p = ray.o + ray.d * si.t;
        }
    }

    return si;
}

Interaction Light::rayTriangleIntersect(Ray ray, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f n)
{
    Interaction si = this->rayPlaneIntersect(ray, v1, n);

    if (si.didIntersect) {
        bool edge1 = false, edge2 = false, edge3 = false;

        // Check edge 1
        {
            Vector3f nIp = Cross((si.p - v1), (v3 - v1));
            Vector3f nTri = Cross((v2 - v1), (v3 - v1));
            edge1 = Dot(nIp, nTri) > 0;
        }

        // Check edge 2
        {
            Vector3f nIp = Cross((si.p - v1), (v2 - v1));
            Vector3f nTri = Cross((v3 - v1), (v2 - v1));
            edge2 = Dot(nIp, nTri) > 0;
        }

        // Check edge 3
        {
            Vector3f nIp = Cross((si.p - v2), (v3 - v2));
            Vector3f nTri = Cross((v1 - v2), (v3 - v2));
            edge3 = Dot(nIp, nTri) > 0;
        }

        if (edge1 && edge2 && edge3 && si.t >= 0.f) {
            // Intersected triangle!
            si.didIntersect = true;
        }
        else {
            si.didIntersect = false;
        }
    }

    return si;
}


Interaction Light::intersectLight(Ray *ray)
{
    Interaction si;
    memset(&si, 0, sizeof(si));

    if (type == LightType::AREA_LIGHT)
    {
        // TODO: Implement this
        // point on the plane
        auto p = center;
        auto n = normal;
        si = rayPlaneIntersect(*ray, p, n);
        if (si.didIntersect && (Dot(ray->o - p, n) > 0))
        {
            Vector3f p1 = center + vx + vy;
            Vector3f p2 = center - vx + vy;
            Vector3f p3 = center - vx - vy;
            Vector3f p4 = center + vx - vy;
            si = rayTriangleIntersect(*ray, p1, p2, p3, n);
            if(si.didIntersect){
                si.emissiveColor = radiance;
                return si;
            }
            si = rayTriangleIntersect(*ray, p1, p3, p4, n);
            if(si.didIntersect){
                // std::cout<<si.t<<"\n";

                si.emissiveColor = radiance;
                return si;
            }
        }
    }

    si.didIntersect = false;
    return si;
}