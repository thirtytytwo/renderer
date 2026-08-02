# Renderer 项目代码索引

## 项目概述

纯 CPU 软件渲染器，基于 MiniFB 做窗口管理与帧缓冲呈现，使用 SIMD (SSE/AVX) 加速向量运算，不依赖 GPU。

- **语言**: C++17
- **构建系统**: CMake 3.16+
- **平台**: Windows / macOS (Apple Silicon 通过 SIMDe 映射到 NEON)
- **依赖库**: MiniFB (源码内置于 `external/minifb`), stb_image (单头文件)

> 窗口后端已由 SDL3 迁移至 MiniFB。所有平台相关代码收敛在 `src/Platform/`，
> 渲染逻辑不再直接依赖任何窗口库类型。

---

## 平台抽象层 (src/Platform/)

| 文件 | 职责 |
|------|------|
| `PixelFormat.h` | `enum class PixelFormat` + `PackColor` / `PackColorF`，替代 `SDL_PixelFormat` / `SDL_MapRGB` |
| `Input.h` | `enum class Key` / `MouseButton` + `InputState`，替代 `SDL_SCANCODE_*` / `SDL_GetKeyboardState` |
| `Window.h` / `Window.cpp` | 窗口创建、事件轮询、帧缓冲呈现、帧间隔计时；唯一引用 MiniFB 的位置 |

`Window` 对上层暴露轮询式接口：`Create` / `PollEvents` / `Present` / `Input` / `DeltaTime`。
更换窗口后端只需重写 `Window.cpp`。

**注意**: Y 轴翻转已下沉到 `SimpleShader::Vertex`（`screenY = (1 - ndc.y) * 0.5 * height`），
不再需要整帧翻转操作。

---

## 目录结构

```
Renderer/
├── src/                    # 源代码
│   ├── main.cpp            # 程序入口 & 主循环
│   ├── Math.h              # SIMD 向量/矩阵 (Vec4f/Vec4d/Mat4) + 数学工具类
│   ├── Mesh.h / Mesh.cpp   # 网格数据
│   ├── Camera.h            # 摄像机 (依赖 Platform/Input.h)
│   ├── Light.h             # 平行光
│   ├── PixelBuffer.h       # 顶点阶段输出缓冲
│   ├── RenderObject.h      # 网格 + 着色器绑定
│   ├── Shader.h            # 光栅化基类
│   ├── SimpleShader.h      # 兰伯特着色实现
│   ├── Platform/           # 平台抽象层 (见上)
│   └── Render/
│       └── RenderResources.h  # RenderContext (颜色/深度缓冲)
├── include/                # 第三方头文件
│   └── stb_image.h         # 图片加载库
├── external/               # 源码形式的第三方依赖
│   ├── minifb/             # 窗口 / 事件 / 帧缓冲呈现
│   ├── simde/              # SSE/AVX → NEON 映射 (Apple Silicon)
│   └── compat/arm/         # immintrin.h 兼容垫片
├── resources/image/        # 资源文件
├── memory/                 # 开发文档
└── CMakeLists.txt          # 构建配置
```

### 构建注意事项

- **不要**把 `src/` 加入 include 搜索路径。macOS 文件系统大小写不敏感，
  `src/Math.h` 会遮蔽 libc++ 的 `<math.h>`，导致大量虚假的标准库报错。
  项目内头文件统一用相对路径引用（`.clangd` 中亦有对应的 `Remove` 规则）。
- 构建命令：`cmake -B build && cmake --build build`

---

## 模块详解

### 1. main.cpp — 程序入口

| 内容 |
|------|
| 创建 `Window`（含窗口 + 事件 + 呈现） |
| stb_image 加载图片 (强制 RGBA 4 通道) |
| 构建 `Mesh` / `RenderObject` / `RenderContext`（颜色 + 深度缓冲） |
| 主循环: `PollEvents` → 处理输入 → 更新 Uniform → `RenderObject::Render` → `Window::Present` |
| 清理资源 |

