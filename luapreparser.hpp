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

				char* token;			//end token
				char* begin;			//where the block begins
				bool nest;				//allows nesting of blocks
			};

			Node* first;

			Stack() { first = 0; }
			void push(char* token, char* begin, bool nest) { Node* node = new Node(); node->next = first; first = node; node->token = token; node->begin = begin; node->nest = nest; }
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

		struct Token {
			char* end;
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

			/*
				node.nest				allow nesting within block
										if false, ignore tokens until matching end token is found, but still allows nesting of the parent token

				node.begin				where the block begins
				
				node.token				the end token of the block (block exit)
										this token may be ignored by escaping it
			*/

			for (char* c = data.str; c < data.str + data.size; ++c) {
				//exit from an ender token, double
				//error, incorrect check
				if (stack.first != 0 && *c == stack.first->token[0] && *(c + 1) == stack.first->token[1]) {
					stack.pop();
					++c;
				}
				//exit from an ender token, single
				//error, incorrect check
				else if (stack.first != 0 && *c == stack.first->token[0]) {
					stack.pop();
				}
				//enter a double token
				else if (ender[*c][*(c + 1)] != 0) {
					stack.push(ender[*c][*(c + 1)], c, nested[*c][*(c + 1)]);
					++c;
				}
				//enter a single token
				else if (ender[*c][0] != 0) {
					stack.push(ender[*c][0], c, nested[*c][0]);

					if (stack.first != 0 && stack.first->nest && !stack.first->next->nest) {

					}
				}
				//continue preprocess
				else if (stack.first->nest) {
					//do nothing
				}
				//continue code
				else {
					//do nothing
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
