#ifndef MOVING_SPHERE_H
#define MOVING_SPHERE_H
#include "rtweekend.h"
#include "hittable.h"

// 运动的球体
class moving_sphere : public hittable 
{
public:
    moving_sphere() {}
    moving_sphere(
        vec3 cen0, vec3 cen1, double t0, double t1, double r, shared_ptr<material> m)
        : center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mat_ptr(m)
    {};

    virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const;
    virtual bool bounding_box(double t0, double t1, aabb& output_box) const;

    vec3 center(double time) const;

public:
    vec3 center0, center1;
    double time0, time1;
    double radius;
    shared_ptr<material> mat_ptr;
};

vec3 moving_sphere::center(double time) const 
{
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

bool moving_sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const 
{
    // 二次求根方程：ax^2 + bx + c = 0
    vec3 oc = r.origin() - center(r.time());
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());               // b取一半，方便简化（把2约掉）
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    // 光线与球相交
    if (discriminant > 0) 
    {
        auto root = sqrt(discriminant);

        auto temp = (-half_b - root) / a;
        if (temp < t_max && temp > t_min) 
        {
            rec.t = temp;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center(r.time())) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat_ptr = mat_ptr;
            return true;
        }

        temp = (-half_b + root) / a;
        if (temp < t_max && temp > t_min) 
        {
            rec.t = temp;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center(r.time())) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }
    return false;
}

// 先求球体在t0时刻的包围盒，再求球体在t1时刻的包围盒，然后再计算两个盒子的包围盒
bool moving_sphere::bounding_box(double t0, double t1, aabb& output_box) const 
{
    aabb box0(
        center(t0) - vec3(radius, radius, radius),
        center(t0) + vec3(radius, radius, radius));
    aabb box1(
        center(t1) - vec3(radius, radius, radius),
        center(t1) + vec3(radius, radius, radius));
    output_box = surrounding_box(box0, box1);
    return true;
}

#endif
