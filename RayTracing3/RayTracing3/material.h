#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "pdf.h"

struct scatter_record
{
    // attenuation��˥�����������ɢ��, �������߻�䰵����
    // scattered��ɢ�䣬����ɢ���Ĺ���(����˵���������������)
    ray specular_ray;
    bool is_specular;
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
};

class material 
{
public:
    // ���������ɫ��Ĭ�Ϸ��غ�ɫ
    virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const
    {
        return color(0, 0, 0);
    }

    
    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const 
    {
        return false;
    }

    // ɢ������ܶȺ���,��Ҫ�Բ���
    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        return 0;
    }
};

// ��������ʣ�lambertian
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

    // ɢ������ܶȺ���,��Ҫ�Բ���
    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
    }

public:
    shared_ptr<texture> albedo;
};

// ��������
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
    double fuzz;    // ģ��(�ֲ�)���� fuzz=0 ʱ�������ģ��
};

// ��Ե����ʣ�͸���Ĳ��ϣ�ˮ����������ʯ��
// �����߻����������ʱ, һ�����߻�ֳ�����, һ����������, һ���������䡣
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

    double ref_idx;     // ����������(�涨����Ϊ1.0������Ϊ1.3-1.7����ʯΪ2.4)

private:
    static double reflectance(double cosine, double ref_idx)
    {
        // ʹ��Schlick�ķ����ʽ���ֵ
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;

        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
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
