#pragma once

#include "common.h"
#include "scene.h"
#include "camera.h"

enum LightType {
	POINT_LIGHT=0,
	DIRECTIONAL_LIGHT=1,
	NUM_LIGHT_TYPES
};

struct Light {
	LightType type;
	Vector3f radiance;
	Vector3f location;
	Vector3f direction;
	Light(LightType type, const Vector3f& radiance, const Vector3f& location, const Vector3f& direction)
        : type(type), radiance(radiance), location(location), direction(direction) {}
};