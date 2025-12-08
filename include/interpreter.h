#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>

class Interpreter {
public:
    struct RuntimeValue {
        enum class Kind { VOID, INT, FLOAT, BOOL, STRING, SEQUENCE };
        
        Kind kind = Kind::VOID;
        long long intValue = 0;
        double floatValue = 0.0;
        bool boolValue = false;
        std::string stringValue;
        std::vector<RuntimeValue> sequenceValue;
        
        static RuntimeValue Void();
        static RuntimeValue FromInt(long long value);
        static RuntimeValue FromFloat(double value);
        static RuntimeValue FromBool(bool value);
        static RuntimeValue FromString(const std::string& value);
        static RuntimeValue FromSequence(const std::vector<RuntimeValue>& values);
        
        bool isNumeric() const;
        double asFloat() const;
        long long asInt() const;
        bool asBool() const;
        bool isTruthy() const;
        std::string toString() const;
    };
    
    struct ExecutionResult {
        bool success = false;
        int exitCode = 0;
        std::vector<std::string> outputLog;
        std::string errorMessage;
    };
    
    explicit Interpreter(Program* program);
    
    ExecutionResult run();
    
private:
    struct ReturnSignal : public std::exception {
        RuntimeValue value;
        explicit ReturnSignal(const RuntimeValue& value) : value(value) {}
    };
    
    Program* program;
    std::unordered_map<std::string, FunctionDecl*> functions;
    std::vector<std::unordered_map<std::string, RuntimeValue>> scopes;
    std::vector<std::string> outputLog;
    
    void initializeFunctionTable();
    void pushScope();
    void popScope();
    void define(const std::string& name, const RuntimeValue& value);
    void assign(const std::string& name, const RuntimeValue& value);
    RuntimeValue get(const std::string& name);
    
    RuntimeValue executeFunction(FunctionDecl* function, const std::vector<RuntimeValue>& args);
    void executeStatement(Stmt* stmt);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements);
    
    RuntimeValue evaluateExpression(Expr* expr);
    RuntimeValue evaluateBinary(BinaryExpr* expr);
    RuntimeValue evaluateUnary(UnaryExpr* expr);
    RuntimeValue evaluateLiteral(LiteralExpr* expr);
    RuntimeValue evaluateVariable(VariableExpr* expr);
    RuntimeValue evaluateCall(CallExpr* expr);
    RuntimeValue evaluateSequence(SequenceExpr* expr);
    
    RuntimeValue callUserFunction(const std::string& name, const std::vector<RuntimeValue>& args);
    
    RuntimeValue handlePrint(CallExpr* expr);
    RuntimeValue handleLength(CallExpr* expr);
    RuntimeValue handleGet(CallExpr* expr);
    RuntimeValue handleMap(CallExpr* expr);
    RuntimeValue handleFilter(CallExpr* expr);
    RuntimeValue handleGenerate(CallExpr* expr);
    RuntimeValue handleInput(CallExpr* expr);
    
    std::string extractFunctionName(Expr* expr);
};

#endif

