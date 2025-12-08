#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../include/token.h"
#include "../include/parser.h"
#include "../include/semantic.h"
#include "../include/codegen.h"
#include "../include/optimizer.h"
#include "../include/interpreter.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create file '" << filename << "'" << std::endl;
        return;
    }
    
    file << content;
    std::cout << "Output written to '" << filename << "'" << std::endl;
}

void printTokens(const std::vector<Token>& tokens) {
    std::cout << "Tokens:" << std::endl;
    std::cout << "=======" << std::endl;
    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl;
    }
    std::cout << std::endl;
}

void printAST(Program* program) {
    if (program) {
        std::cout << "Abstract Syntax Tree:" << std::endl;
        std::cout << "=====================" << std::endl;
        std::cout << program->toString() << std::endl;
    }
}

void printSemanticResults(SemanticAnalyzer& semantic) {
    const auto& warnings = semantic.getWarnings();
    if (!warnings.empty()) {
        std::cout << "Warnings:" << std::endl;
        std::cout << "=========" << std::endl;
        for (const auto& warning : warnings) {
            std::cout << "⚠️  " << warning << std::endl;
        }
        std::cout << std::endl;
    }
    
    const auto& errors = semantic.getErrors();
    if (!errors.empty()) {
        std::cout << "Errors:" << std::endl;
        std::cout << "=======" << std::endl;
        for (const auto& error : errors) {
            std::cout << "❌ " << error << std::endl;
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -tokens    Print tokens" << std::endl;
        std::cerr << "  -ast       Print AST" << std::endl;
        std::cerr << "  -no-opt    Disable optimization" << std::endl;
        std::cerr << "  -output <file> Output file for generated code" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    bool printTokensFlag = false;
    bool printASTFlag = false;
    bool enableOptimization = true;
    std::string outputFile;
    
    // Parse command line options
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-tokens") {
            printTokensFlag = true;
        } else if (arg == "-ast") {
            printASTFlag = true;
        } else if (arg == "-no-opt") {
            enableOptimization = false;
        } else if (arg == "-output" && i + 1 < argc) {
            outputFile = argv[++i];
        }
    }
    
    // Read source code
    std::string source = readFile(inputFile);
    if (source.empty()) {
        return 1;
    }
    
    std::cout << "Compiling: " << inputFile << std::endl;
    std::cout << "=========================================" << std::endl;
    
    try {
        // Phase 1: Lexical Analysis
        std::cout << "Phase 1: Lexical Analysis..." << std::endl;
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        if (printTokensFlag) {
            printTokens(tokens);
        }
        
        // Check for lexical errors
        if (!tokens.empty() && tokens.back().type == TokenType::ERROR) {
            std::cerr << "Lexical error: " << tokens.back().lexeme << std::endl;
            return 1;
        }
        
        // Phase 2: Syntax Analysis
        std::cout << "Phase 2: Syntax Analysis..." << std::endl;
        Parser parser(tokens);
        
        std::unique_ptr<Program> program;
        try {
            program = parser.parse();
        } catch (const std::exception& e) {
            std::cerr << "Parse Error: " << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "Unknown error during parsing!" << std::endl;
            return 1;
        }
        
        if (!program) {
            std::cerr << "Parsing failed! (parser returned nullptr)" << std::endl;
            return 1;
        }
        
        if (printASTFlag) {
            printAST(program.get());
        }
        
        // Phase 3: Semantic Analysis
        std::cout << "Phase 3: Semantic Analysis..." << std::endl;
        SemanticAnalyzer semantic;
        bool semanticSuccess = semantic.analyze(program.get());
        
        printSemanticResults(semantic);
        
        if (!semanticSuccess) {
            std::cerr << "Compilation failed due to semantic errors!" << std::endl;
            return 1;
        }
        
        // Phase 4: Intermediate Code Generation
        std::cout << "Phase 4: Intermediate Code Generation..." << std::endl;
        CodeGenerator codegen;
        auto intermediateCode = codegen.generate(program.get());
        
        std::cout << "Generated Intermediate Code:" << std::endl;
        std::cout << "============================" << std::endl;
        codegen.printCode(std::cout);
        std::cout << std::endl;
        
        // Phase 5: Optimization
        std::vector<ThreeAddressCode> finalCode;
        if (enableOptimization) {
            std::cout << "Phase 5: Optimization..." << std::endl;
            Optimizer optimizer(intermediateCode);
            finalCode = optimizer.optimize();
            optimizer.printOptimizedCode(std::cout);
        } else {
            finalCode = intermediateCode;
            std::cout << "Optimization skipped." << std::endl;
        }
        std::cout << std::endl;
        
        // Phase 6: Code Generation (Output)
        std::cout << "Phase 6: Final Code Output..." << std::endl;
        
        // Interpret program to capture runtime output
        Interpreter interpreter(program.get());
        auto executionResult = interpreter.run();
        
        if (executionResult.success) {
            std::cout << "Program Output:" << std::endl;
            std::cout << "===============" << std::endl;
            if (executionResult.outputLog.empty()) {
                std::cout << "(no print statements)" << std::endl;
            } else {
                for (const auto& line : executionResult.outputLog) {
                    std::cout << line << std::endl;
                }
            }
        } else {
            std::cout << "Program Output skipped: " << executionResult.errorMessage << std::endl;
        }
        std::cout << std::endl;
        
        // Generate final output
        std::stringstream finalOutput;
        finalOutput << "; MathSeq Compiler Output" << std::endl;
        finalOutput << "; Source: " << inputFile << std::endl;
        finalOutput << "; =======================" << std::endl << std::endl;
        
        for (const auto& instr : finalCode) {
            finalOutput << instr.toString() << std::endl;
        }
        
        finalOutput << std::endl;
        finalOutput << "; Program Output" << std::endl;
        finalOutput << "; --------------" << std::endl;
        if (executionResult.success) {
            if (executionResult.outputLog.empty()) {
                finalOutput << "; (no print statements)" << std::endl;
            } else {
                for (const auto& line : executionResult.outputLog) {
                    finalOutput << "; " << line << std::endl;
                }
            }
            finalOutput << "; Exit Code: " << executionResult.exitCode << std::endl;
        } else {
            finalOutput << "; Execution skipped: " << executionResult.errorMessage << std::endl;
        }
        
        // Output results
        if (!outputFile.empty()) {
            writeFile(outputFile, finalOutput.str());
        } else {
            std::cout << "Final Output:" << std::endl;
            std::cout << "=============" << std::endl;
            std::cout << finalOutput.str();
        }
        
        std::cout << std::endl << "✅ Compilation completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error during compilation: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}