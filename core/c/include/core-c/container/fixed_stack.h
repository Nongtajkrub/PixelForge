#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/allocator/xalloc.h>

#include <stddef.h>

#define FSTACK_NEW(capacity_, type_) fstack_new(capacity_, sizeof(type_))
#define FSTACK_NEW_XALLOC(capacity_, type_, fn_, allocator_) \
	fstack_new_with_xalloc(capacity_, sizeof(type_), fn_, allocator_)

typedef struct {
	xalloc_info_single_t xalloc;

	size_t capacity;
	size_t stk_size;
	size_t elm_size;

	char* buf;
} fixed_stack_t;

/*
* elm_size: Must be naturally aligned for safe indexing. 
* Use sizeof(T). Do not use with tightly packed structs (e.g., [[gnu::packed]]).
*/
fixed_stack_t fstack_new(size_t capacity, size_t elm_size);
fixed_stack_t fstack_new_with_xalloc(
	size_t capacity, size_t elm_size, xallocfn fn, void* allocator);

bool fstack_is_valid(const fixed_stack_t* fstack);
bool fstack_is_full(const fixed_stack_t* fstack);

void fstack_push(fixed_stack_t* fstack, char* data);
char* fstack_pop(fixed_stack_t* fstack);
void fstack_pop_into(fixed_stack_t* fstack, char* buf);
void fstack_pop_rm(fixed_stack_t *fstack);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
