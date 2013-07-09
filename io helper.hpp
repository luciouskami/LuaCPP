#pragma once

#include <fstream>
#include <stdint.h>

using namespace std;

class IO_Helper {
	public:
		static char* read(const char* filename) {
			// open the file for binary reading
			ifstream file;
			file.open(filename, ios::binary);
			if(!file.is_open()) return 0;

			// get the length of the file
			file.seekg(0, ios::end);
			uint32_t fileSize = file.tellg();

			// read the file
			file.seekg(0, ios::beg);
			char* data = new char[fileSize + 1];
			file.read(data, fileSize);
			data[fileSize] = 0;

			// close the file
			file.close();

			return data;
		} //read
}; //IO_Helper