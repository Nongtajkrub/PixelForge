#include <core-c/utilities/bump_arena.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

typedef size_t arena_meta_t; 

/* -------------------------------------------------------------------------- */
/*                                    TESTS                                   */
/* -------------------------------------------------------------------------- */

static void test_arena_creation_and_free(void) {
    printf("Running: test_arena_creation_and_free...\n");
    
    bump_arena_t arena = bump_arena_new(1024);
    
    assert(bump_arena_is_valid(&arena) == true);
    assert(arena.capacity == 1024);
    assert(arena.offset == 0);

    bump_arena_free(&arena);
    
    assert(arena.data == NULL);
    assert(arena.capacity == 0);
    assert(arena.offset == 0);

    printf("  [PASS]\n");
}

static void test_arena_nested_creation_and_free(void) {
	printf("Running: test_arena_nested_creation_and_free...\n");

	bump_arena_t outer = bump_arena_new(1024);
	assert(bump_arena_is_valid(&outer) == true);

	bump_arena_t* nested = bump_arena_new_nested(512, &outer);
	assert(bump_arena_is_valid(nested) == true);

	int* data = BUMP_ARENA_ALLOC(nested, int);
    assert(data != NULL);

	*data = 5;
    assert(*data == 5);

	bump_arena_free(&outer);

    printf("  [PASS]\n");
}

static void test_arena_alloc_basic_and_zeroed(void) {
    printf("Running: test_arena_alloc_basic_and_zeroed...\n");

    bump_arena_t arena = bump_arena_new(512);

    // Allocate 32 bytes with alignment 0 (or valid alignment)
    size_t alloc_size = sizeof(int);
    int* data = (int*)bump_arena_alloc(&arena, alloc_size, alignof(int));

    assert(data != NULL);
    assert(arena.offset > alloc_size); // Offset includes allocation + metadata

    // Verify memory was zero-initialized by memset
    for (size_t i = 0; i < alloc_size / sizeof(int); i++) {
        assert(data[i] == 0);
    }

    bump_arena_free(&arena);
    printf("  [PASS]\n");
}

static void test_arena_alloc_multiple_distinct(void) {
    printf("Running: test_arena_alloc_multiple_distinct...\n");

    bump_arena_t arena = bump_arena_new(1024);

    char* ptr1 = (char*)bump_arena_alloc(&arena, 64, alignof(char));
    char* ptr2 = (char*)bump_arena_alloc(&arena, 64, alignof(char));

    assert(ptr1 != NULL);
    assert(ptr2 != NULL);
    assert(ptr1 != ptr2);

    // Ensure memory buffers don't overlap
    // ptr2 should start after ptr1 + payload + metadata
    assert((uintptr_t)ptr2 >= (uintptr_t)ptr1 + 64 + sizeof(arena_meta_t));

    // Write data to verify non-interference
    memset(ptr1, 'A', 64);
    memset(ptr2, 'B', 64);

    assert(ptr1[0] == 'A');
    assert(ptr2[0] == 'B');

    bump_arena_free(&arena);
    printf("  [PASS]\n");
}

static void test_arena_out_of_memory_and_rollback(void) {
    printf("Running: test_arena_out_of_memory_and_rollback...\n");

    bump_arena_t arena = bump_arena_new(128);

    // First valid allocation
    void* ptr1 = bump_arena_alloc(&arena, 32, alignof(void));
    assert(ptr1 != NULL);
    size_t offset_after_first = arena.offset;

    // Second allocation that exceeds remaining capacity
    void* ptr2 = bump_arena_alloc(&arena, 200, alignof(void));
    assert(ptr2 == NULL);

    // Verify offset rolled back to state prior to failed allocation
    assert(arena.offset == offset_after_first);

    bump_arena_free(&arena);
    printf("  [PASS]\n");
}

static void test_arena_free_back_lifo(void) {
    printf("Running: test_arena_free_back_lifo...\n");

    bump_arena_t arena = bump_arena_new(1024);

    void* ptr1 = bump_arena_alloc(&arena, 64, alignof(void));
    size_t offset1 = arena.offset;

    void* ptr2 = bump_arena_alloc(&arena, 128, alignof(void));
    size_t offset2 = arena.offset;

    assert(ptr1 != NULL && ptr2 != NULL);
    assert(offset2 > offset1);

    // Free top allocation (ptr2)
    bump_arena_free_back(&arena);
    assert(arena.offset == offset1);

    // Free bottom allocation (ptr1)
    bump_arena_free_back(&arena);
    assert(arena.offset == 0);

    bump_arena_free(&arena);
    printf("  [PASS]\n");
}

static void test_arena_free_back_empty_guard(void) {
    printf("Running: test_arena_free_back_empty_guard...\n");

    bump_arena_t arena = bump_arena_new(256);

    // Should gracefully do nothing on offset 0 without underflowing
    bump_arena_free_back(&arena);
    assert(arena.offset == 0);

    bump_arena_free(&arena);
    printf("  [PASS]\n");
}

/* -------------------------------------------------------------------------- */
/*                                MAIN DRIVER                                 */
/* -------------------------------------------------------------------------- */

int main(void) {
    printf("=== Starting Bump Arena Unit Tests ===\n\n");

    test_arena_creation_and_free();
	test_arena_nested_creation_and_free();
    test_arena_alloc_basic_and_zeroed();
    test_arena_alloc_multiple_distinct();
    test_arena_out_of_memory_and_rollback();
    test_arena_free_back_lifo();
    test_arena_free_back_empty_guard();

    printf("\n=== All Tests Passed Successfully! ===\n");
    return 0;
}
