/*
	QuickFuck library, a lightweight C++ Brainfuck interpreter library
	Currently has Brainfuck::DynamicInterpreter and Brainfuck::PerformanceInterpreter
	This is the library version, designed to be used in other programs
	By radposeidon
*/

#include <string>
#include <stack>
#include <vector>
#include <fstream>
#include <sstream>
#include <math.h>

namespace Brainfuck {
	/// Base/Abstract class
	class Interpreter {
	public:
		virtual std::string interpret(std::string) {return "";}
		virtual std::vector<int> getTape() {return {};}
		virtual int getValue(size_t) {return 0;}
		virtual size_t getSize() {return 0;}
	};

	/// The most basic interpreter. Rather memory hefty, does not support negative cell coords
	/// This interpreter is dynamically sized, and will grow as more cells are used
	class DynamicInterpreter : public Interpreter {
		std::vector<int> cells;
		std::string code;
		std::stack<int> loops;
		size_t position;
		size_t active_cell = 0;

	public:
		DynamicInterpreter() {}
		/// @param s The source code
		DynamicInterpreter( std::string s ) : code(s) {}
		/// @param f The file to read from
		DynamicInterpreter( std::ifstream &f ) {
			std::stringstream buff;
			buff << f.rdbuf();
			this->code = buff.str();
		}

		/// Load from a string
		/// @param s The string to load from
		void load( std::string s ) {
			this->code = s;
		}
		/// Load form a file
		/// @param f The file to load from
		void load( std::ifstream &f ) {
			std::stringstream buff;
			buff << f.rdbuf();
			this->code = buff.str();
		}

		/// Interpret the code from the beginning
		virtual std::string interpret( std::string input="" ) {
			position = 0;
			// Initilize the tape
			cells = { 0 };
			std::string output = "";
			while( position < code.length() ) {
				switch( code[position] ) {
					case '+':
						cells[active_cell]++;
						break;
					case '-':
						cells[active_cell]--;
						break;
					case '<':
						if(active_cell > 0) // No negative
							active_cell--;
						break;
					case '>':
						if( active_cell == cells.size() -1 )
							cells.emplace_back(0);
						active_cell++;
						break;
					case '[':
						loops.push(position);
						break;
					case ']':
						if( cells[active_cell] == 0 ) {
							loops.pop();
						}else {
							position = loops.top();
						}
						break;
					case '.':
						output += (char)(cells[active_cell]);
						break;
					case ',':
						if( input.length() == 0 ) {
							throw std::range_error("End of input");
						}
						cells[active_cell] = (int)(input[0]);
						input.erase(0, 1);
						break;
				}
				position++;
			}
			return output;
		}

		/// Print the value of all the cells
		virtual std::vector<int> getTape() {
			return cells;
		}
		virtual int getValue(size_t i) {
			if(i > cells.size()) {
				throw std::range_error("Out of bounds");
			}
			return cells[i];
		}
		virtual size_t getSize() {
			return cells.size();
		}
	};

	/// This is the "performance" version of the interpreter, in that it uses marginally less memory
	/// This is not dynamically sized, nor does it support negative cell keys
	class PerformanceInterpreter : public Interpreter {
		unsigned char* bytes;
		size_t position;
		std::stack<size_t> loops;
		size_t active_cell;
		size_t size;
		std::string code;
	public:
		/// @param s The source code to build from
		/// @param width The width/length of the tape
		PerformanceInterpreter( std::string s, size_t width ) : code(s), size(width) {
			bytes = (unsigned char*)malloc(width);
			// Zero bytes
			for(size_t i = 0; i < width; i++) {
				bytes[i] = 0;
			}
		}
		PerformanceInterpreter( std::ifstream &f, size_t width ) : size(width) {
			bytes = (unsigned char*)malloc(width);
			// Zero bytes
			for(size_t i = 0; i < width; i++) {
				bytes[i] = 0;
			}
			std::stringstream buff;
			buff << f.rdbuf();
			this->code = buff.str();
		}

		/// Interprets the code form position 0
		virtual std::string interpret( std::string input="") {
			position = 0;
			active_cell = 0;

			std::string output = "";
			while( position < code.length() ) {
				switch(code[position]) {
					case '+':
						bytes[active_cell]++;
						break;
					case '-':
						bytes[active_cell]--;
						break;
					case '<': // This version has no hand holds
						active_cell--;
						break;
					case '>':
						active_cell++;
						break;
					case '[':
						loops.push(position);
						break;
					case ']':
						if(bytes[active_cell] == 0) {
							loops.pop();
						}else {
							position = loops.top();
						}
						break;
					case '.':
						output += (char)(bytes[active_cell]);
						break;
					case ',':
						if(input.length() == 0) {
							throw std::range_error("Input is empty, nothing more to read");
						}
						bytes[active_cell] = input[0];
						input.erase(0,1);
				}
				position++;
			}
			return output;
		}

		// Prints the contents of each cell
		virtual std::vector<int> getTape() {
			std::vector<int> v;
			for(size_t i = 0; i < size; i++) {
				if(i > size) {
					throw std::range_error("Out of bounds");
				}
				v.emplace_back((int)bytes[i]);
			}
			return v;
		}
		virtual int getValue(size_t i) {
			return (int)bytes[i];
		}
		virtual size_t getSize() {
			return size;
		}
	};
}