/*
* Written by Google Gemini.
*/

#include <core-c/primitive_view.h>

#include <core-c/types.h>

#include <stddef.h>

u8 read_u8_v(const u8_v v) {
    return (u8)v[0];
}

u16 read_u16_v(const u16_v v) {
    const u8* p = (const u8*)v;
    return (u16)p[0] |
           ((u16)p[1] << 8);
}

u32 read_u32_v(const u32_v v) {
    const u8* p = (const u8*)v;
    return (u32)p[0] |
           ((u32)p[1] << 8) |
           ((u32)p[2] << 16) |
           ((u32)p[3] << 24);
}

u64 read_u64_v(const u64_v v) {
    const u8* p = (const u8*)v;
    return (u64)p[0] |
           ((u64)p[1] << 8)  |
           ((u64)p[2] << 16) |
           ((u64)p[3] << 24) |
           ((u64)p[4] << 32) |
           ((u64)p[5] << 40) |
           ((u64)p[6] << 48) |
           ((u64)p[7] << 56);
}

i8 read_i8_v(const i8_v v) {
    return (i8)read_u8_v(v);
}

i16 read_i16_v(const i16_v v) {
    return (i16)read_u16_v(v);
}

i32 read_i32_v(const i32_v v) {
    return (i32)read_u32_v(v);
}

i64 read_i64_v(const i64_v v) {
    return (i64)read_u64_v(v);
}

size_t read_size_t_v(const size_t_v v) {
    if (sizeof(size_t) == 8) {
        return (size_t)read_u64_v(v);
    } else {
        return (size_t)read_u32_v(v);
    }
}
