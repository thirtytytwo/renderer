# Renderer 项目实现记录

## 目录

- [项目概述](#项目概述)
- [SDL3 窗口初始化](#sdl3-窗口初始化)
- [图片加载 (stb\_image)](#图片加载-stb_image)
- [SDL 事件循环](#sdl-事件循环)
- [CPU 软件渲染](#cpu-软件渲染)
- [OpenMP 并行计算](#openmp-并行计算)
- [SIMD Vector4 实现](#simd-vector4-实现)

---

## 项目概述

### 目的
构建一个纯 CPU 渲染的 SDL3 应用程序，不依赖 GPU 层，实现图片加载、像素操作和并行计算。

### 环境
- **平台**: Windows 11
- **图形库**: SDL3
- **图片库**: stb_image
- **并行计算**: OpenMP
- **SIMD**: SSE/AVX

---

## SDL3 窗口初始化

### 目的
创建可调整大小的 SDL 窗口作为渲染目标。

### 相关代码
```cpp
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

// 初始化 SDL 视频子系统
SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

// 创建窗口
SDL_Window* WindowHandle = SDL_CreateWindow(
    TITLE.c_str(),
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    SDL_WINDOW_RESIZABLE  // 窗口标志
);

// 获取窗口 Surface
SDL_Surface* SurfaceHandle = SDL_GetWindowSurface(WindowHandle);
```

### 用到的函数
| 函数 | 说明 |
|------|------|
| `SDL_Init()` | 初始化 SDL 子系统 |
| `SDL_CreateWindow()` | 创建窗口 |
| `SDL_GetWindowSurface()` | 获取窗口关联的 Surface |
| `SDL_WINDOW_RESIZABLE` | 窗口可调整大小标志 |

---

## 图片加载 (stb_image)

### 目的
使用 stb_image 库读取 PNG/JPG 等格式图片，获取像素数据。

### 相关代码
```cpp
#define STB_IMAGE_IMPLEMENTATION  // 在包含头文件前定义
#include <stb_image.h>

// 读取图片
int imgWidth, imgHeight, imgChannels;
unsigned char* pixelData = stbi_load(
    imgPath.c_str(),  // 图片路径
    &imgWidth,        // 输出：宽度
    &imgHeight,       // 输出：高度
    &imgChannels,     // 输出：通道数
    4                 // 强制 RGBA 4通道
);

// 释放内存
stbi_image_free(pixelData);

// 获取错误信息
stbi_failure_reason();
```

### 用到的函数
| 函数 | 说明 |
|------|------|
| `stbi_load()` | 加载图片并返回像素数据 |
| `stbi_image_free()` | 释放 stb_image 分配的内存 |
| `stbi_failure_reason()` | 获取加载失败的原因 |

### 支持格式
PNG, JPEG, BMP, TGA, HDR, GIF 等

---

## SDL 事件循环

### 目的
处理用户输入，实现窗口关闭和 ESC 退出功能。

### 相关代码
```cpp
bool isRunning = true;
SDL_Event event;

while (isRunning)
{
    // 处理所有待处理事件
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                    isRunning = false;
                break;
        }
    }

    // 刷新窗口表面
    SDL_UpdateWindowSurface(WindowHandle);
}
```

### 用到的函数
| 函数 | 说明 |
|------|------|
| `SDL_PollEvent()` | 非阻塞获取事件 |
| `SDL_EVENT_QUIT` | 窗口关闭事件 |
| `SDL_EVENT_KEY_DOWN` | 键盘按下事件 |
| `SDLK_ESCAPE` | ESC 键码 |
| `SDL_UpdateWindowSurface()` | 更新窗口 Surface |

---

## CPU 软件渲染

### 目的
直接操作 Surface 像素，不依赖 GPU，实现纯 CPU 渲染。

### 相关代码
```cpp
// 锁定 Surface 以便直接操作像素
SDL_LockSurface(SurfaceHandle);

Uint32* DestPixels = (Uint32*)SurfaceHandle->pixels;

// 居中显示图片
int offsetX = (SCREEN_WIDTH - imgWidth) / 2;
int offsetY = (SCREEN_HEIGHT - imgHeight) / 2;

for (int y = 0; y < imgHeight; y++)
{
    for (int x = 0; x < imgWidth; x++)
    {
        int destX = x + offsetX;
        int destY = y + offsetY;

        if (destX >= 0 && destX < SCREEN_WIDTH &&
            destY >= 0 && destY < SCREEN_HEIGHT)
        {
            // 获取像素颜色
            Uint32 Color = SDL_MapRGB(
                SDL_GetPixelFormatDetails(SurfaceHandle->format),
                0,  // palette index
                pixelData[(y * imgWidth + x) * 4 + 0],  // R
                pixelData[(y * imgWidth + x) * 4 + 1],  // G
                pixelData[(y * imgWidth + x) * 4 + 2]   // B
            );
            DestPixels[destY * SCREEN_WIDTH + destX] = Color;
        }
    }
}

SDL_UnlockSurface(SurfaceHandle);
```

### 用到的函数
| 函数 | 说明 |
|------|------|
| `SDL_LockSurface()` | 锁定 Surface 以便直接访问像素 |
| `SDL_UnlockSurface()` | 解锁 Surface |
| `SDL_GetPixelFormatDetails()` | 获取像素格式详情 (SDL3) |
| `SDL_MapRGB()` | 将 RGB 转换为像素格式 |

---

## OpenMP 并行计算

### 目的
使用多核 CPU 并行处理像素数据，加速渲染。

### CMakeLists.txt 配置
```cmake
cmake_minimum_required(VERSION 3.10)
project(Renderer)

# 启用 OpenMP
find_package(OpenMP)

# ... 其他配置 ...

# 链接 OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()
```

### 相关代码
```cpp
#include <omp.h>

#pragma omp parallel for
for (int y = 0; y < SCREEN_HEIGHT; y++)
{
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        DestPixels[y * SCREEN_WIDTH + x] = 0xFF0000FF;
    }
}
```

### 用到的函数/pragma
| 函数/pragma | 说明 |
|-------------|------|
| `find_package(OpenMP)` | CMake 查找 OpenMP |
| `OpenMP::OpenMP_CXX` | OpenMP C++ 链接库 |
| `#pragma omp parallel for` | 并行化 for 循环 |

---

## SIMD Vector4 实现

### 目的
使用 SIMD 指令加速向量运算，提供 32 位和 64 位两种精度。

### 文件结构
- `Vec4f`: 4 x 32-bit floats (SSE `__m128`)
- `Vec4d`: 4 x 64-bit doubles (AVX `__m256d`)

### 头文件
```cpp
#include <immintrin.h>
#include <cmath>
#include <ostream>
```

### Vec4f 实现 (SSE)
```cpp
struct Vec4f
{
    union
    {
        __m128 simd;
        float data[4];
        struct { float x, y, z, w; };
    };

    // 构造器
    Vec4f() : simd(_mm_setzero_ps()) {}
    Vec4f(__m128 m) : simd(m) {}
    Vec4f(float x, float y, float z, float w) : simd(_mm_set_ps(w, z, y, x)) {}
    explicit Vec4f(float scalar) : simd(_mm_set1_ps(scalar)) {}

    // SIMD 运算
    Vec4f operator+(const Vec4f& rhs) const
        { return Vec4f(_mm_add_ps(simd, rhs.simd)); }
    Vec4f operator-(const Vec4f& rhs) const
        { return Vec4f(_mm_sub_ps(simd, rhs.simd)); }
    Vec4f operator*(const Vec4f& rhs) const
        { return Vec4f(_mm_mul_ps(simd, rhs.simd)); }
    Vec4f operator/(const Vec4f& rhs) const
        { return Vec4f(_mm_div_ps(simd, rhs.simd)); }

    // 点积
    float dot(const Vec4f& rhs) const
    {
        __m128 prod = _mm_mul_ps(simd, rhs.simd);
        prod = _mm_add_ps(prod, _mm_movehl_ps(prod, prod));
        prod = _mm_add_ss(prod, _mm_shuffle_ps(prod, prod, 1));
        return _mm_cvtss_f32(prod);
    }
};
```

### Vec4d 实现 (AVX)
```cpp
struct Vec4d
{
    union
    {
        __m256d simd;      // 4 x 64-bit doubles
        double data[4];
        struct { double x, y, z, w; };
    };

    // SIMD 运算
    Vec4d operator+(const Vec4d& rhs) const
        { return Vec4d(_mm256_add_pd(simd, rhs.simd)); }
    Vec4d operator-(const Vec4d& rhs) const
        { return Vec4d(_mm256_sub_pd(simd, rhs.simd)); }
    Vec4d operator*(const Vec4d& rhs) const
        { return Vec4d(_mm256_mul_pd(simd, rhs.simd)); }
    Vec4d operator/(const Vec4d& rhs) const
        { return Vec4d(_mm256_div_pd(simd, rhs.simd)); }
};
```

### SIMD 函数对照表

| 操作 | Vec4f (SSE) | Vec4d (AVX) |
|------|-------------|-------------|
| 加法 | `_mm_add_ps` | `_mm256_add_pd` |
| 减法 | `_mm_sub_ps` | `_mm256_sub_pd` |
| 乘法 | `_mm_mul_ps` | `_mm256_mul_pd` |
| 除法 | `_mm_div_ps` | `_mm256_div_pd` |
| 置零 | `_mm_setzero_ps` | `_mm256_setzero_pd` |
| 置1 | `_mm_set1_ps` | `_mm256_set1_pd` |
| 置值 | `_mm_set_ps` | `_mm256_set_pd` |

### SIMD Intrinsic 说明

| Intrinsic | 说明 |
|-----------|------|
| `__m128` | 128-bit SIMD 寄存器 (4 x 32-bit floats) |
| `__m256d` | 256-bit SIMD 寄存器 (4 x 64-bit doubles) |
| `_mm_set_ps(w,z,y,x)` | 创建 4-float 向量 |
| `_mm256_set_pd(w,z,y,x)` | 创建 4-double 向量 |
| `_mm_set1_ps(x)` | 创建所有元素相同的 4-float 向量 |
| `_mm_movehl_ps(a,b)` | 移动高位 |
| `_mm_shuffle_ps(a,b,imm)` | 混洗 |
| `_mm_cvtss_f32` | 提取 scalar float |

---

## 总结

### 项目技术栈
| 类别 | 技术 |
|------|------|
| 窗口 | SDL3 |
| 图片加载 | stb_image |
| 事件处理 | SDL Events |
| 并行计算 | OpenMP |
| SIMD | SSE (Vec4f) / AVX (Vec4d) |

### 关键文件
- `src/main.cpp` - 主程序
- `src/Mesh.h` - SIMD Vector4 实现
- `CMakeLists.txt` - 构建配置
