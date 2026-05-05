#ifndef RENDERER_MATH_INCLUDE
#define RENDERER_MATH_INCLUDE

#include <immintrin.h>
#include <cmath>
#include <ostream>

// ============================================================
// Vec4f: 4 x 32-bit floats (128-bit SIMD, 使用 SSE)
// ============================================================
struct Vec4f
{
    union
    {
        __m128 simd;
        float data[4];
        struct { float x, y, z, w; };
    };

    Vec4f() : simd(_mm_setzero_ps()) {}
    Vec4f(__m128 m) : simd(m) {}
    Vec4f(float x, float y, float z, float w) : simd(_mm_set_ps(w, z, y, x)) {}
    explicit Vec4f(float scalar) : simd(_mm_set1_ps(scalar)) {}

    float& operator[](int i) { return data[i]; }
    float operator[](int i) const { return data[i]; }

    Vec4f operator+(const Vec4f& rhs) const
    {
        return Vec4f(_mm_add_ps(simd, rhs.simd));
    }

    Vec4f operator-(const Vec4f& rhs) const
    {
        return Vec4f(_mm_sub_ps(simd, rhs.simd));
    }

    Vec4f operator*(const Vec4f& rhs) const
    {
        return Vec4f(_mm_mul_ps(simd, rhs.simd));
    }

    Vec4f operator/(const Vec4f& rhs) const
    {
        return Vec4f(_mm_div_ps(simd, rhs.simd));
    }

    Vec4f operator+(float scalar) const { return *this + Vec4f(scalar); }
    Vec4f operator-(float scalar) const { return *this - Vec4f(scalar); }
    Vec4f operator*(float scalar) const { return *this * Vec4f(scalar); }
    Vec4f operator/(float scalar) const { return *this / Vec4f(scalar); }

    friend Vec4f operator+(float lhs, const Vec4f& rhs) { return Vec4f(lhs) + rhs; }
    friend Vec4f operator-(float lhs, const Vec4f& rhs) { return Vec4f(lhs) - rhs; }
    friend Vec4f operator*(float lhs, const Vec4f& rhs) { return Vec4f(lhs) * rhs; }
    friend Vec4f operator/(float lhs, const Vec4f& rhs) { return Vec4f(lhs) / rhs; }

    Vec4f& operator+=(const Vec4f& rhs) { simd = _mm_add_ps(simd, rhs.simd); return *this; }
    Vec4f& operator-=(const Vec4f& rhs) { simd = _mm_sub_ps(simd, rhs.simd); return *this; }
    Vec4f& operator*=(const Vec4f& rhs) { simd = _mm_mul_ps(simd, rhs.simd); return *this; }
    Vec4f& operator/=(const Vec4f& rhs) { simd = _mm_div_ps(simd, rhs.simd); return *this; }

    Vec4f operator-() const
    {
        return Vec4f(_mm_sub_ps(_mm_setzero_ps(), simd));
    }

    float dot(const Vec4f& rhs) const
    {
        __m128 prod = _mm_mul_ps(simd, rhs.simd);
        prod = _mm_add_ps(prod, _mm_movehl_ps(prod, prod));
        prod = _mm_add_ss(prod, _mm_shuffle_ps(prod, prod, 1));
        return _mm_cvtss_f32(prod);
    }

    Vec4f cross(const Vec4f& rhs) const
    {
        return Vec4f(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x,
            0.0f
        );
    }

    float cross2D(const Vec4f& rhs) const
    {
        return x * rhs.y - y * rhs.x;
    }

    float lengthSq() const { return dot(*this); }

    float length() const { return std::sqrt(lengthSq()); }

    Vec4f normalized() const
    {
        float len = length();
        return (len > 0.0f) ? *this / len : Vec4f(0.0f);
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec4f& v)
    {
        return os << "Vec4f(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    }
};


// ============================================================
// Vec4d: 4 x 64-bit doubles (256-bit SIMD, 使用 AVX)
// ============================================================
struct Vec4d
{
    union
    {
        __m256d simd;
        double data[4];
        struct { double x, y, z, w; };
    };

