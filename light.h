#pragma once
#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "vector3d.h"
#include "color.h"
class Light
{
public:
	Color color_;
	Vector3d P_;
	Light() {};
	Light(const Color& color, const Vector3d& v) : color_{ color }, P_{ v } {};
};
#endif