# QuickFuck
A simple, lightweight-ish C++ based brainfuck interpreter and library. It is named such because I wrote it in ~1 day. QuickFuck Currently does not support any extensions or negative cell locations.

## Interpreter Usage
The interpreter, which is built from `qfmain.cpp`, is easy to use:

`quickfuck <file-path>` - Executes the file provided

`quickfuck -e <code>` - Executes the code directly

If multiple arguments that can be interpreted as a file or code are provided, the last one will always be used:

`quickfuck <first> <second>` will execute second only
### Flags
- `--performance` or `-p`, switches to the "Performance" or fixed-size interpreter, in place of the dynamically sized one. The size defaults to `256`, but it can be changed by preceding the flag with a number: `-p 32`
- `--verbose` or `-v`, Simply prints the values of the cells after execution. Not really much point when `#` exists.
- `--eval` or `-e`, switches from file mode to direct evaluation.
- `--help` or `-h`, it's help

## Library usage
```cpp
// You can initialize either a dynamic interpreter
Brainfuck::DynamicInterpreter interpreter("++++++++[->++++++<]>.");
std::string output = interpreter.interpret();
```
```cpp
// or a performance interpreter
// Open with 1 cell
Brainfuck::PerformanceInterpreter interpreter(",.,.,.,.,.,.,.,.,.,.,.,.", 1u);
// Pass input to interpret
std::cout << interpreter.interpret("Hello World!") << std::endl;
```
You can also use the `Brainfuck::Interpreter` class to contain a generic interpreter.
There is also `getTape()` which returns an `std::vector<int>` containing the values in each cell, as well as `getValue(size_t)`, which returns an `int` for the value in the accompanying cell.

### TODO
- Library improvements
  - Step-wise interpretation
  - More manipulation functions