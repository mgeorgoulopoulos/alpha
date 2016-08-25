// CodeGenx86.cpp

#include "CodeGenx86.h"

CodeGenx86::CodeGenx86() {

}

CodeGenx86::~CodeGenx86() {

}

bool CodeGenx86::generate(const std::vector<Token> &bytecode, std::string &assembly) {

	// Load the header
	assembly = dataSection;

	// add variable declarations to data section
	generateVariableDeclarations(bytecode, assembly);

	// add init code
	assembly += initializationSection;

	// append program source
	for (size_t i = 0; i < bytecode.size(); i++) {
		assembly += removeNewline(bytecode[i].text) + " ";
	}
	assembly += "\n; --------------------------------------------------------------\n";

	// generate instructions
	for (size_t i = 0; i < bytecode.size(); i++) {
		Token t = bytecode[i];

		switch (t.type) {
		case TT_PREDEFINED_WORD:
			switch (t.value) {
			case TOKEN_WORD_DEFINITION_BEGIN:
				assembly += "\t; :\n";
				assembly += "\tJMP WORD_" + std::to_string(bytecode[t.match1].value) + "_END\n";
				break;
			case TOKEN_WORD_DEFINITION_END:
				assembly += "\t; ;\n";
				assembly += "\tRET\n";
				assembly += "WORD_" + std::to_string(bytecode[t.match1].value) + "_END:\n";
				break;
			case TOKEN_RET:
				assembly += "\t; RET\n";
				assembly += "\tRET\n";
				break;
			case TOKEN_IF:
				assembly += "\t; IF\n";
				assembly += "\tSUB EDX, 4\n";
				assembly += "\tMOV EAX, [EDX]\n";
				assembly += "\tTEST EAX, EAX\n";
				assembly += std::string("\tJZ >IF_") + std::to_string(t.match1) + "\n";
				break;
			case TOKEN_ELSE:
				assembly += "\t; ELSE\n";
				assembly += std::string("\tJMP IF_") + std::to_string(t.match1) + "\n";
				assembly += "IF_" + std::to_string(i) + ":\n";
				break;
			case TOKEN_FI:
				assembly += "\t; FI\n";
				assembly += std::string("IF_") + std::to_string(i) + ":\n";
				break;
			case TOKEN_DO:
				assembly += "\t; DO\n";
				assembly += std::string("DO_") + std::to_string(t.match1) + ":\n";
				assembly += "\tMOV EAX, [EDX - 4]\n";
				assembly += "\tTEST EAX, EAX\n";
				assembly += std::string("\tJNZ >DO_") + std::to_string(i) + "\n";
				assembly += "\tSUB EDX, 4\n";
				assembly += "\tTEST EAX, EAX\n";
				assembly += std::string("\tJZ >DO_") + std::to_string(t.match1) + "_END\n";
				assembly += std::string("DO_") + std::to_string(i) + ":\n";
				break;
			case TOKEN_OD:
				assembly += "\t; OD\n";
				assembly += std::string("\tJMP DO_") + std::to_string(i) + "\n";
				assembly += std::string("DO_") + std::to_string(i) + "_END:\n";
				break;
			case TOKEN_PLUS:
				assembly += "\t; +\n";
				assembly += "\tMOV EAX, [EDX - 8]\n";
				assembly += "\tADD EAX, [EDX - 4]\n";
				assembly += "\tMOV [EDX - 8], EAX\n";
				assembly += "\tSUB EDX, 4\n";
				break;
			case TOKEN_MINUS:
				assembly += "\t; -\n";
				assembly += "\tMOV EAX, [EDX - 8]\n";
				assembly += "\tSUB EAX, [EDX - 4]\n";
				assembly += "\tMOV[EDX - 8], EAX\n";
				assembly += "\tSUB EDX, 4\n";
				break;
			case TOKEN_TIMES:
				assembly += "\t; *\n";
				assembly += "\tMOV EAX, [EDX - 8]\n";
				assembly += "\tMOV ECX, D[EDX - 4]\n";
				assembly += "\tPUSH EDX\n";
				assembly += "\tMUL ECX\n";
				assembly += "\tPOP EDX\n";
				assembly += "\tMOV[EDX - 8], EAX\n";
				assembly += "\tSUB EDX, 4\n";
				break;
			case TOKEN_OVER:
				assembly += "\t; / \n";
				assembly += "\tMOV EAX, [EDX - 8]\n";
				assembly += "\tMOV ECX, D[EDX - 4]\n";
				assembly += "\tPUSH EDX\n";
				assembly += "\tMOV EDX, 0\n";
				assembly += "\tDIV ECX\n";
				assembly += "\tPOP EDX\n";
				assembly += "\tMOV[EDX - 8], EAX\n";
				assembly += "\tSUB EDX, 4\n";
				break;
			case TOKEN_PRINT_TOP_CHAR:
				assembly += "\t; ..\n";
				assembly += "\tSUB EDX, 4\n";
				assembly += "\tMOV EAX, [EDX]\n";
				assembly += "\tMOV [TmpDWORD], EAX\n";
				assembly += "\tPUSH EDX\n";
				assembly += "\tPUSH 0, ADDR WRKEEP\n";
				assembly += "\tPUSH 1, ADDR TmpDWORD\n";
				assembly += "\tPUSH [STDOUT]\n";
				assembly += "\tCALL WriteFile\n";
				assembly += "\tPOP EDX\n";
				break;
			case TOKEN_STACK_SIZE:
				assembly += "\t; #\n";
				assembly += "\tMOV EAX, EDX\n";
				assembly += "\tSUB EAX, ADDR DataStack\n";
				assembly += "\tSHR EAX, 2\n";
				assembly += "\tMOV [EDX], EAX\n";
				assembly += "\tADD EDX, 4\n";
				break;
			case TOKEN_MOVE_TO_RETURN_STACK:
				assembly += "\t; >R\n";
				assembly += "\tSUB EDX, 4\n";
				assembly += "\tPUSH [EDX]\n";
				break;
			case TOKEN_MOVE_FROM_RETURN_STACK:
				assembly += "\t; <R\n";
				assembly += "\tPOP [EDX]\n";
				assembly += "\tADD EDX, 4\n";
				break;

			case TOKEN_VARIABLE:
				TOKEN_ARRAY:
				// ignore
				break;

			case TOKEN_GET:
				assembly += "\t; GET\n";
				assembly += "\tMOV EAX, [EDX-4]\n";
				assembly += "\tMOV EAX, [EAX]\n";
				assembly += "\tMOV [EDX-4], EAX\n";
				break;
			case TOKEN_SET:
				assembly += "\t; SET\n";
				assembly += "\tSUB EDX, 8\n";
				assembly += "\tMOV EAX, [EDX]\n";
				assembly += "\tMOV ECX, [EDX+4]\n";
				assembly += "\tMOV [ECX], EAX\n";
				break;
			case TOKEN_SQUARE_BRACKETS:
				assembly += "\t; []\n";
				assembly += "\tSUB EDX, 4\n";
				assembly += "\tMOV EAX, [EDX-4]\n";
				assembly += "\tSHL EAX, 2\n";
				assembly += "\tADD EAX, [EDX]\n";
				assembly += "\tMOV [EDX-4], EAX\n";
				break;
			case TOKEN_LESS_THAN:
				assembly += "\t; <\n";
				assembly += "\tSUB EDX, 4\n";
				assembly += "\tMOV EAX, D[EDX-4]\n";
				assembly += "\tCMP EAX, [EDX]\n";
				assembly += "\tMOV D[EDX-4], 1\n";
				assembly += std::string("\tJS >LESS_THAN") + std::to_string(i) + "\n";
				assembly += "\tMOV D[EDX-4], 0\n";
				assembly += std::string("LESS_THAN") + std::to_string(i) + ":\n";
				break;
			}
			break;

		case TT_IDENTIFIER:
			if (t.semantic == ST_WORD_DEFINITION) {
				assembly += "\t; Word def " + t.text + "\n";
				assembly += "WORD_" + std::to_string(t.value) + ":\n";
			} else if (t.semantic == ST_WORD_INVOCATION) {
				assembly += "\t; Invoke " + t.text + "\n";
				assembly += "\tCALL WORD_" + std::to_string(t.value) + "\n";
			} else if (t.semantic == ST_VARIABLE_DECLARATION) {
				// ignore
			} else if (t.semantic == ST_VARIABLE_REFERENCE) {
				// push variable address to data stack
				assembly += "\t; Variable ref " + t.text + "\n";
				assembly += std::string("\tMOV [EDX], ADDR VAR_") + t.text + "\n";
				assembly += "\tADD EDX, 4\n";
			}
			break;

		case TT_INT_LITERAL:
			assembly += std::string("\t; int literal ") + std::to_string(t.value) + "\n";
			assembly += std::string("\tMOV D[EDX], ") + std::to_string(t.value) + "\n";
			assembly += "\tADD EDX, 4\n";
			break;

		case TT_CHAR_LITERAL:
			assembly += std::string("\t; char literal \'") + removeNewline(t.text) + "\'\n";
			for (int j = t.text.size() - 1; j >= 0; j--) {
				assembly += std::string("\tMOV D[EDX], ") + std::to_string((int)t.text[j]) + "\n";
				assembly += "\tADD EDX, 4\n";
			}
			break;

		case TT_STRING_LITERAL:
			assembly += std::string("\t; string literal \"") + removeNewline(t.text) + "\"\n";
			assembly += "\tMOV D[EDX], 0\n";
			assembly += "\tADD EDX, 4\n";
			for (int j = t.text.size() - 1; j >= 0; j--) {
				assembly += std::string("\tMOV D[EDX], ") + std::to_string((int)t.text[j]) + "\n";
				assembly += "\tADD EDX, 4\n";
			}
			break;
		}

	}

	// append the footer
	assembly += footer;

	return true;
}

