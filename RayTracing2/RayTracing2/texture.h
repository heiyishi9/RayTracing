#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "perlin.h"

class texture 
{
public:
    virtual vec3 value(double u, double v, const vec3& p) const = 0;
};

// 固定纹理
class constant_texture : public texture 
{
public:
    constant_texture() {}
    constant_texture(vec3 c) : color(c) {}

    virtual vec3 value(double u, double v, const vec3& p) const 
    {
        return color;
    }

public:
    vec3 color;
};

// 使用sine和cosine函数周期性的变化来做一个棋盘格纹理。
// 如果在三个维度都乘上这个周期函数, 就会形成一个3D的棋盘格模型。
class checker_texture : public texture 
{
public:
    checker_texture() {}
    checker_texture(shared_ptr<texture> t0, shared_ptr<texture> t1) : even(t0), odd(t1) {}
    checker_texture(vec3& c1, vec3& c2) 
        : even(make_shared<constant_texture>(c1)), odd(make_shared<constant_texture>(c2)) {}

    virtual vec3 value(double u, double v, const vec3& p) const 
    {
        auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
        if (sines < 0)
            return odd->value(u, v, p);
        else
            return even->value(u, v, p);
    }

public:
    // 奇偶格的指针可以指向一个静态纹理, 也可以指向一些程序生成的纹理。
    shared_ptr<texture> odd;
    shared_ptr<texture> even;
};

// 柏林噪声纹理
class noise_texture : public texture
{
public:
    noise_texture() {}
    noise_texture(double sc) : scale(sc) {}

    virtual vec3 value(double u, double v, const vec3& p) const
    {
        // 柏林插值结果可能为负数
        // 将输出映射到0到1之间
        //return vec3(1, 1, 1) * 0.5 * (1.0 + noise.noise(scale * p));
        // 大理石纹理：让颜色与sin函数的值成比例，并使用扰动函数取调整相位（平移sin(x)中的x）
        // 使得带状条纹起伏波荡
        return vec3(1, 1, 1) * 0.5 * (1 + sin(scale*p.z() + 10*noise.turb(p)));
    }

public:
    perlin noise;
    double scale;   // 变化频率，更快的发生变化
};

// 图片导入纹理
class image_texture : public texture {
public:
    image_texture() {}
    image_texture(unsigned char* pixels, int A, int B)
        : data(pixels), nx(A), ny(B) {}

    ~image_texture() {
        delete data;
    }

    virtual vec3 value(double u, double v, const vec3& p) const {
        // If we have no texture data, then always emit cyan (as a debugging aid).
        if (data == nullptr)
            return vec3(0, 1, 1);

        auto i = static_cast<int>((u)*nx);
        auto j = static_cast<int>((1 - v) * ny - 0.001);

        if (i < 0) i = 0;
        if (j < 0) j = 0;
        if (i > nx - 1) i = nx - 1;
        if (j > ny - 1) j = ny - 1;

        auto r = static_cast<int>(data[3 * i + 3 * nx * j + 0]) / 255.0;
        auto g = static_cast<int>(data[3 * i + 3 * nx * j + 1]) / 255.0;
        auto b = static_cast<int>(data[3 * i + 3 * nx * j + 2]) / 255.0;

        return vec3(r, g, b);
    }

public:
    unsigned char* data;
    int nx, ny;
};

#endif
