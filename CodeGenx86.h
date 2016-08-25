// CodeGenx86.h

#ifndef __CODE_GEN_x86_H_
#define __CODE_GEN_x86_H_

#include "tokenizer.h"

class CodeGenx86 {
public:
	CodeGenx86();
	virtual ~CodeGenx86();

	bool generate(const std::vector<Token> &bytecode, std::string &assembly);

private:
	static const std::string dataSection, initializationSection, footer;

	bool generateVariableDeclarations(const std::vector<Token> &bytecode, std::string &assembly);

	std::string removeNewline(const std::string &s);
};

#endif // ndef __CODE_GEN_x86_H_