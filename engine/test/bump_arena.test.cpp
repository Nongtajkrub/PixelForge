#include "../src/core/cplusplus/container/bump_arena.hpp"

#include <cassert>
#include <iostream>
#include <cstdint>

using namespace core;

// Test types with different sizes and alignments
struct SmallType {
    char value;
};

struct MediumType {
    int a;
    char b;
};

struct LargeType {
    double a;
    int b;
    char c;
};

struct alignas(16) AlignedType {
    char value;
};

struct alignas(32) HighlyAlignedType {
    int value;
};

// Helper function to check if a pointer is aligned
template <typename T>
bool is_aligned(void* ptr) {
    return reinterpret_cast<uintptr_t>(ptr) % alignof(T) == 0;
}

void test_basic_allocation() {
    std::cout << "Testing basic allocation..." << std::endl;
    
    BumpArena arena(1024);
    
    int* p1 = arena.alloc<int>();
    assert(p1 != nullptr);
    assert(is_aligned<int>(p1));
    *p1 = 42;
    assert(*p1 == 42);
    
    double* p2 = arena.alloc<double>();
    assert(p2 != nullptr);
    assert(is_aligned<double>(p2));
    *p2 = 3.14159;
    assert(*p2 == 3.14159);
    
    // Verify first allocation is still valid
    assert(*p1 == 42);
    
    std::cout << "  ✓ Basic allocation works" << std::endl;
}

void test_multiple_allocations() {
    std::cout << "Testing multiple allocations..." << std::endl;
    
    BumpArena arena(1024);
    
    constexpr int COUNT = 10;
    int* ptrs[COUNT];
    
    // Allocate multiple integers
    for (int i = 0; i < COUNT; i++) {
        ptrs[i] = arena.alloc<int>();
        assert(ptrs[i] != nullptr);
        assert(is_aligned<int>(ptrs[i]));
        *ptrs[i] = i * 100;
    }
    
    // Verify all allocations are valid and distinct
    for (int i = 0; i < COUNT; i++) {
        assert(*ptrs[i] == i * 100);
        for (int j = i + 1; j < COUNT; j++) {
            assert(ptrs[i] != ptrs[j]);
        }
    }
    
    std::cout << "  ✓ Multiple allocations work" << std::endl;
}

void test_alignment() {
    std::cout << "Testing alignment..." << std::endl;
    
    BumpArena arena(1024);
    
    // Allocate types with different alignment requirements
    char* p1 = arena.alloc<char>();
    assert(is_aligned<char>(p1));
    
    int* p2 = arena.alloc<int>();
    assert(is_aligned<int>(p2));
    
    double* p3 = arena.alloc<double>();
    assert(is_aligned<double>(p3));
    
    AlignedType* p4 = arena.alloc<AlignedType>();
    assert(is_aligned<AlignedType>(p4));
    assert(reinterpret_cast<uintptr_t>(p4) % 16 == 0);
    
    HighlyAlignedType* p5 = arena.alloc<HighlyAlignedType>();
    assert(is_aligned<HighlyAlignedType>(p5));
    assert(reinterpret_cast<uintptr_t>(p5) % 32 == 0);
    
    std::cout << "  ✓ Alignment requirements met" << std::endl;
}

void test_mixed_type_allocations() {
    std::cout << "Testing mixed type allocations..." << std::endl;
    
    BumpArena arena(2048);
    
    SmallType* s1 = arena.alloc<SmallType>();
    s1->value = 'A';
    
    MediumType* m1 = arena.alloc<MediumType>();
    m1->a = 100;
    m1->b = 'B';
    
    LargeType* l1 = arena.alloc<LargeType>();
    l1->a = 1.5;
    l1->b = 200;
    l1->c = 'C';
    
    SmallType* s2 = arena.alloc<SmallType>();
    s2->value = 'D';
    
    // Verify all values are preserved
    assert(s1->value == 'A');
    assert(m1->a == 100);
    assert(m1->b == 'B');
    assert(l1->a == 1.5);
    assert(l1->b == 200);
    assert(l1->c == 'C');
    assert(s2->value == 'D');
    
    // Verify alignment
    assert(is_aligned<SmallType>(s1));
    assert(is_aligned<MediumType>(m1));
    assert(is_aligned<LargeType>(l1));
    assert(is_aligned<SmallType>(s2));
    
    std::cout << "  ✓ Mixed type allocations work" << std::endl;
}

