#include <stdint.h>

#include "luafile.hpp"
#include "io helper.hpp"

using namespace std;

class LuaPreParser {
	private:
		class Stack {
		public:
			struct Node {
				Node* next;
				char* token;
				char* begin;
				bool in;
			};

			Node* first;

			Stack() { first = 0; }
			void push(char* token, char* begin, bool in) { Node* node = new Node(); node->next = first; first = node; node->token = token; node->begin = begin; node->in = in; }
			void pop() { if (first == 0) return; first = first->next; }
			~Stack() {
				Node* node;

				while (first != 0) {
					node = first->next;
					delete first;
					first = node;
				}
			}
		};

		char ender[128][128][3];
		bool nested[128][128];
		/*
		inline uint8_t read(const uint8_t* str) {
			return (filterT[*str][*(str + 1) >> 4] & (1 << (*(str + 1) & 0xf))) >> (*(str + 1) & 0xf);
		}
		inline void set(const uint8_t* str, bool val) {
			filterT[*str][*(str + 1) >> 4] = filterT[*str][*(str + 1) >> 4] & ~(1 << (*(str + 1) & 0xf)) | (val << (*(str + 1) & 0xf));
		}
		*/
	public:
		inline void addFilter(const char* start, const char* end, bool nested) {
			ender[*start][*(start + 1)][0] = end[0];
			ender[*start][*(start + 1)][1] = end[1];
			ender[*start][*(start + 1)][2] = 0;
			this->nested[*start][*(start + 1)] = nested;
		}
		inline void removeFilter(const char* str) {
			ender[*str][*(str + 1)][0] = 0;
		}
		inline char* readFilter(const char* str) {
			return ender[*str][*(str + 1)];
		}
		inline bool isNestedFilter(const char* str) {
			return nested[*str][*(str + 1)];
		}

		LuaPreParser() {
			addFilter("#\0", "\n", false);
		}

		LuaFile* parse(IO_Helper::Data& data, std::ostream& out) {
			if (data.size == 0) return 0;

			LuaFile& file = *new LuaFile();
			auto iterator = file.begin();
			iterator.push();

			Stack stack;
			bool preprocess = false;

			for (char* c = data.str; c < data.str + data.size; ++c) {
				if (stack.first != 0 && *c == stack.first->token[0] && *(c + 1) == stack.first->token[1]) {

				}
				else if (stack.first != 0 && *c == stack.first->token[0]) {

				}
				else if (ender[*c][*(c + 1)] != 0) {

				}
				else if (ender[*c][0] != 0) {
					
				}
				else {
				}

				if (stack.first != 0 && (*c == stack.first->token[0] || *c == stack.first->token[0] && *(c + 1) == stack.first->token[0])) {

				}

				if (*c == '#' && (stack.first == 0 || stack.first->token[0] != '{') && stack.first->token[0] != '#') {
					preprocess = true;
					if (*(c + 1) == '{') {
						++c;
						stack.push(readFilter("#{"), c + 1, true);
					}
					else {
						stack.push(readFilter("#"), c + 1, true);
					}
				}
				else if (*c == stack.first->token[0]) {
					if (stack.first->in) {
						preprocess = !preprocess;
					}

					stack.pop();
				}
				else {

				}
			}
			/*

			if (data.size != 0) {
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
			*/
		}
};
