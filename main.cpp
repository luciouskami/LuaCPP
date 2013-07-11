#include <iostream>
#include <cstring>

#include <bitset>

#include "luacpp.hpp"
#include "luafile.hpp"
#include "io helper.hpp"
#include "luapreparser.hpp"

#include <fstream>

using namespace std;

namespace Main {
	struct Flags {
		bool debug;
		bool compile;
		bool empty;
		bool invalid;
		char input[64];
		char output[64];
	} flags;

	void read_flags(const int argc, const char* argv []) {
		int count = 0;

		if (argc == 1) { flags.empty = true; flags.invalid = true; return; }

		strcpy(flags.input, argv[1]);

		for (int i = 2; i < argc; ++i) {
			if (strcmp(argv[i], "-d") == 0) {
				flags.debug = true;
				++count;
			} //if
			else if (strcmp(argv[i], "-c") == 0) {
				flags.compile = true;
				++count;
				
				if (++i < argc) {
					strcpy(flags.output, argv[i]);
				} //if
				else {
					return;
				}
			} //else if
			else {
				flags.invalid = true;
				++count;
			} //else
		} //for
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
	LuaPreParser parser;
	
	parser.addFilter("/*", "*/", true);

	/*
	LuaPreParser::Filter filter;

	filter.add("/*");
	filter.remove("/*");
	filter.read("/*");

	setter("11", true);
	setter("12", true);
	cout << (int)reader("11") << endl;
	setter("11", false);
	cout << (int) reader("11") << endl;
	cout << (int) reader("12") << endl;
	return 0;
	//1111110
	uint8_t c = '~' + 1;
	cout << bitset<7>(c) << endl;

	uint8_t index = c >> 4;
	cout << (int)(index) << endl;

	uint8_t bit = c & 0xf;
	cout << (int)(bit) << endl;
	*/
	/*
	read_flags(argc, argv);

	if (flags.invalid || flags.empty) {
		displayUsage();

		return 0;
	} //if

	cout << endl;
	cout << "Input File: " << flags.input << endl;
	if (flags.debug) cout << "Debug Mode: Enabled\n";
	else cout << "Debug Mode: Disabled\n";
	if (flags.compile) cout << "Compile Mode: Enabled -> " << flags.output << endl;
	else cout << "Compile Mode: Disabled\n";

	cout << endl << "-------------------------" << endl;

	auto& file_data = *IO_Helper::read(flags.input);

	cout << "File Size: " << file_data.size << endl;
	cout << "File Data: " << file_data.str << endl << endl;

	if (file_data.size != 0) {
		LuaFile file;

		auto iterator = file.begin();
		ofstream output_temp;
		output_temp.open("lua.tmp");
		iterator.push();

		size_t depth = 0;
		char* start = 0;
		for (char* c = file_data.str; c < file_data.str + file_data.size; ++c) {
			if (*c == '#' && depth == 0) {
				if (*(c + 1) == '{') {
					++depth;
					++c;
					start = c;
				}
			}
			else if (depth != 0) {
				if (*c == '}') {
					--depth;

					if (depth == 0) {
						iterator.push();
					}
				}
				else if (*c == '{') {
					++depth;
				}
				else {
					output_temp << *c;
				}
			}
			else {
			}
		}

		output_temp.close();
	}

	delete &file_data;


	//Lua lua;

	//lua.l_openlibs();
	*/
	return 0;
}
