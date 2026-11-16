#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <stddef.h>

typedef void*(xallocfn)(void*, size_t, size_t); 
typedef void(xallocfn_free)(void*);

typedef struct {
	bool xalloc;
} xalloc_info_single_t;

typedef struct {
	bool xalloc;

	xallocfn* fn;
	xallocfn_free* fn_free;
	void* allocator;
} xalloc_info_t;

void* xallocfn_static_buf(void* buf, size_t size, size_t align);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
