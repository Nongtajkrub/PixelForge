#include "fscript_preprocessor.hpp"

#include "fscript_symbol_table.hpp"
#include "fscript_pattern.hpp"

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
	// Sprite directive was already handle.
	switch (this->tokens.peek().kind) {
	case TokenKind::DIRECT_USE:
		return process_use_direct();
	case TokenKind::DIRECT_SELF:
		process_self_direct();
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

	this->script_sprite = *(this->tokens.peek().lexeme);
	// Add sprite identifier to global scope.
	this->symbols.new_identifier_global(
		this->symbols.intern(*(this->tokens.skip().lexeme)),
		IdenAttr(this->symbols.types.ty_sprite, VarAttr()));

	// Skip semicolon.
	this->tokens.skip();

	return true;
}

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
		this->symbols.intern(*(this->tokens.skip().lexeme)),
		IdenAttr(this->symbols.types.ty_sprite, VarAttr()));

	// Skip semicolon.
	this->tokens.skip();

	return true;
}

void Preprocessor::process_self_direct() {
	this->tokens.replace_and_insert({
		Token(
			TokenKind::IDENTIFIER,
			this->script_sprite, this->tokens.peek().location)
	});
}

} // namespace scr
