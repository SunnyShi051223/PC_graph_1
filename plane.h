#pragma once
// plane.h
#pragma once
#ifndef __PLANE_H__
#define __PLANE_H__
#include "object.h"

class Plane : public Object
{
public:
    Vector3d point_;  // 平面上的一点
    Vector3d normal_; // 平面的法线

    Plane(Vector3d p, Vector3d n) : point_(p), normal_(n) {
        normal_.normalize();
    };

    virtual bool intersect(Ray ray, Hit& hit) {
        float denom = normal_.dot(ray.directionVector_);
        // 如果分母接近0，说明光线与平面平行
        if (std::abs(denom) > 1e-6) {
            Vector3d p0l0 = point_ - ray.origin_;
            float t = p0l0.dot(normal_) / denom;
            if (t >= 0.001f) { // 0.001f 用于防止自相交
                hit.t_ = t;
                hit.material_ = this->material_;
                hit.P_ = ray.origin_ + ray.directionVector_ * t;
                hit.N_ = normal_;
                // 确保法线指向光线来源的一侧
                if (hit.N_.dot(ray.directionVector_) > 0) {
                    hit.N_ = hit.N_ * -1.0f;
                }
                return true;
            }
        }
        return false;
    }
};
#endif