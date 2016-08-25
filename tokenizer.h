// tokenizer.h

#ifndef __TOKENIZER_H_
#define __TOKENIZER_H_

#include "alpha.h"
#include <vector>
#include <string>

enum TokenValue {
	None,

	TOKEN_SINGLE_LINE_COMMENT,
	TOKEN_EOL,
	TOKEN_LPAREN,
	TOKEN_RPAREN,

	TOKEN_WORD_DEFINITION_BEGIN,
	TOKEN_WORD_DEFINITION_END,
	TOKEN_RET,

	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_FI,

	TOKEN_DO,
	TOKEN_OD,

	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_TIMES,
	TOKEN_OVER,

	TOKEN_PRINT_TOP_CHAR,

	TOKEN_STACK_SIZE,
	TOKEN_MOVE_TO_RETURN_STACK,
	TOKEN_MOVE_FROM_RETURN_STACK,


	// Variables
	TOKEN_VARIABLE,
	TOKEN_ARRAY,
	TOKEN_GET,
	TOKEN_SET,
	TOKEN_SQUARE_BRACKETS,

	// comparison
	TOKEN_LESS_THAN,

	Max
};

enum TokenType {
	TT_IGNORED,
	TT_PREDEFINED_WORD,
	TT_IDENTIFIER,
	TT_INT_LITERAL,
	TT_STRING_LITERAL,
	TT_CHAR_LITERAL
};

enum SemanticType {
	ST_NONE,
	ST_WORD_DEFINITION,
	ST_WORD_INVOCATION,
	ST_VARIABLE_DECLARATION,
	ST_VARIABLE_REFERENCE
};

struct Token {
	TokenType type;
	CELL value;

	CELL match1; // After ELSE, OD
	CELL match2; // After FI

	SemanticType semantic;

	std::string text;
	

	Token(TokenType type, CELL value) {
		this->type = type;
		this->value = value;

		match1 = match2 = 0;
		semantic = ST_NONE;
	}

	int getArraySize() const {
		// use match2 for this
		return match2;
	}

	void setArraySize(int n) {
		match2 = n;
	}
};

class Tokenizer {
public:
	Tokenizer();
	virtual ~Tokenizer();

	std::vector<Token> tokenize(const std::string &source);

private:
	enum State {
		Normal,
		SingleLineComment,
		SandwitchComment
	};

	State state;
	std::vector<std::string> userWords;

	Token getToken(const std::string &s);

	bool isSpace(char c) const;
	bool isNewLine(char c) const;
	bool isDigit(char c) const;

	void handleToken(const Token &t, std::vector<Token> &ret);
	int getUserWord(const std::string &s);
	
	char escapeChar(char c);
	bool escapeString(std::string &s);

};

#endif // ndef __TOKENIZER_H_
