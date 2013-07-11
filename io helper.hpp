#pragma once

#include <fstream>
#include <stdint.h>

#include <iostream>

using namespace std;

class IO_Helper {
	public:
		struct Data {
			char* str;
			size_t size;

			Data() { str = 0; size = 0; }
			~Data() { if (str != 0) delete [] str; }
		};

		static Data* read(const char* filename) {
			Data& data = *new Data();

			// open the file for binary reading
			ifstream file;
			file.open(filename, ios::binary);
			if(!file.is_open()) return &data;

			// get the length of the file
			file.seekg(0, ios::end);
			data.size = 1 + file.tellg();

			// read the file
			file.seekg(0, ios::beg);
			data.str = new char[data.size + 1];
			file.read(data.str, data.size);
			data.str[data.size] = 0;

			// close the file
			file.close();

			return &data;
		} //read
}; //IO_Helper
