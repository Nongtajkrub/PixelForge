#include <iostream>

#include "script/lexer.hpp"
#include "script/parser.hpp"
#include "util/bump_arena.hpp"
#include "io/file.hpp"

int main() {
	const auto code = io::file::load_str("script.gby"); 

	if (!code.has_value()) {
		std::cout << "No code file.\n";
		return 1;
	}

	auto lexer = scr::Lexer(code.value());

	for (const auto token : lexer.get_token()) {
		if (token.lexeme.has_value()) {
			std::cout << token.type_as_str() << ", " << token.lexeme.value() << '\n'; 
		} else {
			std::cout << token.type_as_str() << '\n'; 
		}
	}

	auto arena = BumpArena(3200); 

	auto parser = scr::Parser(lexer.get_token(), arena);
		
	if (!parser.parse()) {
		return -1;
	}

	for (const auto node : parser.get_ast()) {
		std::cout << static_cast<int>(*node.adr) << '\n';
	}
}
