// vm.cpp

#include "vm.h"

VM::VM() {

}

VM::~VM() {

}

void VM::execute(const std::vector<Token> &bytecode) {
	PC = 0;

	while (step(bytecode));

	// free variable memory
	for (size_t i = 0; i < variableAddresses.size(); i++) {
		free(variableAddresses[i]);
	}
	variableAddresses.clear();
}

bool VM::step(const std::vector<Token> bytecode) {
	if (PC < 0 || PC >= bytecode.size()) {
		return false;
	}

	// fetch
	Token t = bytecode[PC++];

	switch (t.type) {
	case TT_PREDEFINED_WORD:
		switch (t.value) {
		case TOKEN_WORD_DEFINITION_BEGIN:
			return handleDefinitionBegin(bytecode);
			break;

		case TOKEN_VARIABLE:
		case TOKEN_ARRAY: // arrays are just variables with more space
			return handleVariableDeclaration(bytecode);
			break;
		

		case TOKEN_IF:
			return handleIF(bytecode, t.match1, t.match2);
			break;

		case TOKEN_ELSE:
			return handleELSE(bytecode, t.match1);
			break;
		case TOKEN_FI:
			// ignore FI if reached from within IF
			break;

		// this is identical to RET
		case TOKEN_WORD_DEFINITION_END:
			return handleDefinitionEnd(bytecode);
			break;

		case TOKEN_RET:
			handleRET(bytecode);
			break;

		case TOKEN_DO:
			return handleDO(bytecode, t.match1);
			break;
		case TOKEN_OD:
			return handleOD(bytecode);
			break;

		case TOKEN_PLUS:
			return handlePLUS();
			break;
		case TOKEN_MINUS:
			return handleMINUS();
			break;
		case TOKEN_TIMES:
			return handleTIMES();
			break;
		case TOKEN_OVER:
			return handleOVER();
			break;

		case TOKEN_PRINT_TOP_CHAR:
			return handlePrintTopChar();
			break;

		case TOKEN_STACK_SIZE:
			return handleStackSize();
			break;
		case TOKEN_MOVE_TO_RETURN_STACK:
			return handleMoveToReturnStack();
			break;
		case TOKEN_MOVE_FROM_RETURN_STACK:
			return handleMoveFromReturnStack();
			break;

		// variable get/set
		case TOKEN_GET:
			return handleGET();
			break;
		case TOKEN_SET:
			return handleSET();
			break;
		case TOKEN_SQUARE_BRACKETS:
			return handleSquareBrackets();
			break;

		// comparisons
		case TOKEN_LESS_THAN:
			return handleLessThan();
			break;

		}
		break;

	case TT_IDENTIFIER:
		if (t.semantic == ST_WORD_INVOCATION) if (!handleUserWordInvocation(bytecode)) return false;
		if (t.semantic == ST_VARIABLE_REFERENCE) if (!handleVariableReference(bytecode, t)) return false;
		break;


	case TT_INT_LITERAL:
		// push literal to data stack
		dataStack.push(t.value);
		break;

	case TT_STRING_LITERAL:
		// push literal followed by zero (reversed) to data stack
		dataStack.push(0);
		for (int i = t.text.size() - 1; i >= 0; i--) {
			dataStack.push((CELL)t.text[i]);
		}
		break;

	case TT_CHAR_LITERAL:
		// push literal (reversed) to data stack
		for (int i = t.text.size() - 1; i >= 0; i--) {
			dataStack.push((CELL)t.text[i]);
		}
		break;

	}

	return true;
}

bool VM::handleDefinitionBegin(const std::vector<Token> bytecode) {
#ifdef RUNTIME_CHECKS
	// PC better point to a user word
	if (bytecode[PC].type != TT_IDENTIFIER) {
		printf("E: %s is not a user word\n", bytecode[PC].text.c_str());
		return false;
	}

	// user words must come in order from the matcher
	if (userWordEntryPoints.size() != bytecode[PC].value) {
		printf("E: User word %s not in order (%d != %d)\n", bytecode[PC].text.c_str(), bytecode[PC].value, userWordEntryPoints.size());
		return false;
	}
#endif
	
	// take a note of the entry point for the user word
	userWordEntryPoints.push_back(PC + 1);
	PC++;

	// skip to the end of definition
	while (!(bytecode[PC].type == TT_PREDEFINED_WORD && bytecode[PC].value == TOKEN_WORD_DEFINITION_END)) {
		if (PC >= bytecode.size()) return false;
		PC++;
	}
	
	// go to next token
	PC++;

	return true;
}

