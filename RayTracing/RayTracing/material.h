#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

class material 
{
public:
    // attenuation：衰减，如果发生散射, 决定光线会变暗多少
    // scattered：散射，生成散射后的光线(或者说它吸收了入射光线)
    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

// 漫反射材质，lambertian
class lambertian : public material 
{
public:
    lambertian(const vec3& a) : albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();
        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

public:
    vec3 albedo;
};

// 金属材质
class metal : public material 
{
public:
    metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);    // dot<0我们认为吸收
    }

public:
    vec3 albedo;
    double fuzz;    // 模糊(粗糙)，当 fuzz=0 时不会产生模糊
};

// 绝缘体材质（透明的材料：水、玻璃、钻石）
// 当光线击中这类材料时, 一条光线会分成两条, 一条发生反射, 一条发生折射。
class dielectric : public material 
{
public:
    dielectric(double ri) : ref_idx(ri) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        attenuation = vec3(1.0, 1.0, 1.0);
        double etai_over_etat = (rec.front_face) ? (1.0 / ref_idx) : (ref_idx);

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = ffmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        // 光线从高折射律介质射入低折射率介质时，可能没有实解，
        // 这时候就不会发生折射，所以就会出现许多小黑点
        if (etai_over_etat * sin_theta > 1.0) 
        {
            // 必须发生反射，没有折射
            vec3 reflected = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, reflected);
            return true;
        }
        double reflect_prob = schlick(cos_theta, etai_over_etat);
        if (random_double() < reflect_prob)
        {
            vec3 reflected = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, reflected);
            return true;
        }

        vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
        scattered = ray(rec.p, refracted);
        return true;
    }

    double ref_idx;     // 介质折射率(规定空气为1.0，玻璃为1.3-1.7，钻石为2.4)
};

#endif 

