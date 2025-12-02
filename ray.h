#pragma once
#ifndef __RAY_H__
#define __RAY_H__
#include "vector3d.h"
class Ray
{
public:
	Vector3d origin_, directionVector_;
	Ray() {};
	Ray(const Vector3d& p, const Vector3d& d) : origin_{ p }, directionVector_{ d } {};
};
#endif