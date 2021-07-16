// -*- c++ -*-
#include "misc.h"
#include <stdio.h>

static u32 rnd_z = 12345;
static u32 rnd_w = 65435;

unsigned rnd()
{
  rnd_z = 36969 * (rnd_z & 65535) + (rnd_z >> 16);
  rnd_w = 18000 * (rnd_w & 65535) + (rnd_w >> 16);
  return (rnd_z << 16) + rnd_w;
}

f32 rnd01()
{
  unsigned val = rnd() & UINT16_MAX;
  return f32(val) * (1.0 / f32(UINT16_MAX));
}

bool fuzzy_equal(f32 a, f32 b)
{
  if (a == b) {
    return true;
  }
  f32 diff = fabsf(a - b);
  if (a == 0 || b == 0 || diff < FLT_EPSILON) {
    return diff < FLT_EPSILON;
  }
  f32 absA = fabsf(a);
  f32 absB = fabsf(b);
  return diff / (absA + absB) < FLT_EPSILON;
}

f32 angles_lerp(f32 a, f32 b, f32 t)
{
  a = normalize_rad(a);
  b = normalize_rad(b);
  f32 delta = b - a;
  if (fabsf(delta) > PI) {
    if (delta > 0) {
      a += TWO_PI;
    } else {
      b += TWO_PI;
    }
  }
  return normalize_rad(lerp(a, b, t));
}

Vec2 bezier4(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, f32 t)
{
  assert(t >= 0);
  assert(t <= 1);
  const f32 t0 = (1 - t) * (1 - t) * (1 - t);
  const f32 t1 = 3 * t * (1 - t) * (1 - t);
  const f32 t2 = 3 * t * t * (1 - t);
  const f32 t3 = t * t * t;
  return p0 * t0 + p1 * t1 + p2 * t2 + p3 * t3;
}

Vec2 bezier4_tangent(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3,
                     f32 t)
{
  assert(t >= 0);
  assert(t <= 1);
  const f32 t0 = -3 * (1 - t) * (1 - t);
  const f32 t1 = 3 * (1 - t) * (1 - t) - 6 * (1 - t) * t;
  const f32 t2 = 6 * (1 - t) * t - 3 * t * t;
  const f32 t3 = 3 * t * t;
  return normalized(p0 * t0 + p1 * t1 + p2 * t2 + p3 * t3);
}

Vec2 rotate(const Vec2& v, f32 a)
{
  const f32 s = sinf(a);
  const f32 c = cosf(a);
  return vec2(v.x * c - v.y * s, v.y * c + v.x * s);
}

Vec2 xunit_rotated(f32 a)
{
  return vec2(cosf(a), sinf(a));
}

Vec2 yunit_rotated(f32 a)
{
  return vec2(-sinf(a), cosf(a));
}

f32 hermite_interp(f32 y0, f32 y1, f32 y2, f32 y3, f32 mu)
{
  f32 d0 = (y1 - y0) * 0.5f;
  f32 d1 = (y2 - y1) * 0.5f;
  f32 d2 = (y3 - y2) * 0.5f;
  f32 mu2 = mu * mu;
  f32 mu3 = mu2 * mu;
  f32 m0 = d0 + d1;
  f32 m1 = d1 + d2;
  f32 a0 = 2 * mu3 - 3 * mu2 + 1;
  f32 a1 = mu3 - 2 * mu2 + mu;
  f32 a2 = mu3 - mu2;
  f32 a3 = -2 * mu3 + 3 *mu2;
  return a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2;
}
