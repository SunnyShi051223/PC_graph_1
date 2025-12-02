#pragma once
#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "color.h"
#include "ray.h"
class Hit;
class Material
{
public:
	Color color_;
	Color ambientColor_;
	Color specularColor_;
	Color diffuseColor_;
	float alpha_;
	float refractIndex_;
	float specExponent_;
	float reflectionCoeff_;
	Material() {};
};
class Object
{
public:
	Material material_;
	Object() {};
	virtual bool intersect(Ray ray, Hit& hit) = 0;
};
class Hit
{
public:
	Vector3d P_;
	Vector3d N_;
	Object* obj_;
	float t_;
	Material material_;
	Hit() {};
};
#endif