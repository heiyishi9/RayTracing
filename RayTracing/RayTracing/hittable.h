#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"

class material;

struct hit_record 
{
    vec3 p;                         // ���߹�ʽ��p(t) = a + tb
    vec3 normal;                    // ����
    shared_ptr<material> mat_ptr;   // ���ʵ�ָ��
    double t;                       // �����������ཻ���Ǹ�tֵ
    bool front_face;                // �����жϷ��෽��

    // ��Զ�÷��������䷽���෴
    inline void set_face_normal(const ray& r, const vec3& outward_normal) 
    {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// �κο���������󽻵Ķ���ʵ��ʱ���̳������
class hittable 
{
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

#endif
