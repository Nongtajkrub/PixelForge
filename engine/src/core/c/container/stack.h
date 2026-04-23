#pragma once

#include "vec.h"
#include "block.h"

#include <stddef.h>

// Wraper around vec_t
typedef struct {
	vec_t data;
} stack_t;

stack_t stack_new(size_t esize);

void stack_push(stack_t* stack, char* data);
block_t stack_pop(stack_t* stack);
