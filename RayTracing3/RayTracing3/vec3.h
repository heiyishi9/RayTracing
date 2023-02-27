#ifndef VEC3_H
#define VEC3_H

#include <iostream>
#include "rtweekend.h"

class vec3 
{
public:
    vec3() : e{ 0,0,0 } {}
    vec3(double e0, double e1, double e2) : e{ e0, e1, e2 } {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    vec3& operator+=(const vec3& v) 
    {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3& operator*=(const double t) 
    {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator/=(const double t) 
    {
        return *this *= 1 / t;
    }

    double length() const 
    {
        return sqrt(length_squared());
    }

    double length_squared() const 
    {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    bool near_zero() const
    {
        // �������������ά���϶��ӽ����㣬�򷵻�true
        const auto s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }

    inline static vec3 random()
    {
        return vec3(random_double(), random_double(), random_double());
    }

    inline static vec3 random(double min, double max)
    {
        return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }

public:
    double e[3];
};

inline std::ostream& operator<<(std::ostream& out, const vec3& v) 
{
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) 
{
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) 
{
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) 
{
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v) 
{
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, double t) 
{
    return t * v;
}

inline vec3 operator/(vec3 v, double t) 
{
    return (1 / t) * v;
}

inline double dot(const vec3& u, const vec3& v) 
{
    return u.e[0] * v.e[0]
        + u.e[1] * v.e[1]
        + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) 
{
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
        u.e[2] * v.e[0] - u.e[0] * v.e[2],
        u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

// ��λ����
inline vec3 unit_vector(vec3 v) 
{
    return v / v.length();
}

// �񶨷������ɵ�λ�����ڵ������
vec3 random_in_unit_sphere() 
{
    while (true) 
    {
        auto p = vec3::random(-1, 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

// ֱ�Ӵ�����㿪ʼѡȡһ������ķ���, Ȼ�����ж��Ƿ��ڷ��������ڵ��Ǹ�����
vec3 random_in_hemisphere(const vec3& normal) 
{
    vec3 in_unit_sphere = random_in_unit_sphere();
    // �ͷ�����ͬһ����
    if (dot(in_unit_sphere, normal) > 0.0) 
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}

// �÷���������ķ���������ʹ��
// ������lambertianɢ���Ĺ��߾��뷨��ȽϽ��ĸ��ʻ����, ���Ƿֲ��ɻ���Ӿ��⡣
vec3 random_unit_vector() 
{
    auto a = random_double(0, 2 * pi);
    auto z = random_double(-1, 1);
    auto r = sqrt(1 - z * z);
    return vec3(r * cos(a), r * sin(a), z);
}

// ����
vec3 reflect(const vec3& v, const vec3& n) 
{
    return v - 2 * dot(v, n) * n;
}

// ����
// uv������ⷽ��n���������һ��ķ�����
// etai_over_etat��������һ����ʵ�������/������һ����ʵ�������
vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) 
{
    auto cos_theta = dot(-uv, n);
    // ������ߵ�ˮƽ����
    vec3 r_out_parallel = etai_over_etat * (uv + cos_theta * n);
    // ������ߵĴ�ֱ����
    vec3 r_out_perp = -sqrt(1.0 - r_out_parallel.length_squared()) * n;
    return r_out_parallel + r_out_perp;
}

// ��ʵ�����еĲ���, ��������ĸ��ʻ���������Ƕ��ı�
// Schlick����Ľ��Ƶĵ�ʽ
double schlick(double cosine, double ref_idx) {
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

// ��һ����λСԲ���������
// ��ɢ��ģ�������
vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

// Type aliases for vec3
using point3 = vec3;        // 3D point
using color = vec3;         // RGB color

void write_color(std::ostream& out, color pixel_color, int samples_per_pixel)
{
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    if (r != r) r = 0.0;
    if (g != g) g = 0.0;
    if (b != b) b = 0.0;

    // ����ɫ�������Բ�������
    // gamma 2У������ζ�����յ���ɫֵҪ����ָ��1/2������ƽ����
    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    // д��ÿ����ɫ���������[0,255]ֵ��
    char color[3];
    color[0] = 256 * clamp(r, 0.0, 0.999);
    color[1] = 256 * clamp(g, 0.0, 0.999);
    color[2] = 256 * clamp(b, 0.0, 0.999);

    out.write(color, 3);
}
#endif 