void test_sequential_addresses() {
    std::cout << "Testing sequential address allocation..." << std::endl;
    
    BumpArena arena(1024);
    
    char* p1 = arena.alloc<char>();
    char* p2 = arena.alloc<char>();
    char* p3 = arena.alloc<char>();
    
    // Characters should be allocated sequentially (no alignment padding needed)
    assert(p2 == p1 + 1);
    assert(p3 == p2 + 1);
    
    std::cout << "  ✓ Sequential allocation verified" << std::endl;
}

void test_alignment_padding() {
    std::cout << "Testing alignment padding..." << std::endl;
    
    BumpArena arena(1024);
    
    // Allocate a char, then an int - should see padding
    char* p1 = arena.alloc<char>();
    int* p2 = arena.alloc<int>();
    
    // The int should be aligned, which may require padding after the char
    assert(is_aligned<int>(p2));
    
    // The distance should be at least 1 byte (char) and p2 must be aligned
    ptrdiff_t distance = reinterpret_cast<char*>(p2) - p1;
    assert(distance >= 1);
    assert(distance <= alignof(int)); // At most alignof(int) bytes
    
    std::cout << "  ✓ Alignment padding works correctly" << std::endl;
}

void test_struct_allocation() {
    std::cout << "Testing struct allocation..." << std::endl;
    
    struct TestStruct {
        int id;
        double value;
        char flag;
    };
    
    BumpArena arena(1024);
    
    TestStruct* s1 = arena.alloc<TestStruct>();
    assert(s1 != nullptr);
    assert(is_aligned<TestStruct>(s1));
    
    s1->id = 123;
    s1->value = 45.67;
    s1->flag = 'X';
    
    TestStruct* s2 = arena.alloc<TestStruct>();
    s2->id = 456;
    s2->value = 89.01;
    s2->flag = 'Y';
    
    // Verify independence
    assert(s1->id == 123);
    assert(s1->value == 45.67);
    assert(s1->flag == 'X');
    assert(s2->id == 456);
    assert(s2->value == 89.01);
    assert(s2->flag == 'Y');
    
    std::cout << "  ✓ Struct allocation works" << std::endl;
}

void test_large_allocation() {
    std::cout << "Testing large allocation..." << std::endl;
    
    constexpr size_t ARRAY_SIZE = 100;
    BumpArena arena(sizeof(int) * ARRAY_SIZE + 64); // Extra space for alignment
    
    // Allocate space for many integers
    int* array[ARRAY_SIZE];
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array[i] = arena.alloc<int>();
        *array[i] = static_cast<int>(i);
    }
    
    // Verify all values
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        assert(*array[i] == static_cast<int>(i));
    }
    
    std::cout << "  ✓ Large allocation works" << std::endl;
}

void test_zero_sized_arena() {
    std::cout << "Testing zero-sized arena..." << std::endl;
    
    BumpArena arena(0);
    
    // This should still work (malloc(0) is implementation-defined but typically valid)
    // However, we might not be able to allocate anything useful
    
    std::cout << "  ✓ Zero-sized arena doesn't crash" << std::endl;
}

void test_arena_destruction() {
    std::cout << "Testing arena destruction..." << std::endl;
    
    {
        BumpArena arena(1024);
        int* p = arena.alloc<int>();
        *p = 42;
        // Arena destructor should be called here
    }
    
    std::cout << "  ✓ Arena destruction works" << std::endl;
}

int main() {
    std::cout << "Running BumpArena tests...\n" << std::endl;
    
    try {
        test_basic_allocation();
        test_multiple_allocations();
        test_alignment();
        test_mixed_type_allocations();
        test_sequential_addresses();
        test_alignment_padding();
        test_struct_allocation();
        test_large_allocation();
        test_zero_sized_arena();
        test_arena_destruction();
        
        std::cout << "\n✅ All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
