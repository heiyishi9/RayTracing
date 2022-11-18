#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

class material 
{
public:
    // attenuation��˥�����������ɢ��, �������߻�䰵����
    // scattered��ɢ�䣬����ɢ���Ĺ���(����˵���������������)
    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

// ��������ʣ�lambertian
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

// ��������
class metal : public material 
{
public:
    metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
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

    double ref_idx;     // ����������(�涨����Ϊ1.0������Ϊ1.3-1.7����ʯΪ2.4)
};

#endif 

