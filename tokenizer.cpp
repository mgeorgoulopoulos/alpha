// tokenizer.cpp

#include "tokenizer.h"

Tokenizer::Tokenizer() {
	state = Normal;
}

Tokenizer::~Tokenizer() {
	
}

std::vector<Token> Tokenizer::tokenize(const std::string &source) {
	std::vector<Token> ret;

	std::string currentToken = "";

	for (size_t i = 0; i < source.size(); i++) {
		char c = source.c_str()[i];

		// handle chars and strings
		if (c == '\"' || c == '\'') {
			// fill the token until end of string
			currentToken = "";
			i++;
			while (source[i] != c) {
				currentToken += source[i];
				i++;
				if (i >= source.size()) {
					printf("E: Unmatched %c literal\n", c);
					return ret;
				}
			}

			Token st(TT_CHAR_LITERAL, 0);
			if (c == '\"') st.type = TT_STRING_LITERAL;
			st.text = currentToken;
			if (!escapeString(st.text)) {
				return ret;
			}
			handleToken(st, ret);
			currentToken = "";

			i++;
			c = source[i]; // continue from here
		}

		if (isSpace(c) || isNewLine(c)) {
			Token t = getToken(currentToken);
			t.text = currentToken;
			handleToken(t, ret);
			currentToken = "";

			if (isNewLine(c)) {
				handleToken(Token(TT_PREDEFINED_WORD, TOKEN_EOL), ret);
			}

		} else {
			currentToken += c;
		}		
	}

	// if there are no spaces/newlines in end of file
	Token t = getToken(currentToken);
	t.text = currentToken;
	handleToken(t, ret);
	currentToken = "";

	return ret;
	
}

Token Tokenizer::getToken(const std::string &s) {
	if (s == "") return Token(TT_IGNORED, 0);

	// handle comments first
	if (s == "//")	return Token(TT_PREDEFINED_WORD,TOKEN_SINGLE_LINE_COMMENT);
	if (s == "(")	return Token(TT_PREDEFINED_WORD,TOKEN_LPAREN);
	if (s == ")")	return Token(TT_PREDEFINED_WORD,TOKEN_RPAREN);

	// if we are not in normal mode we can safely discard other tokens
	if (state != Normal) return Token(TT_IGNORED, 0);

	// Predefined words
	if (s == ":")	return Token(TT_PREDEFINED_WORD,TOKEN_WORD_DEFINITION_BEGIN);
	if (s == ";")	return Token(TT_PREDEFINED_WORD,TOKEN_WORD_DEFINITION_END);
	if (s == "RET")	return Token(TT_PREDEFINED_WORD, TOKEN_RET);

	if (s == "IF")	return Token(TT_PREDEFINED_WORD, TOKEN_IF);
	if (s == "ELSE") {
		return Token(TT_PREDEFINED_WORD, TOKEN_ELSE);
	}
	if (s == "FI")	return Token(TT_PREDEFINED_WORD, TOKEN_FI);

	if (s == "DO")	return Token(TT_PREDEFINED_WORD, TOKEN_DO);
	if (s == "OD")	return Token(TT_PREDEFINED_WORD, TOKEN_OD);

	if (s == "+")	return Token(TT_PREDEFINED_WORD, TOKEN_PLUS);
	if (s == "-")	return Token(TT_PREDEFINED_WORD, TOKEN_MINUS);
	if (s == "*")	return Token(TT_PREDEFINED_WORD, TOKEN_TIMES);
	if (s == "/")	return Token(TT_PREDEFINED_WORD, TOKEN_OVER);

	if (s == "..")	return Token(TT_PREDEFINED_WORD, TOKEN_PRINT_TOP_CHAR);

	if (s == "#")	return Token(TT_PREDEFINED_WORD, TOKEN_STACK_SIZE);
	if (s == ">R")	return Token(TT_PREDEFINED_WORD, TOKEN_MOVE_TO_RETURN_STACK);
	if (s == "<R")	return Token(TT_PREDEFINED_WORD, TOKEN_MOVE_FROM_RETURN_STACK);

	if (s == "VARIABLE")return Token(TT_PREDEFINED_WORD, TOKEN_VARIABLE);
	if (s == "ARRAY")return Token(TT_PREDEFINED_WORD, TOKEN_ARRAY);
	if (s == "GET")	return Token(TT_PREDEFINED_WORD, TOKEN_GET);
	if (s == "SET")	return Token(TT_PREDEFINED_WORD, TOKEN_SET);
	if (s == "[]")	return Token(TT_PREDEFINED_WORD, TOKEN_SQUARE_BRACKETS);

	// comparisons
	if (s == "<")	return Token(TT_PREDEFINED_WORD, TOKEN_LESS_THAN);

	// now check for literals
	if (s[0] == '-' || isDigit(s[0])) {
		CELL literal = atoi(s.c_str());
		return Token(TT_INT_LITERAL, literal);
	}

	// unknown - this must be a user word
	return Token(TT_IDENTIFIER, getUserWord(s));
}

bool Tokenizer::isSpace(char c) const {
	return c == ' ' || c == '\t';
}

bool Tokenizer::isNewLine(char c) const {
	return c == 0x0A || c == 0x0D;
}

bool Tokenizer::isDigit(char c) const {
	return c >= '0' && c <= '9';
}

void Tokenizer::handleToken(const Token &t, std::vector<Token> &ret) {
	switch (state) {
	case Normal:
		if (t.type == TT_PREDEFINED_WORD) {
			switch (t.value) {
			case TOKEN_SINGLE_LINE_COMMENT:
				state = SingleLineComment;
				break;
			case TOKEN_LPAREN:
				state = SandwitchComment;
				break;
			case TOKEN_EOL:
				// ignore
				break;
			default:
				ret.push_back(t);
				break;
			}
		}
		else if (t.type != TT_IGNORED) {
			ret.push_back(t);
		}
		break;
	case SingleLineComment:
		if (t.value == TOKEN_EOL) {
			state = Normal;
		}
		break;
	case SandwitchComment:
		if (t.value == TOKEN_RPAREN) {
			state = Normal;
		}
		break;
	}
}

int Tokenizer::getUserWord(const std::string &s) {
	for (size_t i = 0; i < userWords.size(); i++) {
		if (s == userWords[i]) return (int)i;
	}

	userWords.push_back(s);
	return (int)userWords.size() - 1;
}

char Tokenizer::escapeChar(char c) {
	switch (c) {
	case 'a': return '\a';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	case 'v': return '\v';
	case '\\': return '\\';
	case '\'': return '\'';
	case '\"': return '\"';
	case '?': return '?';
	}

	return '?';
}

bool Tokenizer::escapeString(std::string &s) {
	std::string ret = "";

	for (size_t i = 0; i < s.size(); i++) {
		char c = s[i];
		if (c == '\\') {
			if (i >= s.size() - 1) {
				printf("E: invalid escape sequence in literal: %s\n", s.c_str());
				return false;
			}
			i++;
			ret += escapeChar(s[i]);
		} else {
			ret += s[i];
		}
	}

	s = ret;
	return true;
}