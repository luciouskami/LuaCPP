#include <iostream>
#include <string.h>

#include "luacpp.h"

using namespace std;

namespace Main {
	struct Flags {
		bool debug;
		bool compile;
		bool empty;
		bool invalid;
		char input[64];
		char output[64];
	};

	Flags* getFlags(const int argc, const char* argv []) {
		Flags* flags = new Flags();
		int count = 0;

		if (argc == 1) { return 0; }

		strcpy(flags->input, argv[1]);

		for (int i = 2; i < argc; ++i) {
			if (strcmp(argv[i], "-d") == 0) {
				flags->debug = true;
				++count;
			} //if
			else if (strcmp(argv[i], "-c") == 0) {
				flags->compile = true;
				++count;
				
				if (++i < argc) {
					strcpy(flags->output, argv[i]);
				} //if
				else {
					delete flags;

					return 0;
				}
			} //else if
			else {
				flags->invalid = true;
				++count;
			} //else
		} //for

		if (flags->empty || flags->invalid) {
			delete flags;

			return 0;
		}

		return flags;
	}

	void displayUsage() {
		cout << endl;
		cout << "Usage\n\n";
		cout << "---------------------------------------------------------------\n\n";
		cout << "\t\"filename\"\n\t\tName of the input file with code in it\n\t\tMust Be First Argument\n\n";
		cout << "\t-d\n\t\tDebug Mode (Non-Compile Mode Only)\n\n";
		cout << "\t-c \"filename\"\n\t\tCompile Mode with output file name\n\n\n";
	}
}

using namespace Main;

int main(const int argc, const char* argv []) {
	Flags* flags = getFlags(argc, argv);

	if (flags == 0) {
		displayUsage();

		return 0;
	} //if

	cout << endl;
	cout << "Input File: " << flags->input << endl;
	if (flags->debug) cout << "Debug Mode: Enabled\n";
	else cout << "Debug Mode: Disabled\n";
	if (flags->compile) cout << "Compile Mode: Enabled -> " << flags->output << endl;
	else cout << "Compile Mode: Disabled\n";

	cout << endl;

	//Lua lua;

	//lua.l_openlibs();

	delete flags;

	return 0;
}