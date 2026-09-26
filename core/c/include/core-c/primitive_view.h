#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "types.h"

#include <stddef.h>

typedef char* u64_v;
typedef char* u32_v;
typedef char* u16_v;
typedef char* u8_v;

typedef char* i64_v;
typedef char* i32_v;
typedef char* i16_v;
typedef char* i8_v;

typedef char* size_t_v;

u8 read_u8_v(const u8_v v);
u16 read_u16_v(const u16_v v);
u32 read_u32_v(const u32_v v);
u64 read_u64_v(const u64_v v);

i8 read_i8_v(const i8_v v);
i16 read_i16_v(const i16_v v);
i32 read_i32_v(const i32_v v);
i64 read_i64_v(const i64_v v);

size_t read_size_t_v(const size_t_v v);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
