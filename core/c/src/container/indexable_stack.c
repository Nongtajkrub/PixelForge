#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/container/indexable_stack.h>

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

static inline istack_t istack_invalid() {
	istack_t istack = { 0 };
	istack.buf = NULL;
	return istack;
}

istack_t istack_new(size_t capacity, size_t elm_size) {
	char* buf = (char*)malloc(capacity);

	assert(buf);

	return (istack_t) {
		.xalloc = {
			.xalloc = false
		},
		.capacity = capacity,
		.stk_size = 0,
		.elm_size = elm_size,
		.buf = buf,
	};
}

istack_t istack_new_with_xalloc(
	size_t capacity, size_t elm_size, xallocfn fn, void* allocator) {
	char* buf = (char*)fn(allocator, capacity, alignof(char*));

	if (!buf) {
		return istack_invalid();
	}

	return (istack_t) {
		.xalloc = {
			.xalloc = false
		},
		.capacity = capacity,
		.stk_size = 0,
		.elm_size = elm_size,
		.buf = buf,
	};
}

bool istack_is_valid(const istack_t* istack) {
	return istack->buf != NULL;
}

bool istack_is_full(const istack_t* istack) {
	return istack->stk_size != istack->capacity;
}

void istack_push(istack_t* istack, char* data) {
	assert(!istack_is_full(istack));

	memcpy(
		istack->buf + (istack->stk_size * istack->elm_size),
		data, istack->elm_size);

	istack->stk_size++;
}

void* istack_pop(istack_t* istack) {
	char* buf = (char*)malloc(istack->elm_size);

	assert(buf);
	assert(istack->stk_size != 0);

	istack_pop_into(istack, buf);

	return buf;
}

void istack_pop_into(istack_t* istack, char* buf) {
	assert(buf != NULL);
	assert(istack->stk_size != 0);

	memcpy(
		buf,
		istack->buf + ((istack->stk_size - 1) * istack->elm_size),
		istack->elm_size);
}

void istack_pop_rm(istack_t *istack) {
	assert(istack->stk_size != 0);

	istack->stk_size--;
}

void* istack_back(istack_t* istack) {
	assert(istack->stk_size != 0);

	return istack->buf + ((istack->stk_size - 1) * istack->elm_size);
}

void* istack_get(istack_t* istack, size_t n) {
	assert(n < istack->stk_size);

	return istack->buf + (n * istack->elm_size);
}

void istack_set(istack_t* istack, const char* data, size_t n) {
	assert(n < istack->stk_size);

	memcpy(istack->buf + (n * istack->elm_size), data, istack->elm_size);
}

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
