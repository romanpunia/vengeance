#ifndef VI_VECTORCLASS_H
#define VI_VECTORCLASS_H
#ifdef VI_VECTORCLASS
#include <instrset.h>
#if INSTRSET >= 2
#include <vectorclass.h>
#include <vectormath_trig.h>
#define LOAD_V2(Name, Ctx) Vec4f (Name)(Ctx.x, Ctx.y, 0.0f, 0.0f)
#define LOAD_AV2(Name, x, y) Vec4f (Name)(x, y, 0.0f, 0.0f)
#define LOAD_FV2(Name) Vec4f (Name)(x, y, 0.0f, 0.0f)
#define LOAD_V3(Name, Ctx) Vec4f (Name)(Ctx.x, Ctx.y, Ctx.z, 0.0f)
#define LOAD_AV3(Name, x, y, z) Vec4f (Name)(x, y, z, 0.0f)
#define LOAD_FV3(Name) Vec4f (Name)(x, y, z, 0.0f)
#define LOAD_V4(Name, Ctx) Vec4f (Name);(Name).load((float*)&Ctx)
#define LOAD_AV4(Name, x, y, z, w) Vec4f (Name)(x, y, z, w)
#define LOAD_FV4(Name) Vec4f (Name);(Name).load((float*)this)
#define LOAD_V16(Name, Ctx) Vec16f (Name);(Name).load(Ctx.row)
#define LOAD_AV16(Name, A) Vec16f (Name);(Name).load(A)
#define LOAD_FV16(Name) Vec16f (Name);(Name).load(row)
#define LOAD_VAR(Name, A) Vec4f (Name);(Name).load(A)
#define LOAD_VAL(Name, A) Vec4f (Name)(A)
#else
#undef VI_VECTORCLASS
#endif
#endif
#endif