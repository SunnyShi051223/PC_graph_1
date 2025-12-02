#pragma once
#ifndef __SPHERE_H__
#define __SPHERE_H__
#include "vector3d.h"
#include "color.h"
#include "ray.h"
#include "object.h"
class Sphere : public Object
{
public:
	Vector3d center_;
	float radius_;
	Sphere() {};
	virtual bool intersect(Ray ray, Hit& hit);
};
#endif