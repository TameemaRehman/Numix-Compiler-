# Numix Compiler

A domain-specific language (DSL) compiler for numerical patterns and mathematical sequences, implemented in C++.

## Overview

MathSeq is an imperative/procedural language designed specifically for working with mathematical sequences, patterns, and numerical computations. It features Python-like syntax with strict typing and compiles to intermediate three-address code.

## Language Features

### 1. **Variables and Type System**
- **Strict typing** with explicit type declarations
- **Data types:**
  - `int` - Integer numbers
  - `float` - Floating-point numbers
  - `bool` - Boolean values (true/false)
  - `sequence` - Arrays/lists of values
  - `pattern` - Pattern definitions (future)

```mathseq
let x: int = 42
let pi: float = 3.14159
let isValid: bool = true
let numbers: sequence = [1, 2, 3, 4, 5]
```

### 2. **Basic Math Operations**
```mathseq
let sum: int = a + b
let difference: int = a - b
let product: int = a * b
let quotient: int = a / b
let remainder: int = a % b
```

### 3. **Control Flow**

**If-Else Statements:**
```mathseq
if condition {
    # code
} else {
    # code
}
```

**While Loops:**
```mathseq
while condition {
    # code
}
```

### 4. **Functions**
```mathseq
func functionName(param1: type1, param2: type2) -> returnType {
    # function body
    return value
}
```

### 5. **Sequence Operations**
```mathseq
# Create sequences
let seq: sequence = [1, 2, 3, 4, 5]

# Concatenate sequences
let combined: sequence = seq1 + seq2

# Access elements
let first: int = seq[0]
```

### 6. **Built-in Functions**

- `print(...)` - Print values to console
- `length(seq)` - Get sequence length
- `map(seq, func)` - Apply function to each element
- `filter(seq, func)` - Filter sequence elements
- `generate(...)` - Generate sequences

## Installation

### Prerequisites
- C++ compiler with C++17 support (g++ recommended)
- Make

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd MathScript-Compiler

# Build release version
make release

# Build debug version
make debug

# Run tests
make test

# Install system-wide (optional, requires sudo)
sudo make install
```

## Usage

### Compile a Program
```bash
./bin/mathseqc program.mathseq
```

### Command-Line Options
- `-tokens` - Print token stream
- `-ast` - Print abstract syntax tree
- `-no-opt` - Disable optimization
- `-output <file>` - Specify output file for generated code

### Example Usage
```bash
# Compile and show all phases
./bin/mathseqc test/examples/fibonacci.mathseq -ast -output fibonacci.asm

# Compile with optimization disabled
./bin/mathseqc test/examples/arithmetic.mathseq -no-opt

# Show tokens and AST
./bin/mathseqc test/examples/simple.mathseq -tokens -ast
```

## Example Programs

### Simple Program
```mathseq
func main() -> int {
    let a: int = 1
    print a
    return 0
}
```

### Fibonacci Sequence Generator
```mathseq
func fibonacci(n: int) -> sequence {
    let a: int = 0
    let b: int = 1
    let result: sequence = [a, b]
    
    while length(result) < n {
        let next: int = a + b
        result = result + [next]
        a = b
        b = next
    }
    
    return result
}

func main() -> int {
    let count: int = 10
    let fib_sequence: sequence = fibonacci(count)
    
    print "Fibonacci sequence: " fib_sequence
    
    return 0
}
```

### Arithmetic Operations
```mathseq
func calculate(a: int, b: int) -> int {
    let sum: int = a + b
    let product: int = a * b
    let difference: int = a - b
    
    if sum > product {
        return sum
    } else {
        return product
    }
}

func main() -> int {
    let x: int = 15
    let y: int = 3
    let result: int = calculate(x, y)
    
    print "Result: " result
    
    return 0
}
```

## Compiler Architecture

### Phase 1: Lexical Analysis
- Tokenizes source code
- Handles keywords, identifiers, operators, literals
- Supports comments (`#`)

### Phase 2: Syntax Analysis (Parsing)
- Builds Abstract Syntax Tree (AST)
- Recursive descent parser
- Grammar rules for expressions, statements, functions

### Phase 3: Semantic Analysis
- Type checking
- Symbol table management
- Scope analysis
- Error detection and reporting

### Phase 4: Intermediate Code Generation
- Three-address code (TAC) generation
- Temporary variable management
- Label generation for control flow

### Phase 5: Optimization
- Constant folding
- Constant propagation
- Dead code elimination
- Algebraic simplification
- Redundant assignment removal

### Phase 6: Code Output
- Final intermediate representation
- Ready for target code generation

## Project Structure

```
MathScript-Compiler/
├── include/           # Header files
│   ├── token.h       # Lexer and token definitions
│   ├── ast.h         # AST node definitions
│   ├── parser.h      # Parser interface
│   ├── semantic.h    # Semantic analyzer
│   ├── symbol_table.h # Symbol table management
│   ├── codegen.h     # Code generation
│   └── optimizer.h   # Optimization passes
├── src/              # Implementation files
│   ├── lexer.cpp
│   ├── parser.cpp
│   ├── semantic.cpp
│   ├── codegen.cpp
│   ├── optimizer.cpp
│   └── main.cpp
├── test/
│   └── examples/     # Example programs
├── Makefile
└── README.md
```

## Language Syntax Reference

### Comments
```mathseq
# This is a single-line comment
```

### Variable Declaration
```mathseq
let variableName: type = initialValue
```

### Assignment
```mathseq
variableName = newValue
```

### Function Declaration
```mathseq
func functionName(param1: type1, param2: type2) -> returnType {
    # body
    return value
}
```

### Control Flow
```mathseq
# If statement
if condition {
    # code
}

# If-else statement
if condition {
    # code
} else {
    # code
}

# While loop
while condition {
    # code
}
```

### Operators

**Arithmetic:** `+`, `-`, `*`, `/`, `%`

**Comparison:** `==`, `!=`, `<`, `>`, `<=`, `>=`

**Logical:** `and`, `or`, `not`

**Assignment:** `=`

## Error Messages

The compiler provides detailed error messages:

### Lexical Errors
- Unterminated strings
- Invalid tokens

### Parse Errors
- Missing tokens (parentheses, braces, etc.)
- Invalid syntax

### Semantic Errors
- Type mismatches
- Undefined variables/functions
- Redeclaration errors
- Invalid operations
- Return type mismatches

### Warnings
- Uninitialized variables
- Missing main function
- Functions without return statements

## Future Enhancements

- [ ] Pattern matching implementation
- [ ] More built-in mathematical functions
- [ ] Backend code generation (x86, LLVM IR)
- [ ] Advanced optimizations
- [ ] Standard library
- [ ] Module system
- [ ] Recursive pattern definitions
- [ ] Matrix operations
- [ ] Complex number support

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## License

MIT License - See LICENSE file for details

## Author

Tameema Rehman, Uzair Hussain, Huzaifa Naseer, Umar Farooq, Rayyan Solanki

## Acknowledgments


This compiler was built as an educational project to demonstrate compiler construction principles applied to domain-specific languages for mathematical computing.
