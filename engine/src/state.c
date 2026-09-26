#include "state.h"

#include "mem_region.h"

engine_state_t engine_init(engine_state_t* engine) {
	return (engine_state_t) {
		.mem_region = memreg_new(),
	};
}
