#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "../src/core/c/container/block.h"
#include "../src/core/c/container/vec.h"

// --- Helper for debugging ---
void print_block(const char* label, block_t* b) {
    printf("%s: data='%s', size=%zu\n", label, b->data ? b->data : "NULL", b->size);
}

void test_basic_ops() {
    printf("Running test_basic_ops...\n");

    // Initialize vector for integers (simple type)
    vec_t v;
    vec_init(&v, sizeof(int), NULL, NULL);

    assert(v.size == 0);
    assert(v.esize == sizeof(int));

    // Test Push
    for (int i = 0; i < 10; i++) {
        vec_push(&v, (char*)&i);
    }
    assert(v.size == 10);
    assert(v.capacity >= 10);

    // Test Get
    for (int i = 0; i < 10; i++) {
        int val = *(int*)vec_get(&v, i);
        assert(val == i);
    }

    // Test Pop
    vec_pop(&v);
    assert(v.size == 9);

    // Test Set
    int newVal = 99;
    vec_set(&v, 0, (char*)&newVal);
    assert(*(int*)vec_get(&v, 0) == 99);

    vec_destroy(&v);
    printf("test_basic_ops passed!\n\n");
}

void test_block_integration() {
    printf("Running test_block_integration...\n");

    // Initialize vector for block_t (complex type with callbacks)
    // Using vec_new this time
    vec_t v = vec_new(sizeof(block_t), block_freefn, block_copyfn);

    // 1. Test vec_push_null
    block_t* b1 = (block_t*)vec_push_null(&v);
    block_init(b1, "Hello", 6);

    // 2. Test standard vec_push
    block_t b2 = block_new("World", 6);
    vec_push(&v, (char*)&b2);
    // Note: If vec_push uses copyfn, b2's data is copied. 
    // If it's a simple bitwise copy, we must be careful about ownership.
    // Usually, vec_push should trigger copyfn if provided.

    assert(v.size == 2);

    // Verify content
    block_t* retrieved1 = (block_t*)vec_get(&v, 0);
    assert(strcmp(retrieved1->data, "Hello") == 0);

    block_t* retrieved2 = (block_t*)vec_get(&v, 1);
    assert(strcmp(retrieved2->data, "World") == 0);

    // 3. Test vec_copy (Deep Copy)
    vec_t v_copy;
    vec_copy(&v_copy, &v);

    assert(v_copy.size == v.size);
    block_t* copy_b1 = (block_t*)vec_get(&v_copy, 0);
    
    // Ensure it's a deep copy (different memory addresses for string data)
    assert(copy_b1->data != retrieved1->data); 
    assert(strcmp(copy_b1->data, "Hello") == 0);

    // 4. Test vec_get_copy
    block_t stack_block;
    // This should use block_copyfn to fill stack_block
    vec_get_copy(&v, (char*)&stack_block, 1);
    assert(strcmp(stack_block.data, "World") == 0);

    // Cleanup
    vec_destroy(&v);
    vec_destroy(&v_copy);
    block_free(&stack_block); // Clean up the manual copy

    printf("test_block_integration passed!\n\n");
}

int main() {
    test_basic_ops();
    test_block_integration();

    printf("All tests passed successfully!\n");
    return 0;
}
