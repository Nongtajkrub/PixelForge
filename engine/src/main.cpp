#include <iostream>

#include "script/lexer.hpp"
#include "script/ast.hpp"
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

	/*
	for (const auto token : lexer.get_token()) {
		if (token.lexeme.has_value()) {
			std::cout << token.type_as_str() << ", " << token.lexeme.value() << '\n'; 
		} else {
			std::cout << token.type_as_str() << '\n'; 
		}
	}
	*/

	auto arena = BumpArena(3200); 

	auto parser = scr::Parser(lexer.get_token(), arena);
	if (!parser.parse()) {
		return -1;
	}

	auto ast = parser.get_ast(1);

	std::cout << "Node kind: " << static_cast<int>(*ast.adr) << '\n';

	auto statment = reinterpret_cast<const scr::DeclarationStmt*>(ast.adr);

	std::cout << "Identifier: " << statment->name.lexeme.value() << '\n';

	auto expression =
		reinterpret_cast<const scr::BinaryExpr*>(statment->init.value().adr);

	auto left =
		reinterpret_cast<const scr::PrimaryExpr*>(expression->left.adr);

	std::cout << "left: " << left->token.lexeme.value() << '\n';

	std::cout << "First level op: " << expression->op.kind_as_str() << '\n';

	std::cout << "Expression Kind: " << static_cast<int>(*expression->right.adr) << '\n';

	auto right =
		reinterpret_cast<const scr::BinaryExpr*>(expression->right.adr);

	auto inner_left = reinterpret_cast<const scr::PrimaryExpr*>(right->left.adr);
	
	std::cout << "right: left: " << inner_left->token.lexeme.value() << '\n';

	std::cout << "Second level op: " << right->op.kind_as_str() << '\n';

	auto inner_right =
		reinterpret_cast<const scr::PrimaryExpr*>(right->right.adr);

	std::cout << "right: right: " << inner_right->token.lexeme.value() << '\n';

	/*
	auto inner_expression = 
		reinterpret_cast<const scr::BinaryExpr*>(expression->right.adr);

	std::cout << "Inner lvel op: " << inner_expression->op.kind_as_str() << '\n';
	*/
}
