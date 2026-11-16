#include <core-c/allocator/bump_arena.h>

#include <core-c//utilities/primitive_view.h>
#include <core-c/allocator/xalloc.h>
#include <core-c/io/log.h>

#include <stdalign.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// prevent wrong pointer arithmetic and read operations.
typedef size_t arena_meta_t;
static size_t (*read_arena_meta)(const size_t_v) = read_size_t_v;

static bump_arena_t bump_arena_invalid() {
	return (bump_arena_t) {
		.xalloc = {
			.xalloc = false
		},
		.data = NULL,
		.capacity = 0,
		.offset = 0,
	};
}

bump_arena_t bump_arena_new(size_t capacity) {
	char* data = (char*)malloc(capacity);

	if (!data) return bump_arena_invalid();

	return (bump_arena_t) {
		.xalloc = {
			.xalloc = false
		},
		.data = data,
		.capacity = capacity,
		.offset = 0,
	};
}

bump_arena_t bump_arena_new_with_xalloc(
	xallocfn fn, void* allocator, size_t capacity) {
	char* data = (char*)fn(allocator, capacity, alignof(char*));

	if (!data) return bump_arena_invalid();

	return (bump_arena_t) {
		.xalloc = {
			.xalloc = true
		},
		.data = data,
		.capacity = capacity,
		.offset = 0,
	};
}

bool bump_arena_is_valid(const bump_arena_t* arena) {
	if (arena == NULL) return false; 
	return arena->data;
}

void* bump_arena_alloc(bump_arena_t* arena, size_t size, size_t align) {
	assert(align <= alignof(max_align_t));
	assert(align != 0);
	assert(((align & (align - 1)) == 0));

	const size_t unaligned_offset = arena->offset;
	arena->offset = (arena->offset + align - 1) & ~(align - 1);

	if (arena->offset + (size + sizeof(arena_meta_t)) > arena->capacity) {
		LOG_WARN("Out of memory");
		arena->offset = unaligned_offset;
		return NULL;
	}

	void* ptr = arena->data + arena->offset;
	
	// prepare memory and write metadata.
	memset(ptr, 0, size);

	arena->offset += size + sizeof(arena_meta_t);
	const arena_meta_t offset_diff = arena->offset - unaligned_offset;
	memcpy((char*)ptr + size, &offset_diff, sizeof(arena_meta_t));

	return ptr;
}

void* bump_arena_xallocfn(void* arena, size_t size, size_t align) {
	return bump_arena_alloc((bump_arena_t*)arena, size, align);
}

void bump_arena_xallocfn_free(void* _arena) {
	static_assert(
		true,
		"Allocation made by the bump arena allocator can't be free individualy. "\
		"Yout must free the allocator owned memory directly.");
}

void bump_arena_free(bump_arena_t* arena) {
	assert(!arena->xalloc.xalloc);

	free(arena->data);
	arena->data = NULL;
	arena->capacity = 0;
	arena->offset = 0;
}

void bump_arena_free_back(bump_arena_t* arena) {
	if (arena->offset < sizeof(arena_meta_t)) return;

	arena->offset -=
		read_arena_meta(arena->data + (arena->offset - sizeof(arena_meta_t)));
}