    Vec4d() : simd(_mm256_setzero_pd()) {}
    Vec4d(__m256d m) : simd(m) {}
    Vec4d(double x, double y, double z, double w) : simd(_mm256_set_pd(w, z, y, x)) {}
    explicit Vec4d(double scalar) : simd(_mm256_set1_pd(scalar)) {}

    double& operator[](int i) { return data[i]; }
    double operator[](int i) const { return data[i]; }

    Vec4d operator+(const Vec4d& rhs) const
    {
        return Vec4d(_mm256_add_pd(simd, rhs.simd));
    }

    Vec4d operator-(const Vec4d& rhs) const
    {
        return Vec4d(_mm256_sub_pd(simd, rhs.simd));
    }

    Vec4d operator*(const Vec4d& rhs) const
    {
        return Vec4d(_mm256_mul_pd(simd, rhs.simd));
    }

    Vec4d operator/(const Vec4d& rhs) const
    {
        return Vec4d(_mm256_div_pd(simd, rhs.simd));
    }

    Vec4d operator+(double scalar) const { return *this + Vec4d(scalar); }
    Vec4d operator-(double scalar) const { return *this - Vec4d(scalar); }
    Vec4d operator*(double scalar) const { return *this * Vec4d(scalar); }
    Vec4d operator/(double scalar) const { return *this / Vec4d(scalar); }

    friend Vec4d operator+(double lhs, const Vec4d& rhs) { return Vec4d(lhs) + rhs; }
    friend Vec4d operator-(double lhs, const Vec4d& rhs) { return Vec4d(lhs) - rhs; }
    friend Vec4d operator*(double lhs, const Vec4d& rhs) { return Vec4d(lhs) * rhs; }
    friend Vec4d operator/(double lhs, const Vec4d& rhs) { return Vec4d(lhs) / rhs; }

    Vec4d& operator+=(const Vec4d& rhs) { simd = _mm256_add_pd(simd, rhs.simd); return *this; }
    Vec4d& operator-=(const Vec4d& rhs) { simd = _mm256_sub_pd(simd, rhs.simd); return *this; }
    Vec4d& operator*=(const Vec4d& rhs) { simd = _mm256_mul_pd(simd, rhs.simd); return *this; }
    Vec4d& operator/=(const Vec4d& rhs) { simd = _mm256_div_pd(simd, rhs.simd); return *this; }

    Vec4d operator-() const
    {
        return Vec4d(_mm256_sub_pd(_mm256_setzero_pd(), simd));
    }

    double dot(const Vec4d& rhs) const
    {
        __m256d prod = _mm256_mul_pd(simd, rhs.simd);
        __m128d sum128 = _mm_add_pd(_mm256_castpd256_pd128(prod),
                                    _mm256_extractf128_pd(prod, 1));
        sum128 = _mm_hadd_pd(sum128, sum128);
        return _mm_cvtsd_f64(sum128);
    }

    double lengthSq() const { return dot(*this); }

    double length() const { return std::sqrt(lengthSq()); }

    Vec4d normalized() const
    {
        double len = length();
        return (len > 0.0) ? *this / len : Vec4d(0.0);
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec4d& v)
    {
        return os << "Vec4d(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    }
};


// ============================================================
// 便捷别名
// ============================================================
using Vec4 = Vec4f;
using Vec4hp = Vec4d;

// ============================================================
// Mat4: 4×4 矩阵 (列主序, SIMD 加速)
// ============================================================
struct Mat4
{
    Vec4f cols[4];

    Mat4()
    {
        cols[0] = Vec4f(1, 0, 0, 0);
        cols[1] = Vec4f(0, 1, 0, 0);
        cols[2] = Vec4f(0, 0, 1, 0);
        cols[3] = Vec4f(0, 0, 0, 1);
    }

    Mat4(Vec4f c0, Vec4f c1, Vec4f c2, Vec4f c3)
    {
        cols[0] = c0;
        cols[1] = c1;
        cols[2] = c2;
        cols[3] = c3;
    }

    Vec4f& operator[](int i) { return cols[i]; }
    const Vec4f& operator[](int i) const { return cols[i]; }