**关键调用链**: `main()` → `RenderObject::Render()` → `Shader::Render()` → `Shader::Vertex()` / `DoBarycentric()` / `Pixel()`

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

| 内容 |
|------|
| C++17 标准，导出 `compile_commands.json` |
| `add_subdirectory(external/minifb)`，关闭 MiniFB 自带示例 |
| 源文件: `src/*` + `src/Render/*` + `src/Platform/*` |
| 链接: `minifb`（macOS 上自动带 Cocoa / QuartzCore / Metal，Windows 上带 winmm / opengl32） |
| Apple Silicon: 追加 `external/compat/arm` + `external/simde` include 路径 |
| 后期构建: 复制 resources 到输出目录 |

---

## 依赖关系图

```
main.cpp
  ├── Platform/Window.h                  (窗口/事件/呈现)
  │     ├── Platform/Input.h             (Key / MouseButton / InputState)
  │     ├── Platform/PixelFormat.h       (PixelFormat, PackColor/PackColorF)
  │     └── Window.cpp → <MiniFB.h>      (唯一引用窗口库的位置) → 链接 minifb
  ├── stb_image.h                        (图片加载, #define STB_IMAGE_IMPLEMENTATION)
  ├── Camera.h → Platform/Input.h, Math.h
  ├── Light.h  → Math.h
  └── RenderObject.h
        ├── Mesh.h → Math.h
        └── Shader.h
              ├── <cstdint>, <algorithm>
              ├── Math.h                 (Vec4f/Vec4d/Mat4, Math 类)
              │     ├── <immintrin.h>    (SSE/AVX；ARM 上经 compat/arm 垫片走 SIMDe)
              │     ├── <cmath>
              │     └── <ostream>
              ├── PixelBuffer.h
              └── Render/RenderResources.h → Platform/PixelFormat.h
```

---

## 关键数据流

```
1. Window::Create → MiniFB 创建原生窗口
2. stbi_load → 加载 RGBA 像素数据 (unsigned char*)
3. 主循环: Window::PollEvents → Camera 更新 → Shader::SetUniforms
           → RenderObject::Render(ctx) → Window::Present(ctx.color, w, h)
4. Shader::Render: 清深度/颜色缓冲 → Vertex() 顶点变换 → 遍历 (x,y)
           → DoBarycentric() 重心坐标 + 深度测试 → Pixel() → PackColorF 写入颜色
```

---

## 当前状态 & 已知问题

| 项目 | 状态 |
|------|------|
| 窗口创建 & 事件循环 (MiniFB) | ✅ 正常，Windows / macOS 双平台 |
| 平台抽象层 (src/Platform) | ✅ 渲染逻辑已与窗口库解耦 |
| SIMD Vec4f/Vec4d/Mat4 | ✅ 完整 |
| 重心坐标光栅化 + 深度测试 | ✅ 基本可用 |
| 兰伯特光照 (SimpleShader) | ✅ 可用 |
| 窗口缩放 | ❌ 未处理，缓冲固定 800x600（创建时未加 RESIZABLE） |
| 图片加载 & 显示 | ⚠️ 加载了但未在 Render 中使用 |
| OpenMP 并行 | ❌ 已注释 |

---

## 约定 & 编码风格

- 头文件保护宏: `RENDERER_*_INCLUDE`
- SIMD 优先: 向量运算全部使用 intrinsic 实现
- `Vec4` 是默认类型别名 (指向 `Vec4f`)
- 整数类型使用 `<cstdint>` 的 `uint8_t/uint32_t` 等，不使用窗口库自带类型
- 平台相关代码只允许出现在 `src/Platform/`
- 数学计算函数放在 `Math` 类中作为 `static` 方法
- 注释使用中文
- CMake 构建命令: `cmake -B build && cmake --build build`