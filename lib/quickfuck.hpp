/*
	QuickFuck library, a lightweight C++ Brainfuck interpreter library
	Currently has Brainfuck::DynamicInterpreter and Brainfuck::PerformanceInterpreter
	This is the library version, designed to be used in other programs
	By Robonics
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
	protected:
		std::string output;
		std::string input;

		std::string code;
		size_t position;
		size_t active_cell = 0;
		std::stack<size_t> loops;
	public:

		Interpreter() {}
		Interpreter( std::string c ) : code(c) {}

		virtual std::string interpret() {return output;}
		virtual std::string interpret(std::string) {return output;}
		virtual void reset() {}
		virtual void step() {}
		std::string getOutput() {
			return output;
		}
		void clearOutput() {
			output = "";
		}
		std::string getInput() {
			return input;
		}
		void setInput( std::string s ) {
			input = s;
		}
		void addInput( std::string s ) {
			input += s;
		}
		std::string& getCode() {
			return code;
		}
		size_t getPosition() {
			return position;
		}
		void setPosition(size_t i) {
			position = i;
		}
		size_t getIndex() {
			return active_cell;
		}
		void setIndex(size_t i) {
			active_cell = i;
		}
		virtual std::vector<char> getTape() {return {};}
		virtual char getValue(size_t) {return 0;}
		virtual char getValue() {return 0;}
		virtual void setValue(size_t,char) {}
		virtual void setValue(char) {}
		virtual size_t getSize() {return 0;}
	};

	/// The most basic interpreter. Rather memory hefty, does not support negative cell coords
	/// This interpreter is dynamically sized, and will grow as more cells are used
	class DynamicInterpreter : public Interpreter {
		std::vector<char> cells;

	public:
		DynamicInterpreter() {}
		/// @param s The source code
		DynamicInterpreter( std::string s ) : Interpreter(s) {}
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
		virtual std::string interpret() {
			this->reset();
			while( position < code.length() ) {
				this->step();
			}
			return output;
		}
		virtual std::string interpret( std::string in ) {
			this->input = in;
			this->reset();
			while( position < code.length() ) {
				this->step();
			}
			return output;
		}

		virtual void reset() {
			position = 0;
			cells = { 0 };
			output = "";
			input = "";
		}

		virtual void step() {
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
					cells[active_cell] = input[0];
					input.erase(0, 1);
					break;
			}
			position++;
		}

		/// Print the value of all the cells
		virtual std::vector<char> getTape() {
			return cells;
		}
		virtual char getValue(size_t i) {
			if(i > cells.size()) {
				throw std::range_error("Out of bounds");
			}
			return cells[i];
		}
		virtual char getValue() {
			return cells[active_cell];
		}
		virtual void setValue(size_t i, char v) {
			cells[i] = v;
		}
		virtual void setValue(char v) {
			cells[active_cell] = v;
		}
		virtual size_t getSize() {
			return cells.size();
		}
	};

	/// This is the "performance" version of the interpreter, in that it uses marginally less memory
	/// This is not dynamically sized, nor does it support negative cell keys
	class PerformanceInterpreter : public Interpreter {
		unsigned char* bytes;
		size_t size;
	public:
		/// @param s The source code to build from
		/// @param width The width/length of the tape
		PerformanceInterpreter( std::string s, size_t width ) : Interpreter(s), size(width) {
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
		virtual std::string interpret() {
			this->reset();

			while( position < code.length() ) {
				this->step();
			}
			return output;
		}
		virtual std::string interpret(std::string in) {
			input = in;
			this->reset();

			while( position < code.length() ) {
				this->step();
			}
			return output;
		}

		virtual void reset() {
			position = 0;
			active_cell = 0;

			free(bytes);
			bytes = (unsigned char*)malloc(size);
			for(size_t i = 0; i < size; i++) {
				bytes[i] = 0;
			}

			output = "";
			input = "";
		}

		virtual void step() {
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

		// Prints the contents of each cell
		virtual std::vector<char> getTape() {
			std::vector<char> v;
			for(size_t i = 0; i < size; i++) {
				if(i > size) {
					throw std::range_error("Out of bounds");
				}
				v.emplace_back((char)bytes[i]);
			}
			return v;
		}
		virtual char getValue(size_t i) {
			return (char)bytes[i];
		}
		virtual char getValue() {
			return (char)bytes[active_cell];
		}
		virtual void setValue(size_t i, char v) {
			bytes[i] = v;
		}
		virtual void setValue(char v) {
			bytes[active_cell] = v;
		}
		virtual size_t getSize() {
			return size;
		}
	};
}