    Vec4f operator*(const Vec4f& v) const
    {
        Vec4f result;
        result.simd = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(cols[0].simd, _mm_set1_ps(v.x)),
                _mm_mul_ps(cols[1].simd, _mm_set1_ps(v.y))
            ),
            _mm_add_ps(
                _mm_mul_ps(cols[2].simd, _mm_set1_ps(v.z)),
                _mm_mul_ps(cols[3].simd, _mm_set1_ps(v.w))
            )
        );
        return result;
    }

    Mat4 operator*(const Mat4& rhs) const
    {
        Mat4 result;
        for (int i = 0; i < 4; i++)
            result.cols[i] = *this * rhs.cols[i];
        return result;
    }

    static Mat4 Identity()
    {
        return Mat4();
    }

    static Mat4 Translation(float tx, float ty, float tz)
    {
        Mat4 m;
        m.cols[0] = Vec4f(1, 0, 0, 0);
        m.cols[1] = Vec4f(0, 1, 0, 0);
        m.cols[2] = Vec4f(0, 0, 1, 0);
        m.cols[3] = Vec4f(tx, ty, tz, 1);
        return m;
    }

    static Mat4 Scale(float sx, float sy, float sz)
    {
        Mat4 m;
        m.cols[0] = Vec4f(sx, 0, 0, 0);
        m.cols[1] = Vec4f(0, sy, 0, 0);
        m.cols[2] = Vec4f(0, 0, sz, 0);
        m.cols[3] = Vec4f(0, 0, 0, 1);
        return m;
    }

    static Mat4 RotationX(float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Mat4 m;
        m.cols[0] = Vec4f(1, 0, 0, 0);
        m.cols[1] = Vec4f(0, c, s, 0);
        m.cols[2] = Vec4f(0, -s, c, 0);
        m.cols[3] = Vec4f(0, 0, 0, 1);
        return m;
    }

    static Mat4 RotationY(float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Mat4 m;
        m.cols[0] = Vec4f(c, 0, -s, 0);
        m.cols[1] = Vec4f(0, 1, 0, 0);
        m.cols[2] = Vec4f(s, 0, c, 0);
        m.cols[3] = Vec4f(0, 0, 0, 1);
        return m;
    }

    static Mat4 RotationZ(float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Mat4 m;
        m.cols[0] = Vec4f(c, s, 0, 0);
        m.cols[1] = Vec4f(-s, c, 0, 0);
        m.cols[2] = Vec4f(0, 0, 1, 0);
        m.cols[3] = Vec4f(0, 0, 0, 1);
        return m;
    }

    static Mat4 LookAt(const Vec4f& eye, const Vec4f& target, const Vec4f& up)
    {
        Vec4f f = (target - eye).normalized();
        f.w = 0.0f;
        Vec4f s = f.cross(up).normalized();
        Vec4f u = s.cross(f);

        Mat4 m;
        m.cols[0] = Vec4f(s.x, u.x, -f.x, 0);
        m.cols[1] = Vec4f(s.y, u.y, -f.y, 0);
        m.cols[2] = Vec4f(s.z, u.z, -f.z, 0);
        m.cols[3] = Vec4f(-s.dot(eye), -u.dot(eye), f.dot(eye), 1);
        return m;
    }

    static Mat4 Perspective(float fov, float aspect, float near, float far)
    {
        float f = 1.0f / std::tan(fov * 0.5f);
        Mat4 m;
        m.cols[0] = Vec4f(f / aspect, 0, 0, 0);
        m.cols[1] = Vec4f(0, f, 0, 0);
        m.cols[2] = Vec4f(0, 0, -(far + near) / (far - near), -1);
        m.cols[3] = Vec4f(0, 0, -2 * far * near / (far - near), 0);
        return m;
    }
};

// ============================================================
// 数学工具函数
// ============================================================
class Math
{
public:
    static constexpr float PI = 3.14159265358979323846f;

    static double SignedTriangleArea(Vec4 a, Vec4 b, Vec4 c)
    {
        return 0.5 * ((b.y - a.y) * (b.x + a.x) + (c.y - b.y) * (c.x + b.x) + (a.y - c.y) * (a.x + c.x));
    }
};

#endif