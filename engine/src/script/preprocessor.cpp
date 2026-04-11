#include "preprocessor.hpp"
#include "pattern.hpp"
#include "symbol_table.hpp"

namespace scr {

bool Preprocessor::process() {
	// A script have to start with a sprite directive.
	if (!process_sprite_direct()) {
		return false;
	}

	while (!this->tokens.is_eof() && process_direct()) { }

	return true;
}

bool Preprocessor::process_direct() {
	switch (this->tokens.peek().kind) {
	case TokenKind::DIRECT_USE:
		return process_use_direct();
	case TokenKind::DIRECT_UPDATE:
		process_update_direct();
		return true;
	case TokenKind::DIRECT_COLLIDE:
		process_collide_direct();
		return true;
	default:
		this->tokens.advance();
	}

	return true;
}

bool Preprocessor::process_sprite_direct() {
	// Ensure correct pattern before processing.
	if (!Pattern<
			MutTokenStream,
			TokenKind::DIRECT_SPRITE,
			TokenKind::IDENTIFIER,
			TokenKind::SEMICOLON>
				::match_peek(this->tokens, this->err_stream)) {
		return false;
	}

	// Skip sprite direcitive.
	this->tokens.skip();

	// Add sprite identifier to global scope.
	this->symbols.new_identifier_global(
		this->symbols.intern_iden(*(this->tokens.skip().lexeme)),
		IdenAttr(IdenKind::VAR, TokenKind::SPRITE_T));

	// Skip semicolon.
	this->tokens.skip();

	return true;
}

// Similar process sprite directive, only change pattern matching expectations. 
bool Preprocessor::process_use_direct() {
	// Skip use directive.
	this->tokens.skip();

	// Ensure correct pattern before processing.
	if (!Pattern<
			MutTokenStream,
			TokenKind::IDENTIFIER,
			TokenKind::SEMICOLON>
				::match_peek(this->tokens, this->err_stream)) {
		return false;
	}

	// Add sprite identifier to global scope.
	this->symbols.new_identifier_global(
		this->symbols.intern_iden(*(this->tokens.skip().lexeme)),
		IdenAttr(IdenKind::VAR, TokenKind::SPRITE_T));

	// Skip semicolon.
	this->tokens.skip();

	return true;
}

void Preprocessor::process_update_direct() {
	const auto loc = this->tokens.peek().location;
	this->tokens.replace_and_insert({
		Token(TokenKind::ARROW, loc),
		Token(TokenKind::VOID_T, loc),

		Token(TokenKind::LEFT_PAREN, loc),

		Token(TokenKind::IDENTIFIER, "dt", loc),
		Token(TokenKind::COLON, loc),
		Token(TokenKind::FLOAT_T, loc),

		Token(TokenKind::RIGHT_PAREN, loc),
	});
}

void Preprocessor::process_collide_direct() {
	const auto loc = this->tokens.peek().location;
	this->tokens.replace_and_insert({
		Token(TokenKind::ARROW, loc),
		Token(TokenKind::VOID_T, loc),

		Token(TokenKind::LEFT_PAREN, loc),

		Token(TokenKind::IDENTIFIER, "other", loc),
		Token(TokenKind::COLON, loc),
		Token(TokenKind::SPRITE_T, loc),

		Token(TokenKind::RIGHT_PAREN, loc),
	});
}

} // namespace scr
