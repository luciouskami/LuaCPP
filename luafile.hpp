#pragma once

#include <stdint.h>
#include <cstring>
#include <fstream>
#include <limits>

class LuaFile {
	private:
		struct Data {
			char* start;
			char* end;
		};

		class DynamicArray {
			public:
				size_t size;
				uint32_t pos;
				Data* arr;

				DynamicArray() : size(4), pos(0) {
					arr = new Data[size];
				}

				~DynamicArray() {
					if (arr != nullptr) {
						delete[] arr;
					}
				}

				void resize(size_t newSize) {
					Data* n_arr = new Data[newSize];

					if (arr != nullptr) {
						std::copy(arr, arr + size, n_arr);
						delete[] arr;
					}
					
					arr = n_arr;
					size = newSize;
				}

				Data& operator [](uint32_t index) {
					if (index >= size) {
						if (index < (index << 1)) {
							resize(static_cast<size_t>(index << 1));
						} else {
							resize(static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
						}
					}

					return arr[index];
				}

				void push_back(Data&& v) {
					(*this)[pos++] = v;
				}

				void set_pos(size_t p_pos) {
					pos = p_pos;
				}
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

				Queue() : last(nullptr), first(nullptr) {
				}

				~Queue() {
					Node* node = first;
					while (node != nullptr) {
						first = node->next;
						delete node;
					}
				}

				void push() {
					Node* node = new Node();

					node->next = nullptr;

					if (last == nullptr) {
						first = node;
					} else {
						last->next = node;
					}
					
					last = node;
				}

				void pop() {
					if (first == nullptr) {
						return;
					}

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
					if (region != nullptr) {
						if (region->data.pos) {
							while (region != nullptr && region->data.pos == word) {
								region = region->next;
							}
							word = 0;
						} else {
							++word;
						}
					}
				}

			public:
				iterator(Queue* p_ptr) : ptr(p_ptr), region(p_ptr->first), word(0) {
				}

				iterator& operator++() {
					getNext();
					return *this;
				}

				iterator& operator++(int junk) {
					getNext();
					return *this;
				}

				const Data operator*() {
					if (region == nullptr) {
						return Data();
					}

					return region->data[word];
				}
				
				Data operator->() {
					if (region == nullptr) {
						return Data();
					}

					return region->data[word];
				}
				
				const size_t size() const {
					return static_cast<size_t>(region->data[word].end - region->data[word].start + 1);
				}

				const char* addr() const {
					return region->data[word].start;
				}
				
				void data(char* buffer) {
					std::copy(region->data[word].start, region->data[word].start + size(), buffer);
					buffer[size()] = '\0';
				}
				
				bool operator==(const iterator& rhs) const { return ptr == rhs.ptr; }
				bool operator!=(const iterator& rhs) const { return ptr != rhs.ptr; }
				
				void write(char* start, char* end) {
					if (region != nullptr) { 
						region->data.push_back(Data{start, end});
					}
				}
				
				void push() {
					ptr->push();
					if (region == nullptr) {
						region = ptr->last;
						word = 0;
					}
				}
				
				bool end() const {
					return region == nullptr;
				}
			
			private:
				Queue* ptr;
				Queue::Node* region;
				size_t word;
		};

		size_t size() const {
			size_t fsize = 0;

			for (auto node = file.first; node != 0; node = node->next) {
				for (uint32_t i = 0; i < node->data.pos; ++i) {
					fsize += static_cast<size_t>(node->data[i].end - node->data[i].start + 1);
				}
			}

			return fsize;
		}

		void dump(std::ostream& out) const {
			for (auto node = file.first; node != 0; node = node->next) {
				for (uint32_t i = 0; i < node->data.pos; ++i) {
					out.write(node->data[i].start, static_cast<std::streamsize>(node->data[i].end - node->data[i].start + 1));
				}
			}
		}

		iterator begin() { return iterator(&file); }
};