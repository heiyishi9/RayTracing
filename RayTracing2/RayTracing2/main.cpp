#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "hittable_list.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"

// 射线颜色
vec3 ray_color(const ray& r, const vec3& background, const hittable& world, int depth)
{
    hit_record rec;

    // 如果我们超过了光线反射的极限，就不会有更多的光被收集起来。
    if (depth <= 0)
        return vec3(0, 0, 0);

    // 潜在bug：有些物体反射的光线会在t=0时再次击中自己，所有设定为0.001
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    vec3 attenuation;
    vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

// 场景：随机生成许多球
hittable_list random_scene() 
{
    hittable_list world;

    auto checker = make_shared<checker_texture>(
        make_shared<constant_texture>(vec3(0.2, 0.3, 0.1)),
        make_shared<constant_texture>(vec3(0.9, 0.9, 0.9)));
    world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    int i = 1;
    for (int a = -5; a < 5; a++) 
    {
        for (int b = -5; b < 5; b++) 
        {
            auto choose_mat = random_double();
            vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) 
            {
                if (choose_mat < 0.8) 
                {
                    // diffuse
                    auto albedo = vec3::random() * vec3::random();
                    world.add(make_shared<moving_sphere>(
                        center, center + vec3(0, random_double(0, .5), 0), 0.0, 1.0, 0.2,
                        make_shared<lambertian>(albedo)));
                }
                else if (choose_mat < 0.95) 
                {
                    // metal
                    auto albedo = vec3::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    world.add(make_shared<sphere>(center, 0.2, make_shared<metal>(albedo, fuzz)));
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

    return static_cast<hittable_list>(make_shared<bvh_node>(world, 0, 1));;
}

// 场景：两个球
hittable_list two_spheres()
{
    hittable_list objects;

    auto checker = make_shared<checker_texture>(
        make_shared<constant_texture>(vec3(0.2, 0.3, 0.1)),
        make_shared<constant_texture>(vec3(0.9, 0.9, 0.9)));

    objects.add(make_shared<sphere>(vec3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(vec3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

// 场景：两个柏林噪声的球
hittable_list two_perlin_spheres()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(vec3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

// 场景：地球
hittable_list earth() {
    int nx, ny, nn;
    unsigned char* texture_data = stbi_load("image/earthmap.jpg", &nx, &ny, &nn, 0);

    auto earth_surface = make_shared<lambertian>(make_shared<image_texture>(texture_data, nx, ny));
    auto globe = make_shared<sphere>(vec3(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

// 场景：简单的矩形光源
hittable_list simple_light() 
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(vec3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(make_shared<constant_texture>(vec3(4, 4, 4)));
    objects.add(make_shared<sphere>(vec3(0, 7, 0), 2, difflight));      // 球光源
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));       // 矩形光源

    return objects;
}

// 场景：康奈尔盒子，五堵墙+两个长方体
hittable_list cornell_box() 
{
    hittable_list objects;

    auto red = make_shared<lambertian>(vec3(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(vec3(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(vec3(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(vec3(15, 15, 15));

    // 康奈尔盒子
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    // 两个立方体
    shared_ptr<hittable> box1 = make_shared<box>(vec3(0, 0, 0), vec3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    objects.add(box1);
    shared_ptr<hittable> box2 = make_shared<box>(vec3(0, 0, 0), vec3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));
    objects.add(box2);

    return objects;
}

// 场景：康奈尔盒子，五堵墙+两种体积体(深色和浅色粒子)
hittable_list cornell_smoke()
{
    hittable_list objects;

    auto red = make_shared<lambertian>(vec3(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(vec3(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(vec3(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(vec3(15, 15, 15));

    // 康奈尔盒子
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(vec3(0, 0, 0), vec3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    shared_ptr<hittable> box2 = make_shared<box>(vec3(0, 0, 0), vec3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));

    objects.add(make_shared<constant_medium>(box1, 0.01, vec3(0, 0, 0)));
    objects.add(make_shared<constant_medium>(box2, 0.01, vec3(1, 1, 1)));

    return objects;
}

// 场景：普通球、移动球、地球、玻璃球、金属球、矩形光源、大理石球、立方体
hittable_list final_scene()
{
    // 很多很多立方体作为地面
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(vec3(0.48, 0.83, 0.53));

    // 总计400
    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++)
    {
        for (int j = 0; j < boxes_per_side; j++)
        {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto y0 = 0.0;
            auto z0 = -1000.0 + j * w;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(vec3(x0, y0, z0), vec3(x1, y1, z1), ground));
        }
    }

    // 物体
    hittable_list objects;
    
    // 地面
    objects.add(make_shared<bvh_node>(boxes1, 0, 1));
    // 矩形光源
    auto light = make_shared<diffuse_light>(vec3(15, 15, 15));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    // 运动球（做运动模糊）
    auto center1 = vec3(400, 400, 200);
    auto center2 = center1 + vec3(30, 0, 0);
    auto moving_sphere_material = make_shared<lambertian>(vec3(0.7, 0.3, 0.1));
    objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    // 玻璃球和金属球
    objects.add(make_shared<sphere>(vec3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    objects.add(make_shared<sphere>(vec3(0, 150, 145), 50, make_shared<metal>(vec3(0.8, 0.8, 0.9), 1.0)));

    // 蓝色次表面反射球（在电介质dielectric内部填充体积体）
    auto boundary = make_shared<sphere>(vec3(360, 150, 145), 50, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, vec3(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(vec3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    objects.add(make_shared<constant_medium>(boundary, 0.0001, vec3(1, 1, 1)));

    // 地球
    int nx, ny, nn;
    auto tex_data = stbi_load("image/earthmap.jpg", &nx, &ny, &nn, 0);
    auto emat = make_shared<lambertian>(make_shared<image_texture>(tex_data, nx, ny));
    objects.add(make_shared<sphere>(vec3(400, 200, 400), 100, emat));

    // 大理石球（柏林噪声）
    auto pertext = make_shared<noise_texture>(0.1);
    objects.add(make_shared<sphere>(vec3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

    // 1000个球组成的立方体
    hittable_list boxes2;
    auto white = make_shared<lambertian>(vec3(0.73, 0.73, 0.73));
    int ns = 1000;
    for (int j = 0; j < ns; j++)
    {
        boxes2.add(make_shared<sphere>(vec3::random(0, 165), 10, white));
    }
    objects.add(make_shared<translate>
               (make_shared<rotate_y>
               (make_shared<bvh_node>(boxes2, 0.0, 1.0), 15), vec3(-100, 270, 395)));

    return objects;
}

int main() 
{
    int image_width = 1000;
    int image_height = 500;
    int samples_per_pixel = 100;    // 采样数（计算多重采样）
    int max_depth = 50;             // depth 限制 ray_color 递归层数
    auto aspect_ratio = double(image_width) / image_height;   // 纵横比
    vec3 background(0, 0, 0);       // 背景颜色，默认黑色
    hittable_list world;            // 物体
    // 摄像机
    vec3 lookfrom(13, 2, 3);        // 摄像机位置
    vec3 lookat(0, 0, 0);           // 摄像机看向位置
    vec3 vup(0, 1, 0);              // 世界坐标的上方向
    auto vfov = 40.0;               // 从上到下，以度数为单位
    auto dist_to_focus = 10.0;      // 焦距
    auto aperture = 0.0;            // 相机孔径（光圈）

    // 选择场景以及调整摄像机
    switch (8)
    {
    case 1:
        world = random_scene();
        background = vec3(0.70, 0.80, 1.00);
        lookfrom = vec3(13, 2, 3);
        lookat = vec3(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        break;
    case 2:
        world = two_spheres();
        background = vec3(0.70, 0.80, 1.00);
        lookfrom = vec3(13, 2, 3);
        lookat = vec3(0, 0, 0);
        vfov = 20.0;
        break;
    case 3:
        world = two_perlin_spheres();
        background = vec3(0.70, 0.80, 1.00);
        lookfrom = vec3(13, 2, 3);
        lookat = vec3(0, 0, 0);
        vfov = 20.0;
    case 4:
        world = earth();
        background = vec3(0.70, 0.80, 1.00);
        lookfrom = vec3(13, 2, 3);
        lookat = vec3(0, 0, 0);
        vfov = 20.0;
        break;
    case 5:
        world = simple_light();
        samples_per_pixel = 400;
        background = vec3(0, 0, 0);
        lookfrom = vec3(26, 3, 6);
        lookat = vec3(0, 2, 0);
        vfov = 20.0;
        break;
    case 6:
        world = cornell_box();
        image_width = 600;
        image_height = 600;
        aspect_ratio = 1.0;
        background = vec3(0, 0, 0);
        samples_per_pixel = 200;            // 调整采样数获得更加清晰的图片，同时时间更长
        lookfrom = vec3(278, 278, -800);
        lookat = vec3(278, 278, 0);
        vfov = 40.0;
        break;
    case 7:
        world = cornell_smoke();
        image_width = 600;
        image_height = 600;
        aspect_ratio = 1.0;
        samples_per_pixel = 200;            // 调整采样数获得更加清晰的图片，同时时间更长
        lookfrom = vec3(278, 278, -800);
        lookat = vec3(278, 278, 0);
        vfov = 40.0;
        break;
    case 8:
        world = final_scene();
        image_width = 800;
        image_height = 800;
        aspect_ratio = 1.0;
        samples_per_pixel = 10000;            // 调整采样数获得更加清晰的图片，同时时间更长
        background = vec3(0, 0, 0);
        lookfrom = vec3(478, 278, -600);
        lookat = vec3(278, 278, 0);
        vfov = 40.0;
        break;
    default:
        break;
    }

    std::string render_target = "image/image.ppm";
    std::ofstream ppm_file(render_target, std::ios::binary);
    ppm_file << "P6\n" << image_width << ' ' << image_height << "\n255\n";

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

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
                color += ray_color(r, background, world, max_depth);
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
