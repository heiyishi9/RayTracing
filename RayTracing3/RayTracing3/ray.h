#ifndef RAY_H
#define RAY_H

#include "vec3.h"

// 射线
class ray {
public:
    ray() {}
    ray(const vec3& origin, const vec3& direction, double time = 0.0)
        : orig(origin), dir(direction), tm(time)
    {}

    vec3 origin() const { return orig; }
    vec3 direction() const { return dir; }
    double time() const { return tm; }

    // p(t) = a + tb
    vec3 at(double t) const {
        return orig + t * dir;
    }

public:
    vec3 orig;  // 射线原点
    vec3 dir;   // 射线方向
    double tm;  // 自己所在时刻
};
#endif
