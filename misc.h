// -*- c++ -*-
#pragma once

#define _USE_MATH_DEFINES    // M_PI etc.. on msvc
#include <vector>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stdint.h>

#if _WIN32
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;
#endif

typedef int8_t i8;
typedef u_int8_t u8;
typedef int16_t i16;
typedef u_int16_t u16;
typedef int32_t i32;
typedef u_int32_t u32;
typedef int64_t i64;
typedef u_int64_t u64;
typedef float f32;
typedef double f64;

const f32 DEG_TO_RAD = 0.017453292519943295769236907684886f;
const f32 RAD_TO_DEG = 57.295779513082320876798154814105000f;

const f32 PI = f32(M_PI);
const f32 HALF_PI = f32(PI / 2);
const f32 TWO_PI = f32(PI * 2);
const f32 INV_PI = f32(1.0f / PI);

struct Vec2
{
  f32 x, y;
};

bool fuzzy_equal(f32 a, f32 b);

inline bool fuzzy_zero(f32 a)
{
  return fabsf(a) < FLT_EPSILON;
}

template <typename T>
inline T clamp(T x, T min, T max)
{
  if (x < min) return min;
  if (x > max) return max;
  return x;
}

template <typename T>
inline T clamp0n(T x, T n)
{
  if (x < T(0)) return T(0);
  if (x > n) return n;
  return x;
}

inline f32 clamp01(f32 x)
{
  return clamp0n(x, 1.0f);
}

inline f32 sqr(f32 a)
{
  return a * a;
}

inline f32 lerp(f32 a, f32 b, f32 t)
{
  return a * (1 - t) + b * t;
}

inline f32 ease_linear(f32 t)
{
  return t;
}

inline f32 ease_cubic_in(f32 t)
{
  return t * t * t;
}

inline f32 ease_cubic_out(f32 t)
{
  return 1 + (--t) * t * t;
}

inline f32 ease_cubic_in_out(f32 t)
{
  return t < 0.5 ? 4 * t * t * t : 1 + (--t) * (2 * (--t)) * (2 * t);
}

inline Vec2 vec2()
{
  return Vec2{0.0f, 0.0f};
}

inline Vec2 vec2(f32 v)
{
  return Vec2{v, v};
}

inline Vec2 vec2(f32 x, f32 y)
{
  return Vec2{x, y};
}

inline Vec2 operator+(const Vec2& a, const Vec2& b)
{
  return vec2(a.x + b.x, a.y + b.y);
}

inline Vec2 operator-(const Vec2& a, const Vec2& b)
{
  return vec2(a.x - b.x, a.y - b.y);
}

inline Vec2 operator*(const Vec2& a, f32 s)
{
  return vec2(a.x * s, a.y * s);
}

inline Vec2 operator/(const Vec2& a, f32 s)
{
  return vec2(a.x / s, a.y / s);
}

inline Vec2 operator-(const Vec2& a)
{
  return vec2(-a.x, -a.y);
}

inline f32 length2(const Vec2& a)
{
  return a.x * a.x + a.y * a.y;
}

inline f32 length(const Vec2& a)
{
  return sqrtf(length2(a));
}

inline Vec2 perp(const Vec2& a)
{
  return vec2(-a.y, a.x);
}

inline Vec2 normalized(const Vec2& a)
{
  f32 len = length(a);
  if (fuzzy_zero(len)) {
    return vec2();
  }
  return a / len;
}

inline bool operator==(const Vec2& a, const Vec2& b)
{
  return fuzzy_equal(a.x, b.x) && fuzzy_equal(a.y, b.y);
}

inline bool operator!=(const Vec2& a, const Vec2& b)
{
  return !(a == b);
}

inline f32 angles_diff(f32 a, f32 b)
{
  return atan2f(sinf(b - a), cosf(b - a));
}

inline f32 dot(const Vec2& a, const Vec2& b)
{
  return a.x * b.x + a.y * b.y;
}

inline f32 cross(const Vec2& a, const Vec2& b)
{
  return a.x * b.y - a.y * b.x;
}

inline bool is_normalized2(f32 length2)
{
  return fabsf(length2 - 1.0f) < 2.0f * FLT_EPSILON;
}

inline bool is_normalized(const Vec2& a)
{
  return is_normalized2(length2(a));
}

inline f32 distance(const Vec2& a, const Vec2& b)
{
  return length(a - b);
}

inline f32 distance2(const Vec2& a, const Vec2& b)
{
  return length2(b - a);
}

inline Vec2 lerp(const Vec2& a, const Vec2& b, f32 t)
{
  return vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

inline f32 normalize_rad(f32 x)
{
  return fabsf(x - TWO_PI * floorf(x / TWO_PI));
}

inline f32 normalize_deg(f32 x)
{
  while (x < 0) {
    x += 360;
  }
  if (x >= 360) {
    x = fmodf(x, 360);
  }
  return x;
}

f32 angles_lerp(f32 a, f32 b, f32 t);

f32 hermite_interp(f32 y0, f32 y1, f32 y2, f32 y3, f32 t);
// ...interpolace mezi y1 a y2.
// http://paulbourke.net/miscellaneous/i32erpolation/ (tension == 0,
// bias == 0)

inline f32 sign(f32 a)
{
  return a < 0 ? -1 : 1;
}

inline f32 signz(f32 a)
{
  if (a < 0) return -1;
  if (a > 0) return 1;
  return 0;
}

Vec2 bezier4(const Vec2&, const Vec2&, const Vec2&, const Vec2&, f32);
Vec2 bezier4_tangent(const Vec2&, const Vec2&, const Vec2&, const Vec2&, f32);

Vec2 xunit_rotated(f32);
Vec2 yunit_rotated(f32);
Vec2 rotate(const Vec2&, f32);

template <typename T>
class PoolAllocator
{
  void resize()
  {
    T* m = new T[grow];
    mem.push_back(m);
    u32 plen = pool.size();
    pool.resize(pool.size() + grow);
    for (u32 i = 0; i < grow; i++) {
      pool[plen + i] = m + i;
    }
  }
  u32 grow;
  std::vector<T*> pool;    // pointery do mem (pripravene objekty pro alokaci)
  std::vector<T*> mem;     // bloky o velikosti grow

public:
  PoolAllocator(u32 _grow) : grow(_grow)
  {
    static_assert(std::is_pod<T>::value, "PoolAllocator value must be POD");
  }

  ~PoolAllocator()
  {
    for (const auto& p : mem) {
      delete [] p;
    }
  }

  T* allocate()
  {
    if (pool.empty()) {
      resize();
    }
    T* v = pool.back();
    pool.pop_back();
    return v;
  }

  void clear()
  {
    pool.clear();
    for (auto m : mem) {
      for (u32 i = 0; i < grow; i++) {
        pool.push_back(m + i);
      }
    }
  }

  void free(T* p)
  {
    assert(p);
    pool.push_back(p);
  }
};

unsigned rnd();
f32 rnd01();