bool VM::handleVariableDeclaration(const std::vector<Token> bytecode) {
#ifdef RUNTIME_CHECKS
	// PC better point to a variable
	if (bytecode[PC].type != TT_IDENTIFIER) {
		printf("E: %s is not a variable\n", bytecode[PC].text.c_str());
		return false;
	}

	// variables must come in order from the matcher
	if (variableAddresses.size() != bytecode[PC].value) {
		printf("E: Variable %s not in order (%d != %d)\n", bytecode[PC].text.c_str(), bytecode[PC].value, variableAddresses.size());
		return false;
	}
#endif

	// allocate variable
	variableAddresses.push_back((CELL*)malloc(bytecode[PC].getArraySize() * sizeof(CELL)));
	
	// go to next token
	PC++;

	return true;
}

bool VM::handleDefinitionEnd(const std::vector<Token> bytecode) {
#ifdef RUNTIME_CHECKS
	// this is identical to RET
	if (returnStack.empty()) {
		printf("E: ; : return stack empty\n");
		return false;
	}
#endif

	PC = returnStack.top(); returnStack.pop();
	return true;
}


bool VM::handleRET(const std::vector<Token> bytecode) {
#ifdef RUNTIME_CHECKS
	if (returnStack.empty()) {
		printf("E: RET : return stack empty\n");
		return false;
	}
#endif

	PC = returnStack.top(); returnStack.pop();
	return true;
}

bool VM::handleIF(const std::vector<Token> bytecode, CELL elseIndex, CELL fiIndex) {
#ifdef RUNTIME_CHECKS
	// check truth value on top of data stack
	if (dataStack.empty()) {
		printf("E: IF: Data stack is empty\n");
		return false;
	}
#endif
	
	CELL top = dataStack.top(); dataStack.pop();

	if (top) {
		// good to go
		return true;
	} else {
		// skip to ELSE or FI
		PC = elseIndex + 1;
	}

	return true;
}

bool VM::handleELSE(const std::vector<Token> bytecode, CELL fiIndex) {
	// we reached here without the ELSE being active. Skip to FI
	PC = fiIndex + 1;

	return true;
}

bool VM::handleDO(const std::vector<Token> bytecode, CELL odIndex) {
#ifdef RUNTIME_CHECKS
	// check truth value on top of data stack
	if (dataStack.empty()) {
		printf("E: DO: Data stack is empty\n");
		return false;
	}
#endif 

	CELL top = dataStack.top();
	if (top) {
		// good to go. remember where to jump back when OD is found
		returnStack.push(PC - 1);

		return true;
	} else {
#ifdef RUNTIME_CHECKS
		// terminate loop. first pop the looping variable
		if (dataStack.empty()) {
			printf("E: DO: Data stack empty\n");
			return false;
		}
#endif

		dataStack.pop();

		// skip to OD
		PC = odIndex + 1;
	}

	return true;
}

bool VM::handleOD(const std::vector<Token> bytecode) {
	// we are here because DO was executed			
	// jump back
#ifdef RUNTIME_CHECKS
	if (returnStack.empty()) {
		printf("E: OD: return stack empty\n");
		return false;
	}
#endif

	PC = returnStack.top(); returnStack.pop();
	return true;
}

bool VM::handlePLUS() {
#ifdef RUNTIME_CHECKS
	if (dataStack.size() < 2) {
		printf("E: +: dataStack size %d\n", dataStack.size());
		return false;
	}
#endif

	CELL a = dataStack.top(); dataStack.pop();
	CELL b = dataStack.top(); dataStack.pop();

	a = b + a;
	dataStack.push(a);
	
	return true;
}

bool VM::handleMINUS() {
#ifdef RUNTIME_CHECKS
	if (dataStack.size() < 2) {
		printf("E: -: dataStack size %d\n", dataStack.size());
		return false;
	}
#endif

	CELL a = dataStack.top(); dataStack.pop();
	CELL b = dataStack.top(); dataStack.pop();

	a = b - a;
	dataStack.push(a);

	return true;
}

