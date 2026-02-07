#include <iostream>

#include "script/lexer.hpp"
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
}
