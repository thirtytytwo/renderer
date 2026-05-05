# Renderer 项目代码索引

## 项目概述

纯 CPU 软件渲染器，基于 SDL3 窗口管理，使用 SIMD (SSE/AVX) 加速向量运算，不依赖 GPU。

- **语言**: C++17
- **构建系统**: CMake 3.10+
- **平台**: Windows 11
- **依赖库**: SDL3, stb_image (单头文件)

---

## 目录结构

```
Renderer/
├── src/                    # 源代码
│   ├── main.cpp            # 程序入口 & 主循环
│   ├── Math.h              # SIMD 向量类型 (Vec4f/Vec4d) + 数学工具类 (Math)
│   ├── Mesh.h              # 三角形顶点数据 (triangle[3])
│   ├── Mesh.cpp            # (空) 预留
│   └── Shader.h            # 软件光栅化着色器
├── include/                # 第三方头文件
│   ├── stb_image.h         # 图片加载库
│   └── SDL3/               # SDL3 头文件 (约90个)
├── lib/                    # 静态链接库
│   ├── SDL3.lib
│   ├── SDL3_test.lib
│   └── SDL3.pdb
├── resources/image/        # 资源文件
│   └── test.png            # 测试图片
├── memory/                 # 开发文档
│   └── renderer-implementation.md
└── CMakeLists.txt          # 构建配置
```

---

## 模块详解

### 1. main.cpp — 程序入口

| 行号范围 | 内容 |
|----------|------|
| 1-11 | 包含 & 宏定义 (`STB_IMAGE_IMPLEMENTATION`, `IMAGE_DIRECTORY`) |
| 14-16 | 全局常量 (`SCREEN_WIDTH=800`, `SCREEN_HEIGHT=600`, `TITLE="REnderer"`) |
| 18-37 | SDL 初始化 & 窗口创建 (`SDL_Init`, `SDL_CreateWindow`) |
| 39-60 | stb_image 加载图片 (强制 RGBA 4 通道) |
| 63-72 | 获取窗口 Surface (`SDL_GetWindowSurface`) |
| 74 | 创建 Shader 实例 |
| 77-141 | 主事件循环 (`SDL_PollEvent` → 渲染 → `SDL_UpdateWindowSurface`) |
| 107-137 | 渲染逻辑: 锁定 Surface → Shader.Render() → 解锁 Surface |
| 143-146 | 清理资源 (`stbi_image_free`, `SDL_DestroyWindow`, `SDL_Quit`) |

**关键调用链**: `main()` → `Shader::Render()` → `Shader::DoBarycentric()` → `Math::signed_triangle_area()`

**预留**: 第 113-136 行有 OpenMP 并行渲染的注释代码

### 2. Math.h — SIMD 向量数学 & 数学工具类

**包含关系**: `<immintrin.h>`, `<cmath>`, `<ostream>`

#### Vec4f (第 8-119 行) — 4×float, SSE `__m128`

| 行号 | 功能 |
|------|------|
| 10-15 | `union { __m128 simd; float data[4]; struct { float x,y,z,w; }; }` |
| 17-20 | 构造器: 默认零, `__m128`, `(x,y,z,w)`, 标量广播 |
| 22-23 | 下标访问: `operator[]` |
| 26-46 | 四则运算 (SIMD): `operator+`, `-`, `*`, `/` |
| 48-51 | 标量运算: `Vec4f ±*/ float` |
| 53-56 | 友元标量运算: `float ±*/ Vec4f` |
| 58-61 | 复合赋值: `+=`, `-=`, `*=`, `/=` |
| 63-66 | 取负: `operator-()` |
| 68-76 | **点积**: `_mm_mul_ps` → `_mm_movehl_ps` → `_mm_add_ss` → `_mm_cvtss_f32` |
| 78-86 | **3D 叉积**: `(ay*bz-az*by, az*bx-ax*bz, ax*by-ay*bx, 0)` |
| 88-92 | **2D 叉积**: `x*rhs.y - y*rhs.x` |
| 94 | 长度平方: `dot(*this)` |
| 96 | 长度: `sqrt(lengthSq())` |
| 98-102 | 归一化: `*this / len` (零保护) |
| 104-107 | `operator<<` 打印调试 |

#### Vec4d (第 113-210 行) — 4×double, AVX `__m256d`

与 Vec4f 结构对称，主要差异:
- `__m256d` 代替 `__m128`
- `_mm256_*_pd` 代替 `_mm_*_ps`
- 点积使用 `_mm_hadd_pd` 水平加法
- **无** 叉积方法

#### 别名 (第 214-215 行)

```cpp
using Vec4   = Vec4f;     // 默认单精度
using Vec4hp = Vec4d;     // 高精度
```

