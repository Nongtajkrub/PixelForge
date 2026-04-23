#include "stack.h"
#include "block.h"

#include <stddef.h>

stack_t stack_new(size_t esize) {
	return (stack_t) {
		.data = vec_new(esize)
	};
}

block_t stack_pop(stack_t* stack) {
	block_t buff = vec_get_copy(&stack->data, stack->data.size - 1);
	vec_pop(&stack->data);
	return buff;
}
