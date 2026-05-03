#include "fscript_packer.hpp"

#include <vector>

namespace scr {

std::vector<u8> pack(const ConstPool& cpool, const CodeGenerator& code_gen) {
	std::vector<u8> buf;

	cpool.serialize(buf);
	code_gen.serialize(buf);

	return buf;
}

} // namespace scr