#### Math 类 (第 219-225 行) — 数学工具函数

| 方法 | 说明 |
|------|------|
| `static double SignedTriangleArea(Vec4 a, Vec4 b, Vec4 c)` | 有符号三角形面积公式，用于重心坐标计算 |

### 3. Mesh.h — 三角形顶点数据

**包含关系**: `"Math.h"`

```cpp
Vec4 triangle[3] = {
    Vec4(0.5, 1.0, 0.0, 0.0),
    Vec4(0.0, 0.0, 0.0, 0.0),
    Vec4(1.0, 0.0, 0.0, 0.0)
};
```

### 4. Shader.h — 软件光栅化着色器

**包含关系**: `<SDL3/SDL_stdinc.h>`, `<SDL3/SDL_pixels.h>`, `"Math.h"`, `"Mesh.h"`

| 行号 | 功能 |
|------|------|
| 10-12 | `class Shader`: 默认构造/析构 |
| 13-29 | **Render()**: 遍历所有像素, 调用 `DoBarycentric` 判断是否在三角形内, 写入白色像素 |
| 31-44 | **DoBarycentric()**: 重心坐标法判断点是否在三角形内 — 调用 `Math::SignedTriangleArea()` 计算面积, u/v/w 分量, 判断 u>=0 && v>=0 && w>=0 |
| 46-52 | **VertextShader()**: (未启用) 将归一化坐标乘以屏幕尺寸 |

**注意**: `Render()` 的参数类型是 `Vec4* mesh` (数组指针)，调用时传入全局 `triangle`

### 5. CMakeLists.txt — 构建配置

| 行号 | 内容 |
|------|------|
| 4-5 | C++17 标准 |
| 8 | OpenMP (已注释) |
| 10 | include 路径: `${CMAKE_SOURCE_DIR}/include` |
| 12 | lib 路径: `${CMAKE_SOURCE_DIR}/lib` |
| 15 | 源文件: `src/*.h src/*.cpp` |
| 18 | 链接: `SDL3` |
| 26-29 | 后期构建: 复制 resources 到输出目录 |

---

## 依赖关系图

```
main.cpp
  ├── SDL3/SDL.h, SDL3/SDL_main.h     (窗口/事件)
  ├── stb_image.h                      (图片加载, #define STB_IMAGE_IMPLEMENTATION)
  ├── Shader.h
  │     ├── SDL3/SDL_stdinc.h           (Uint32 等类型)
  │     ├── SDL3/SDL_pixels.h           (SDL_PixelFormat, SDL_MapRGB)
  │     ├── Math.h                       (Vec4f/Vec4d, Math 类)
  │     │     ├── <immintrin.h>         (SSE/AVX intrinsics)
  │     │     ├── <cmath>               (sqrt)
  │     │     └── <ostream>             (调试输出)
  │     └── Mesh.h                       (triangle[3] 顶点数据)
  │           └── Math.h                 (Vec4 类型依赖)
  └── → 链接 SDL3.lib
```

---

## 关键数据流

```
1. SDL_CreateWindow → SDL_GetWindowSurface → 获取像素缓冲区 (Uint32*)
2. stbi_load → 加载 RGBA 像素数据 (unsigned char*)
3. 主循环: SDL_PollEvent → SDL_LockSurface → Shader.Render(pixelData, triangle, w, h, fmt) → SDL_UnlockSurface → SDL_UpdateWindowSurface
4. Shader.Render: 遍历 (x,y) → DoBarycentric(归一化坐标) → Math::SignedTriangleArea() → 写入像素颜色
```

---

## 当前状态 & 已知问题

| 项目 | 状态 |
|------|------|
| 窗口创建 & 事件循环 | ✅ 正常 |
| SIMD Vec4f/Vec4d | ✅ 完整 (已移至 Math.h) |
| Math 类 (数学工具) | ✅ 已实现 (SignedTriangleArea) |
| 重心坐标光栅化 | ✅ 基本可用 |
| 图片加载 & 显示 | ⚠️ 加载了但未在 Render 中使用 |
| OpenMP 并行 | ❌ 已注释 |
| Mesh.cpp | ❌ 空文件 |
| VertextShader | ❌ 调用已注释 |

---

## 约定 & 编码风格

- 头文件保护宏: `RENDERER_*_INCLUDE`
- SIMD 优先: 向量运算全部使用 intrinsic 实现
- `Vec4` 是默认类型别名 (指向 `Vec4f`)
- 全局测试数据定义在 `Mesh.h` 中 (`triangle[3]`)
- 数学计算函数放在 `Math` 类中作为 `static` 方法
- 注释使用中文
- CMake 构建命令 (Windows): `cmake -B build; cmake --build build`