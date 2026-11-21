#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/allocator/xalloc.h>

#include <stddef.h>

#define FSTACK_NEW(capacity_, type_) istack_new(capacity_, sizeof(type_))
#define FSTACK_NEW_XALLOC(capacity_, type_, fn_, allocator_) \
	istack_new_with_xalloc(capacity_, sizeof(type_), fn_, allocator_)

#define FSTACK_POP(istack_, type_) ((type_)*)istack_pop(istack_)
#define FSTACK_BACK(istack_, type_) ((type_)*)istack_back(istack_)

typedef struct {
	xalloc_info_single_t xalloc;

	size_t capacity;
	size_t stk_size;
	size_t elm_size;

	char* buf;
} istack_t;

/*
* elm_size: Must be naturally aligned for safe indexing. 
* Use sizeof(T). Do not use with tightly packed structs (e.g., [[gnu::packed]]).
*/
istack_t istack_new(size_t capacity, size_t elm_size);
istack_t istack_new_with_xalloc(
	size_t capacity, size_t elm_size, xallocfn fn, void* allocator);

bool istack_is_valid(const istack_t* istack);
bool istack_is_full(const istack_t* istack);

void istack_push(istack_t* istack, const char* data);
void* istack_pop(istack_t* istack);
void istack_pop_into(istack_t* istack, char* buf);
void istack_pop_rm(istack_t *istack);
void* istack_back(istack_t* istack);

void* istack_get(istack_t* istack, size_t n);
void istack_set(istack_t* istack, const char* data, size_t n);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
