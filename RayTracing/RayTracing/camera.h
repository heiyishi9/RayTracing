#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

class camera {
public:
    camera(
        vec3 lookfrom, vec3 lookat, vec3 vup,
        double vfov,                            // ���ϵ��£��Զ���Ϊ��λ
        double aspect, 
        double aperture, double focus_dist      // �׾��ͽ���
    ) 
    {
        origin = lookfrom;
        lens_radius = aperture / 2;

        auto theta = degrees_to_radians(vfov);
        auto half_height = tan(theta / 2);
        auto half_width = aspect * half_height;

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);
        lower_left_corner = origin
            - half_width * focus_dist * u
            - half_height * focus_dist * v
            - focus_dist * w;

        horizontal = 2 * half_width * focus_dist * u;
        vertical = 2 * half_height * focus_dist * v;
    }

    ray get_ray(double u, double v) 
    {
        return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
    }

public:
    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;               // �������xyz
    double lens_radius;         // ������׾�(��Ȧ)�İ뾶
};
#endif