bool VM::handleTIMES() {
#ifdef RUNTIME_CHECKS
	if (dataStack.size() < 2) {
		printf("E: *: dataStack size %d\n", dataStack.size());
		return false;
	}
#endif

	CELL a = dataStack.top(); dataStack.pop();
	CELL b = dataStack.top(); dataStack.pop();

	a = b * a;
	dataStack.push(a);

	return true;
}

bool VM::handleOVER() {
#ifdef RUNTIME_CHECKS
	if (dataStack.size() < 2) {
		printf("E: /: dataStack size %d\n", dataStack.size());
		return false;
	}
#endif

	CELL a = dataStack.top(); dataStack.pop();
	CELL b = dataStack.top(); dataStack.pop();

	a = b / a;
	dataStack.push(a);

	return true;
}

bool VM::handlePrintTopChar() {
#ifdef RUNTIME_CHECKS
	if (dataStack.empty()) {
		printf("E: . : data stack is empty\n");
		return false;
	}
#endif

	char a = (char)dataStack.top(); dataStack.pop();
	printf("%c", a);
	return true;
}

bool VM::handleStackSize() {
	dataStack.push((CELL)dataStack.size());
	return true;
}

bool VM::handleMoveToReturnStack() {
#ifdef RUNTIME_CHECKS
	if (dataStack.empty()) {
		printf("E: >R: data stack empty\n");
		return false;
	}
#endif

	returnStack.push(dataStack.top()); dataStack.pop();

	return true;
}

bool VM::handleMoveFromReturnStack() {
#ifdef RUNTIME_CHECKS
	if (returnStack.empty()) {
		printf("E: <R: return stack empty\n");
		return false;
	}
#endif

	dataStack.push(returnStack.top()); returnStack.pop();

	return true;
}

bool VM::handleGET() {
#ifdef RUNTIME_CHECKS
	if (dataStack.empty()) {
		printf("E: GET: data stack is empty\n");
		return false;
	}
#endif

	CELL *p = (CELL*)dataStack.top(); dataStack.pop();
	dataStack.push(*p);

	return true;
}

bool VM::handleSET() {
#ifdef RUNTIME_CHECKS
	if (dataStack.size() < 2) {
		printf("E: SET: data stack less than 2 elements\n");
		return false;
	}
#endif

	CELL *p = (CELL*)dataStack.top(); dataStack.pop();
	CELL val = dataStack.top(); dataStack.pop();
	*p = val;

	return true;
}

bool VM::handleSquareBrackets() {
#ifdef RUNTIME_CHECKS
	if (dataStack.size() < 2) {
		printf("E: []: data stack has fewer than 2 elements\n");
		return false;
	}
#endif

	CELL *p = (CELL*)dataStack.top(); dataStack.pop();
	CELL index = dataStack.top(); dataStack.pop();
	dataStack.push((CELL)(p + index));

	return true;
}

bool VM::handleLessThan() {
#ifdef RUNTIME_CHECKS
	if (dataStack.size() < 2) {
		printf("E: <: data stack has fewer than 2 elements\n");
		return false;
	}
#endif

	CELL b = dataStack.top(); dataStack.pop();
	CELL a = dataStack.top(); dataStack.pop();
	if (a < b) dataStack.push(1);
	else dataStack.push(0);

	return true;
}

bool VM::handleUserWordInvocation(const std::vector<Token> bytecode) {
#ifdef RUNTIME_CHECKS
	if (bytecode[PC-1].value >= userWordEntryPoints.size()) {
		printf("E: User word %s used before definition\n", bytecode[PC-1].text.c_str());
		return false;
	}
#endif

	// set return address to here
	returnStack.push(PC);

	// jump!
	PC = userWordEntryPoints[bytecode[PC-1].value];

	return true;
}

bool VM::handleVariableReference(const std::vector<Token> bytecode, const Token &t) {
#ifdef RUNTIME_CHECKS
	if (t.value >= variableAddresses.size()) {
		printf("E: Variable %s used before declaration\n", t.text.c_str());
		return false;
	}
#endif

	// push address to data stack
	dataStack.push((size_t)variableAddresses[t.value]);

	return true;
}

