// vm.h

#ifndef __VM_H_
#define __VM_H_

#include "tokenizer.h"
#include <stack>
#include <map>

class VM {
public:
	VM();
	virtual ~VM();

	void execute(const std::vector<Token> &bytecode);

private:
	std::stack<CELL> dataStack;
	std::stack<CELL> returnStack;
	int PC;
	std::vector<size_t> userWordEntryPoints;
	std::vector<CELL*> variableAddresses;
	
	bool step(const std::vector<Token> bytecode);

	// word definition
	bool handleDefinitionBegin(const std::vector<Token> bytecode);

	// variable declaration
	bool handleVariableDeclaration(const std::vector<Token> bytecode);
	

	// Flow control
	bool handleIF(const std::vector<Token> bytecode, CELL elseIndex, CELL fiIndex);
	bool handleELSE(const std::vector<Token> bytecode, CELL fiIndex);
	bool handleDO(const std::vector<Token> bytecode, CELL odIndex);
	bool handleOD(const std::vector<Token> bytecode);
	bool handleDefinitionEnd(const std::vector<Token> bytecode);
	bool handleRET(const std::vector<Token> bytecode);
	
	// arithmetic
	bool handlePLUS();
	bool handleMINUS();
	bool handleTIMES();
	bool handleOVER();

	bool handlePrintTopChar();

	// stack manipulation
	bool handleStackSize();
	bool handleMoveToReturnStack();
	bool handleMoveFromReturnStack();

	// variable get/set
	bool handleGET();
	bool handleSET();
	bool handleSquareBrackets();

	// comparisons
	bool handleLessThan();

	bool handleUserWordInvocation(const std::vector<Token> bytecode);
	bool handleVariableReference(const std::vector<Token> bytecode, const Token &t);
};

#endif // ndef __VM_H_