bool CodeGenx86::generateVariableDeclarations(const std::vector<Token> &bytecode, std::string &assembly) {
	for (size_t i = 0; i < bytecode.size(); i++) {
		const Token &t = bytecode[i];
		if (t.semantic != ST_VARIABLE_DECLARATION) continue;

		if (t.getArraySize() == 1) {
			assembly += std::string("\tVAR_") + t.text + " DD 0\n";
		} else {
			assembly += std::string("\tVAR_") + t.text + " DD " + std::to_string(t.getArraySize()) + " DUP 0\n";
		}
	}
	
	return true;
}

std::string CodeGenx86::removeNewline(const std::string &s) {
	std::string ret = "";
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == '\n') ret += "\\n";
		else ret += s[i];
	}

	return ret;
}

const std::string CodeGenx86::dataSection =
"; --------------------------------------------------------------\n"
"; Data\n"
"; --------------------------------------------------------------\n"
"DATA SECTION\n"
"\tWRKEEP DD 0\n"
"\tSTDOUT DD 0\n"
"\tSTDIN DD 0\n"
"\tTmpDWORD DD 0\n"
"\tDataStack DD 1024 DUP ?\n"
"\tLineFeed DB 0Dh, 0Ah\n"
"\n"
;

const std::string CodeGenx86::initializationSection =
"; --------------------------------------------------------------\n"
"; Initialization\n"
"; --------------------------------------------------------------\n"
"\n"
"CODE SECTION\n"
"START:\n"
"\n"
"\t; Allocate a console\n"
"\tCALL AllocConsole\n"
"\n"
"\t; get STDIN and STDOUT handles\n"
"\tPUSH -11\n"
"\tCALL GetStdHandle\n"
"\tMOV [STDOUT], EAX\n"
"\tPUSH -10\n"
"\tCALL GetStdHandle\n"
"\tMOV [STDIN], EAX\n"
"\n"
"\t; EDX is our data stack pointer\n"
"\tMOV EDX, ADDR DataStack\n"
"; --------------------------------------------------------------\n"
"; User code\n"
"; Program source :\n"
"; "
;

const std::string CodeGenx86::footer =
";--------------------------------------------------------------\n"
"; process return\n"
";--------------------------------------------------------------\n"
"\tMOV EAX, 0\n"
"\tRET\n"
"\n"
;