#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "pdf.h"

struct scatter_record
{
    // attenuation：衰减，如果发生散射, 决定光线会变暗多少
    // scattered：散射，生成散射后的光线(或者说它吸收了入射光线)
    ray specular_ray;
    bool is_specular;
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
};

class material 
{
public:
    // 发射出的颜色，默认返回黑色
    virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const
    {
        return color(0, 0, 0);
    }

    
    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const 
    {
        return false;
    }

    // 散射概率密度函数,重要性采样
    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        return 0;
    }
};

// 漫反射材质，lambertian
class lambertian : public material 
{
public:
    lambertian(const vec3& a) : albedo(make_shared<constant_texture>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
    {
        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);

        return true;
    }

    // 散射概率密度函数,重要性采样
    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
    }

public:
    shared_ptr<texture> albedo;
};

// 金属材质
class metal : public material 
{
public:
    metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override 
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        srec.specular_ray = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        srec.attenuation = albedo;
        srec.is_specular = true;
        srec.pdf_ptr = 0;

        return true;
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

    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
    {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = color(1.0, 1.0, 1.0);
        double refraction_ratio = (rec.front_face) ? (1.0 / ref_idx) : (ref_idx);

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = ffmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        
        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        srec.specular_ray = ray(rec.p, direction, r_in.time());
        return true;
    }

    double ref_idx;     // 介质折射率(规定空气为1.0，玻璃为1.3-1.7，钻石为2.4)

private:
    static double reflectance(double cosine, double ref_idx)
    {
        // 使用Schlick的反射率近似值
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;

        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

// 发射光线的材质
class diffuse_light : public material
{
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(vec3 c) : emit(make_shared<constant_texture>(c)) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        return false;
    }

    virtual vec3 emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const override
    {
        if (rec.front_face)
            return emit->value(u, v, p);
        else
            return color(0, 0, 0);
    }

public:
    shared_ptr<texture> emit;
};

// 各向同性的随机单位向量
class isotropic : public material
{
public:
    isotropic(vec3 c) : albedo(make_shared<constant_texture>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
    {
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }
    
public:
    shared_ptr<texture> albedo;
};

#endif 

