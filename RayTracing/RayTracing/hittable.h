#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"

class material;

struct hit_record 
{
    vec3 p;                         // 光线公式：p(t) = a + tb
    vec3 normal;                    // 法相
    shared_ptr<material> mat_ptr;   // 材质的指针
    double t;                       // 光线与物体相交的那个t值
    bool front_face;                // 用来判断法相方向

    // 永远让法相与入射方向相反
    inline void set_face_normal(const ray& r, const vec3& outward_normal) 
    {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// 任何可能与光线求交的东西实现时都继承这个类
class hittable 
{
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

#endif
