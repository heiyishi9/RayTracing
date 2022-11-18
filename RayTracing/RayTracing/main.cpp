#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

// ������ɫ
vec3 ray_color(const ray& r, const hittable& world, int depth)
{
    hit_record rec;

    // ������ǳ����˹��߷���ļ��ޣ��Ͳ����и���Ĺⱻ�ռ�������
    if (depth <= 0)
        return vec3(0, 0, 0);

    // Ǳ��bug����Щ���巴��Ĺ��߻���t=0ʱ�ٴλ����Լ��������趨Ϊ0.001
    if (world.hit(r, 0.001, infinity, rec))
    {
        ray scattered;
        vec3 attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return vec3(0, 0, 0);
    }

    // ������������yֵ��������ɫ���˸����Բ�ֵ�Ļ��
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

// ������������
hittable_list random_scene() 
{
    hittable_list world;

    world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(vec3(0.5, 0.5, 0.5))));

    int i = 1;
    for (int a = -11; a < 11; a++) 
    {
        for (int b = -11; b < 11; b++) 
        {
            auto choose_mat = random_double();
            vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) 
            {
                if (choose_mat < 0.8) 
                {
                    // diffuse
                    auto albedo = vec3::random() * vec3::random();
                    world.add(
                        make_shared<sphere>(center, 0.2, make_shared<lambertian>(albedo)));
                }
                else if (choose_mat < 0.95) 
                {
                    // metal
                    auto albedo = vec3::random(.5, 1);
                    auto fuzz = random_double(0, .5);
                    world.add(
                        make_shared<sphere>(center, 0.2, make_shared<metal>(albedo, fuzz)));
                }
                else 
                {
                    // glass
                    world.add(make_shared<sphere>(center, 0.2, make_shared<dielectric>(1.5)));
                }
            }
        }
    }

    world.add(make_shared<sphere>(vec3(0, 1, 0), 1.0, make_shared<dielectric>(1.5)));

    world.add(make_shared<sphere>(vec3(-4, 1, 0), 1.0, make_shared<lambertian>(vec3(0.4, 0.2, 0.1))));

    world.add(make_shared<sphere>(vec3(4, 1, 0), 1.0, make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));

    return world;
}

int main() 
{
    const int image_width = 1000;
    const int image_height = 500;
    const int samples_per_pixel = 100;  // ��������������ز�����
    const int max_depth = 50;           // depth ���� ray_color �ݹ����
    const auto aspect_ratio = double(image_width) / image_height;   // �ݺ��
    std::string render_target = "image/image.ppm";

    std::ofstream ppm_file(render_target, std::ios::binary);

    ppm_file << "P6\n" << image_width << ' ' << image_height << "\n255\n";

    // ����
    hittable_list world = random_scene();

    // �����
    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);              // ����������Ϸ���
    auto dist_to_focus = 10.0;      // ����
    auto aperture = 0.1;            // ����׾�����Ȧ��
    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    for (int j = image_height - 1; j >= 0; --j) 
    {
        std::cout << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) 
        {
            vec3 color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / image_width;
                auto v = (j + random_double()) / image_height;
                ray r = cam.get_ray(u, v);
                color += ray_color(r, world, max_depth);
            }
            color.write_color(ppm_file, samples_per_pixel);
        }
    }
    std::cout << "\nDone.\n";

    ppm_file.flush();
    ppm_file.close();

    // ppm -> png
    int w, h, channel;
    unsigned char* data = stbi_load(render_target.c_str(), &w, &h, &channel, 0);
    std::string output_image = "image/image.png";
    stbi_write_png(output_image.c_str(), w, h, channel, data, 0);

}
