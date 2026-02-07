#pragma once

#include "source_stream.hpp"
#include "token.hpp"

#include <vector>

namespace scr {

class Parser {
private:
	SourceStream<std::vector<Token>> tokens;

public:
	Parser(const std::vector<Token>& source) :
		tokens(source)
	{ }
};

} // namespace scr
