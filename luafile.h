#pragma once

#include <stdint.h>
#include <cstring>
#include <fstream>

using namespace std;

class LuaFile {
	private:
		union Data { char* start; char* end; };

		class DynamicArray {
			public:
				size_t size;
				uint32_t pos;
				Data* arr;

				DynamicArray() {
					size = 4;
					pos = 0;

					arr = new Data[size];
				}
				~DynamicArray() { delete[] arr; }

				void resize(size_t newSize) {
					Data* n_arr = new Data[newSize];
					memcpy(n_arr, arr, size*sizeof(Data));
					delete[] arr;
					n_arr = arr;
					size = newSize;
				}

				Data& operator [](uint32_t idx) {
					if (idx >= size) resize((size_t)(idx < (idx << 1)? (idx << 1) : UINT32_MAX));
					return arr[idx];
				}

				void push_back(Data v) { (*this)[pos++] = v; }
				void set_pos(size_t pos) { this->pos = pos; }
		};

		class Queue {
			public:
				class Node {
					public:
						DynamicArray data;
						Node* next;
				};

				Node* last;
				Node* first;

				Queue() {
					last = 0;
					first = 0;
				}

				~Queue() {
					Node* node = first;
					while (node != 0) {
						first = node->next;
						delete node;
					}
				}

				void push() {
					Node* node = new Node();

					node->next = 0;

					if (last == 0)	first = node;
					else			last->next = node;
									last = node;
				}

				void pop() {
					if (first == 0) return;

					Node* node = first;
					first = node->next;
					delete node;
				}
		};

		Queue file;

	public:
		class iterator {
			private:
				void getNext() {
					if (region != 0) {
						if (region->data.pos) {
							while (region != 0 && region->data.pos == word) region = region->next;
							word = 0;
						} //if
						else ++word;
					} //if
					if (region == 0) return;
				}
			public:
				iterator(Queue* ptr) { this->ptr = ptr; region = ptr->first; word = 0; }

				iterator& operator++() { getNext(); return *this; }
				iterator& operator++(int junk) { getNext(); return *this; }

				const Data operator*() { if (region == 0) return Data(); return region->data[word]; }
				Data operator->() { if (region == 0) return Data(); return region->data[word]; }
				const size_t size() { return (size_t)(region->data[word].end - region->data[word].start + 1); }
				const char* addr() { return region->data[word].start; }
				void data(char* buffer) { memcpy(buffer, region->data[word].start, size()); buffer[size()] = 0; }
				bool operator==(const iterator& rhs) { return ptr == rhs.ptr; }
				bool operator!=(const iterator& rhs) { return ptr != rhs.ptr; }
				void write(char* start, char* end) {
					if (region == 0) return;
					
					Data data;
					data.start = start;
					data.end = end;
					region->data.push_back(data);
				}
				void push() {
					ptr->push();
					if (region == 0) region = ptr->last; word = 0;
				}
				bool end() { return region == 0;  }
			private:
				Queue* ptr;
				Queue::Node* region;
				size_t word;
		};

		size_t size() {
			size_t fsize = 0;

			for (LuaFile::Queue::Node* node = file.first; node != 0; node = node->next) {
				for (uint32_t i = 0; i < node->data.pos; ++i) {
					fsize += (size_t)(node->data[i].end - node->data[i].start + 1);
				}
			}

			return fsize;
		}
		void dump(char* buffer) {
			for (LuaFile::Queue::Node* node = file.first; node != 0; node = node->next) {
				for (uint32_t i = 0; i < node->data.pos; ++i) {
					memcpy(buffer, node->data[i].start, (size_t) (node->data[i].end - node->data[i].start + 1));
					buffer += (size_t) (node->data[i].end - node->data[i].start + 1);
				}
			}
		}
		void dump(ofstream& fname) {
			for (LuaFile::Queue::Node* node = file.first; node != 0; node = node->next) {
				for (uint32_t i = 0; i < node->data.pos; ++i) {
					fname.write(node->data[i].start, (size_t) (node->data[i].end - node->data[i].start + 1));
				}
			}
		}
};