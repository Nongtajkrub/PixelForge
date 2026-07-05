#include "fscript_interpreter.h"

#include "../../core/c/io/vec_stream.h"
#include "../../core/c/io/log.h"
#include "fscript_instruction.h"

#include <stdlib.h>
#include <stdio.h>

void fscript_interpret(fscript_pkg_t* pkg) {
	vec_stream_t code = vstream_make(&pkg->code.main.data);

	while (!vstream_end(&code)) {
		const opcode_t op = *(opcode_t*)vstream_advance(&code);

		printf("%zu: Opcode: %s\n", vstream_cursor(&code), op_to_str(op));

		if (op_have_operand(op)) {
			if (vstream_end(&code)) {
				LOG_ERR("Invalid code package.");
				exit(1);
			} 

			const word_t operand = *(word_t*)vstream_advance(&code);

			printf("%zu: Operand: %d\n", vstream_cursor(&code), operand);
		}
	}
}
