#include <stdio.h>
#include <assert.h>

#include "../src/core/c/container/vec.h"

// --- Helper Struct for Testing ---
typedef struct {
    int id;
    float score;
} Student;

void test_primitive_types() {
    printf("Testing with integers...");
    vec_t v = vec_new(sizeof(int));
    
    // Test 1: Initial state
    assert(v.size == 0);
    assert(v.esize == sizeof(int));

    // Test 2: Pushing multiple elements
    for (int i = 0; i < 10; i++) {
        vec_push(&v, (char*)&i);
    }
    assert(v.size == 10);

    // Test 3: Correctness of values (Pointer Arithmetic check)
    for (int i = 0; i < 10; i++) {
        int* val = (int*)vec_get(&v, i);
        assert(*val == i);
    }

    // Test 4: Popping
    vec_pop(&v);
    assert(v.size == 9);
    
    printf(" PASSED\n");
}

void test_complex_types() {
    printf("Testing with custom structs...");
    vec_t v = vec_new(sizeof(Student));
    
    Student s1 = {101, 92.5f};
    Student s2 = {202, 88.0f};

    vec_push(&v, (char*)&s1);
    vec_push(&v, (char*)&s2);

    // Verify struct data integrity
    Student* retrieved = (Student*)vec_get(&v, 1);
    assert(retrieved->id == 202);
    assert(retrieved->score == 88.0f);

    printf(" PASSED\n");
}

void test_memory_growth() {
    printf("Testing growth/reallocation...");
    vec_t v = vec_new(sizeof(double));
    
    // If your vec_new starts with a small capacity (e.g. 4), 
    // pushing 100 items will force multiple reallocs.
    for (int i = 0; i < 100; i++) {
        double val = (double)i * 1.5;
        vec_push(&v, (char*)&val);
    }
    
    assert(v.size == 100);
    assert(*(double*)vec_get(&v, 99) == 99 * 1.5);
    
    printf(" PASSED\n");
}

int main() {
    printf("--- Starting vec_t Tests ---\n");
    
    test_primitive_types();
    test_complex_types();
    test_memory_growth();

    printf("\nAll tests completed successfully!\n");
    return 0;
}
