/*
	QuickFuck, a lightweight C++ Brainfuck interpreter
	Currently has Brainfuck::Interpreter and Brainfuck::PerformanceInterpreter
	By Robonics
*/

#include <iostream>
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
		virtual void interpret() {}
		virtual void print() {}
	};

	/// The most basic interpreter. Rather memory hefty, does not support negative cell coords
	/// This interpreter is dynamically sized, and will grow as more cells are used
	class DynamicInterpreter : public Interpreter {
		std::vector<int> cells;
		std::string code;
		std::stack<int> loops;
		size_t position;
		size_t active_cell = 0;
		std::string input_buffer;

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
		virtual void interpret() {
			position = 0;
			// Initilize the tape
			cells = { 0 };
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
						std::cout << (char)(cells[active_cell]);
						break;
					case ',':
						if( input_buffer.length() == 0 ) {
							std::string i;
							std::getline(std::cin, i);
							input_buffer += i;
						}
						cells[active_cell] = (int)(input_buffer[0]);
						input_buffer.erase(0, 1);
						break;
					case '#':
						std::cout << "Debug:\n";
						this->print();
				}
				position++;
			}
			std::cout << std::flush;
		}

		/// Print the value of all the cells
		virtual void print() {
			std::cout << "Cell\tVal\tChar\n";
			for(size_t i = 0; i < cells.size(); i++) {
				std::cout << i << ":\t" << cells[i] << "\t'" << (char)cells[i] << "'\n";
			}
			std::cout << std::endl;
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
		std::string input;
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
		virtual void interpret() {
			position = 0;
			active_cell = 0;

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
						std::cout << (char)(bytes[active_cell]);
						break;
					case ',':
						if(input.length() == 0) {
							std::string i;
							std::getline(std::cin, i);
							input += i;
						}
						bytes[active_cell] = input[0];
						input.erase(0,1);
					case '#':
						std::cout << "\nDebug:\n";
						this->print();
				}
				position++;
			}
		}

		// Prints the contents of each cell
		virtual void print() {
			std::cout << "Cell\tVal\tChar\n";
			for(size_t i = 0; i < size; i++) {
				std::cout << i << ":\t" << (int)bytes[i] << "\t'" << bytes[i] << "'\n";
			}
			std::cout << std::endl;
		}
	};
}

enum Flag {
	Performance = 0b1,
	Verbose = 0b10,
	Expression = 0b100
};

int main( int argc, char** argv ) {

	int flags = 0;
	size_t cell_n = 256u;
	std::string path = "";
	for( int i = 0; i < argc; i++ ) {
		std::string arg = argv[i];
		if( arg == "-p" || arg == "--performance" ) {
			flags |= Flag::Performance; // Enable the performance flag
			try {
				if( i == argc - 1 ) {
					cell_n = 256u; // Default to 256 cells
				}else {
					std::string next_arg = argv[i + 1];
					size_t s = std::stoull(next_arg);
					cell_n = s;
					i++;
				}
			}catch( std::invalid_argument e ) {
				cell_n = 256u;
			}
		}else if( arg == "-v" || arg == "--verbose" ) {
			flags |= Flag::Verbose;
		}else if( arg == "-e" || arg == "--eval" ) {
			flags |= Flag::Expression;
		}else if( arg == "-h" || arg == "--help") {
			std::cout << "Usage:\nquickfuck <file> --flags\n\tFlags:\n\t--performance (-p): Uses the performance interpreter. Specify the size of the tape with a following argument, ex: '-p 32'\n\t--verbose (-v): Show contents of cells after evaluation ends. Also consider using '#' in code\n\t--eval (-e): Switches from file interpretation to interpreting code" << std::endl;
			return 0;
		}else {
			path = argv[i];
		}
	}

	if( path == "" ) {
		std::cerr << "Error: " << ((flags & Flag::Expression)? "expression":"path") << "Cannot be empty" << std::endl;
		return 1;
	}
	std::string code = "";
	if( flags & Flag::Expression ) {
		code = path;
	}else {
		std::ifstream file(path);
		if(!file) {
			std::cerr << "Error: File " << path << " not found" << std::endl;
			return 1;
		}
		std::stringstream buff;
		buff << file.rdbuf();
		code = buff.str();
	}
	if( code == "" ) {
		std::cerr << "Error: No code to evaluate" << std::endl;
		return 1;
	}

	Brainfuck::Interpreter* interp;
	if( flags & Flag::Performance ) {
		if( flags & Flag::Verbose )
			std::cout << "Performance Mode" << std::endl;
		interp = new Brainfuck::PerformanceInterpreter( code, cell_n );
	}else {
		if( flags & Flag::Verbose )
			std::cout << "Dynamic Mode" << std::endl;
		interp = new Brainfuck::DynamicInterpreter( code );
	}
	interp->interpret();
	std::cout << std::endl;
	if( flags & Flag::Verbose )
		interp->print();
}