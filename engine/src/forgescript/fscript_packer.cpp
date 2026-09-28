#include <core-cplusplus/io/byte_io.hpp>

#include "fscript_packer.hpp"

#include <vector>

namespace scr {

CodePackage pack_script(
	const std::vector<u8>& cpool, const std::vector<u8>& code) {
	CodePackage buf;
	auto io = core::BytesBufferWriter(buf);

	io.extend(cpool);
	io.extend(code);

	return buf;
}

} // namespace scr
