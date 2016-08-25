// matcher.h

#ifndef __MATCHER_H_
#define __MATCHER_H_

#include "tokenizer.h"
#include <stack>

class Matcher {
public:
	Matcher();
	virtual ~Matcher();

	bool match(std::vector<Token> &bytecode);

private:	
	std::stack<Token*> unmatched;
};

#endif // ndef __MATCHER_H_