#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/container/fixed_stack.h>

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

static inline fixed_stack_t fstack_invalid() {
	fixed_stack_t fstack = { 0 };
	fstack.buf = NULL;
	return fstack;
}

fixed_stack_t fstack_new(size_t capacity, size_t elm_size) {
	char* buf = (char*)malloc(capacity);

	assert(buf);

	return (fixed_stack_t) {
		.xalloc = {
			.xalloc = false
		},
		.capacity = capacity,
		.stk_size = 0,
		.elm_size = elm_size,
		.buf = buf,
	};
}

fixed_stack_t fstack_new_with_xalloc(
	size_t capacity, size_t elm_size, xallocfn fn, void* allocator) {
	char* buf = (char*)fn(allocator, capacity, alignof(char*));

	if (!buf) {
		return fstack_invalid();
	}

	return (fixed_stack_t) {
		.xalloc = {
			.xalloc = false
		},
		.capacity = capacity,
		.stk_size = 0,
		.elm_size = elm_size,
		.buf = buf,
	};
}

bool fstack_is_valid(const fixed_stack_t* fstack) {
	return fstack->buf != NULL;
}

bool fstack_is_full(const fixed_stack_t* fstack) {
	return fstack->stk_size != fstack->capacity;
}

void fstack_push(fixed_stack_t* fstack, char* data) {
	assert(!fstack_is_full(fstack));

	memcpy(
		fstack->buf + (fstack->stk_size * fstack->elm_size),
		data, fstack->elm_size);

	fstack->stk_size++;
}

char* fstack_pop(fixed_stack_t* fstack) {
	char* buf = (char*)malloc(fstack->elm_size);

	assert(buf);

	fstack_pop_into(fstack, buf);

	return buf;
}

void fstack_pop_into(fixed_stack_t* fstack, char* buf) {
	assert(buf != NULL);

	memcpy(
		buf,
		fstack->buf + ((fstack->stk_size - 1) * fstack->elm_size),
		fstack->elm_size);
}

void fstack_pop_rm(fixed_stack_t *fstack) {
	fstack->stk_size--;
}

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
