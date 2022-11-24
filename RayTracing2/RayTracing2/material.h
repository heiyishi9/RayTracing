#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"

class material 
{
public:
    // ���������ɫ��Ĭ�Ϸ��غ�ɫ
    virtual vec3 emitted(double u, double v, const vec3& p) const 
    {
        return vec3(0, 0, 0);
    }

    // attenuation��˥�����������ɢ��, �������߻�䰵����
    // scattered��ɢ�䣬����ɢ���Ĺ���(����˵���������������)
    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

// ��������ʣ�lambertian
class lambertian : public material 
{
public:
    lambertian(const vec3& a) : albedo(make_shared<constant_texture>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;
};

// ��������
class metal : public material 
{
public:
    metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);    // dot<0������Ϊ����
    }

public:
    vec3 albedo;
    double fuzz;    // ģ��(�ֲ�)���� fuzz=0 ʱ�������ģ��
};

// ��Ե����ʣ�͸���Ĳ��ϣ�ˮ����������ʯ��
// �����߻����������ʱ, һ�����߻�ֳ�����, һ����������, һ���������䡣
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
        // ���ߴӸ������ɽ�������������ʽ���ʱ������û��ʵ�⣬
        // ��ʱ��Ͳ��ᷢ�����䣬���Ծͻ�������С�ڵ�
        if (etai_over_etat * sin_theta > 1.0) 
        {
            // ���뷢�����䣬û������
            vec3 reflected = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, reflected, r_in.time());
            return true;
        }
        double reflect_prob = schlick(cos_theta, etai_over_etat);
        if (random_double() < reflect_prob)
        {
            vec3 reflected = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, reflected, r_in.time());
            return true;
        }

        vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
        scattered = ray(rec.p, refracted, r_in.time());
        return true;
    }

    double ref_idx;     // ����������(�涨����Ϊ1.0������Ϊ1.3-1.7����ʯΪ2.4)
};

// ������ߵĲ���
class diffuse_light : public material
{
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(vec3 c) : emit(make_shared<constant_texture>(c)) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        return false;
    }

    virtual vec3 emitted(double u, double v, const vec3& p) const 
    {
        return emit->value(u, v, p);
    }

public:
    shared_ptr<texture> emit;
};

// ����ͬ�Ե������λ����
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

