// main.cpp
// "alpha" compiler

#include <stdio.h>
#include "tokenizer.h"
#include "matcher.h"
#include "vm.h"
#include "CodeGenx86.h"

#include <windows.h>

// utility
std::string readFile(const std::string &filename, bool printError=true) {
	FILE *fp = fopen(filename.c_str(), "rb");
	if (!fp) {
		if (printError) printf("E: Failed to open file %s\n", filename.c_str());
		return "";
	}

	fseek(fp, 0, SEEK_END);
	size_t sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::string ret;
	ret.resize(sz);
	fread(&ret[0], sizeof(char)/*duh*/, sz, fp);

	fclose(fp);

	return ret;
}


// for speed comparison
int fibo(int n) {
	if (n <= 0) return 1;
	if (n == 1) return 1;

	return fibo(n - 1) + fibo(n - 2);
}

int main(int argc, char *argv[]) {

	std::string source;
	std::string destination;

	std::string library;
	
	bool interpret = false;

	if (argc == 2) {
		source = readFile(argv[1]);
		destination = std::string(argv[1]);
		destination = destination.substr(0, destination.size() - 4) + ".asm";
		library = readFile("library.txt");
		printf("Destination assembly file: %s\n", destination.c_str());
		interpret = false;
	} else {
		source = readFile("../../GoAsm/program.txt");
		destination = "../../GoAsm/program.asm";
		library = readFile("../../GoAsm/library.txt");
		interpret = true;
	}

	// prepend library
	source = library + source;

	Tokenizer tokenizer;
	std::vector<Token> bytecode = tokenizer.tokenize(source);
	
	Matcher matcher;
	if (!matcher.match(bytecode)) {
		printf("E: Unmatched code\n");
		return 0;
	}
	
	if (interpret) {
		VM vm;
		vm.execute(bytecode);
	}
				
	CodeGenx86 codeGen;
	std::string assembly = "";
	if (!codeGen.generate(bytecode, assembly)) {
		printf("E: Failed to generate assembly\n");
		return 0;
	}

	FILE *fo = fopen(destination.c_str(), "w");
	if (!fo) {
		printf("Failed to open output file\n");
		return 0;
	}

	fprintf(fo, "%s", assembly.c_str());

	fclose(fo);

	return 0;
}