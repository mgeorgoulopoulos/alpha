// matcher.cpp

#include "matcher.h"
#include <map>

Matcher::Matcher() {

}

Matcher::~Matcher() {

}

bool Matcher::match(std::vector<Token> &bytecode) {
	std::map<std::string, int> variables;
	std::map<std::string, int> userWords;

	for (size_t i = 0; i < bytecode.size(); i++) {
		Token *t = &bytecode[i];
		
		if (t->type == TT_IDENTIFIER) {
			if (i == 0) {
				printf("E: Identifier %s found as first token of program\n", t->text.c_str());
				return false;
			}

			if (bytecode[i - 1].type == TT_PREDEFINED_WORD && bytecode[i - 1].value == TOKEN_WORD_DEFINITION_BEGIN) {
				t->semantic = ST_WORD_DEFINITION;
				t->value = userWords.size();
				userWords[t->text] = userWords.size();
			} else if (bytecode[i - 1].type == TT_PREDEFINED_WORD && bytecode[i - 1].value == TOKEN_VARIABLE) {
				t->semantic = ST_VARIABLE_DECLARATION;
				t->setArraySize(1);
				t->value = variables.size();
				variables[t->text] = variables.size();
			} else if (bytecode[i - 1].type == TT_PREDEFINED_WORD && bytecode[i - 1].value == TOKEN_ARRAY) {
				if (i < 2) {
					printf("E: Array declaration missing length\n");
					return false;
				}
				if (bytecode[i - 2].type != TT_INT_LITERAL) {
					printf("E: Array declaration missing length\n");
					return false;
				}
				if (bytecode[i - 2].value <= 0) {
					printf("E: Nonpositive array length\n");
					return false;
				}

				// array size must not count as literal
				bytecode[i - 2].type = TT_IGNORED;

				t->semantic = ST_VARIABLE_DECLARATION;
				t->setArraySize(bytecode[i-2].value);
				t->value = variables.size();
				variables[t->text] = variables.size();
			}
			else if (userWords.find(t->text) != userWords.end()) {
				t->semantic = ST_WORD_INVOCATION;
				t->value = userWords[t->text];
			}
			else if (variables.find(t->text) != variables.end()) {
				t->semantic = ST_VARIABLE_REFERENCE;
				t->value = variables[t->text];
			} else {
				printf("E: Unknown identifier %s\n", t->text.c_str());
				return false;
			}
		}
		
		if (t->type == TT_PREDEFINED_WORD) {
			switch (t->value) {
			case TOKEN_WORD_DEFINITION_BEGIN:
				if (bytecode[i + 1].type != TT_IDENTIFIER) {
					printf("E: user word expected after ':'\n");
					return false;
				}
				t->match1 = i + 1;
				
				// we match the end of word definition with the word identifier that
				// (hopefully) follows
				unmatched.push(&bytecode[i+1]);

				break;
			case TOKEN_WORD_DEFINITION_END:
				if (unmatched.empty()) {
					printf("E: ; : unexpected\n");
					return false;
				}

				if (unmatched.top()->type != TT_IDENTIFIER) {
					printf("E: ; : unexpected\n");
					return false;
				}

				t->match1 = (unmatched.top() - &bytecode[0]);

				unmatched.top()->match1 = i;
				unmatched.pop();
				break;
			case TOKEN_IF:
				unmatched.push(t);
				break;
			case TOKEN_ELSE:
				if (unmatched.empty()) {
					printf("E: ELSE : unexpected\n");
					return false;
				}

				if (unmatched.top()->value != TOKEN_IF) {
					printf("E: ELSE : unexpected\n");
					return false;
				}

				unmatched.top()->match1 = i;
				break;
			case TOKEN_FI:
				if (unmatched.empty()) {
					printf("E: FI : unexpected\n");
					return false;
				}

				if (unmatched.top()->value != TOKEN_IF) {
					printf("E: FI : unexpected\n");
					return false;
				}

				unmatched.top()->match2 = i;

				if (unmatched.top()->match1 == 0) {
					// there was no ELSE
					unmatched.top()->match1 = i;
				}
				else {
					// also match the ELSE with its FI
					bytecode[unmatched.top()->match1].match1 = i;
				}

				unmatched.pop();
				break;
			case TOKEN_DO:
				unmatched.push(t);
				break;
			case TOKEN_OD:
				if (unmatched.empty()) {
					printf("E: OD : unexpected\n");
					return false;
				}

				if (unmatched.top()->value != TOKEN_DO) {
					printf("E: OD : unexpected\n");
					return false;
				}
				unmatched.top()->match1 = i;
				unmatched.pop();
				break;
			}

		} // end if (predefined word)
	}

	return unmatched.empty();
}