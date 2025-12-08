#include "../include/interpreter.h"
#include <iostream>
#include <sstream>
#include <cmath>

namespace {
    Interpreter::RuntimeValue ensureNumeric(const Interpreter::RuntimeValue& value, const std::string& op) {
        if (value.kind != Interpreter::RuntimeValue::Kind::INT &&
            value.kind != Interpreter::RuntimeValue::Kind::FLOAT) {
            throw std::runtime_error("Runtime error: operator '" + op + "' requires numeric operands");
        }
        return value;
    }
}

Interpreter::RuntimeValue Interpreter::RuntimeValue::Void() {
    return RuntimeValue();
}

Interpreter::RuntimeValue Interpreter::RuntimeValue::FromInt(long long value) {
    RuntimeValue v;
    v.kind = Kind::INT;
    v.intValue = value;
    v.floatValue = static_cast<double>(value);
    return v;
}

Interpreter::RuntimeValue Interpreter::RuntimeValue::FromFloat(double value) {
    RuntimeValue v;
    v.kind = Kind::FLOAT;
    v.floatValue = value;
    v.intValue = static_cast<long long>(value);
    return v;
}

Interpreter::RuntimeValue Interpreter::RuntimeValue::FromBool(bool value) {
    RuntimeValue v;
    v.kind = Kind::BOOL;
    v.boolValue = value;
    return v;
}

Interpreter::RuntimeValue Interpreter::RuntimeValue::FromString(const std::string& value) {
    RuntimeValue v;
    v.kind = Kind::STRING;
    v.stringValue = value;
    return v;
}

Interpreter::RuntimeValue Interpreter::RuntimeValue::FromSequence(const std::vector<RuntimeValue>& values) {
    RuntimeValue v;
    v.kind = Kind::SEQUENCE;
    v.sequenceValue = values;
    return v;
}

bool Interpreter::RuntimeValue::isNumeric() const {
    return kind == Kind::INT || kind == Kind::FLOAT;
}

double Interpreter::RuntimeValue::asFloat() const {
    if (kind == Kind::FLOAT) return floatValue;
    if (kind == Kind::INT) return static_cast<double>(intValue);
    if (kind == Kind::BOOL) return boolValue ? 1.0 : 0.0;
    throw std::runtime_error("Runtime error: value is not numeric");
}

long long Interpreter::RuntimeValue::asInt() const {
    if (kind == Kind::INT) return intValue;
    if (kind == Kind::FLOAT) return static_cast<long long>(floatValue);
    if (kind == Kind::BOOL) return boolValue ? 1LL : 0LL;
    throw std::runtime_error("Runtime error: value is not an integer");
}

bool Interpreter::RuntimeValue::asBool() const {
    if (kind == Kind::BOOL) return boolValue;
    if (isNumeric()) return asFloat() != 0.0;
    return isTruthy();
}

bool Interpreter::RuntimeValue::isTruthy() const {
    switch (kind) {
        case Kind::VOID:
            return false;
        case Kind::BOOL:
            return boolValue;
        case Kind::INT:
            return intValue != 0;
        case Kind::FLOAT:
            return std::abs(floatValue) > 1e-9;
        case Kind::STRING:
            return !stringValue.empty();
        case Kind::SEQUENCE:
            return !sequenceValue.empty();
    }
    return false;
}

std::string Interpreter::RuntimeValue::toString() const {
    switch (kind) {
        case Kind::VOID:
            return "void";
        case Kind::INT:
            return std::to_string(intValue);
        case Kind::FLOAT: {
            std::ostringstream oss;
            oss << floatValue;
            return oss.str();
        }
        case Kind::BOOL:
            return boolValue ? "true" : "false";
        case Kind::STRING:
            return stringValue;
        case Kind::SEQUENCE: {
            std::string result = "[";
            for (size_t i = 0; i < sequenceValue.size(); ++i) {
                if (i > 0) result += ", ";
                result += sequenceValue[i].toString();
            }
            result += "]";
            return result;
        }
    }
    return "";
}

Interpreter::Interpreter(Program* program)
    : program(program) {
    initializeFunctionTable();
}

void Interpreter::initializeFunctionTable() {
    if (!program) return;
    for (auto& func : program->functions) {
        functions[func->name.lexeme] = func.get();
    }
}

Interpreter::ExecutionResult Interpreter::run() {
    ExecutionResult result;
    if (!program) {
        result.errorMessage = "No program loaded";
        return result;
    }
    
    auto it = functions.find("main");
    if (it == functions.end()) {
        result.errorMessage = "No 'main' function found";
        return result;
    }
    
    try {
        outputLog.clear();
        scopes.clear();
        RuntimeValue returnValue = executeFunction(it->second, {});
        result.success = true;
        if (returnValue.kind == RuntimeValue::Kind::VOID) {
            result.exitCode = 0;
        } else {
            result.exitCode = static_cast<int>(returnValue.asInt());
        }
        result.outputLog = outputLog;
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
        result.outputLog = outputLog;
    }
    
    return result;
}

void Interpreter::pushScope() {
    scopes.emplace_back();
}

void Interpreter::popScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void Interpreter::define(const std::string& name, const RuntimeValue& value) {
    if (scopes.empty()) {
        pushScope();
    }
    scopes.back()[name] = value;
}

void Interpreter::assign(const std::string& name, const RuntimeValue& value) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            found->second = value;
            return;
        }
    }
    // Implicit define if not found (defensive)
    define(name, value);
}

Interpreter::RuntimeValue Interpreter::get(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    throw std::runtime_error("Runtime error: Undefined variable '" + name + "'");
}

Interpreter::RuntimeValue Interpreter::executeFunction(FunctionDecl* function, const std::vector<RuntimeValue>& args) {
    pushScope();
    
    for (size_t i = 0; i < function->parameters.size(); ++i) {
        RuntimeValue value = RuntimeValue::Void();
        if (i < args.size()) {
            value = args[i];
        }
        define(function->parameters[i].first.lexeme, value);
    }
    
    try {
        for (auto& stmt : function->body) {
            executeStatement(stmt.get());
        }
    } catch (const ReturnSignal& signal) {
        popScope();
        return signal.value;
    } catch (...) {
        popScope();
        throw;
    }
    
    popScope();
    return RuntimeValue::Void();
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements) {
    pushScope();
    try {
        for (const auto& stmt : statements) {
            executeStatement(stmt.get());
        }
    } catch (...) {
        popScope();
        throw;
    }
    popScope();
}

void Interpreter::executeStatement(Stmt* stmt) {
    if (auto decl = dynamic_cast<DeclarationStmt*>(stmt)) {
        RuntimeValue value = RuntimeValue::Void();
        if (decl->initializer) {
            value = evaluateExpression(decl->initializer.get());
        }
        define(decl->name.lexeme, value);
    } else if (auto assignment = dynamic_cast<AssignmentStmt*>(stmt)) {
        RuntimeValue value = evaluateExpression(assignment->value.get());
        assign(assignment->name.lexeme, value);
    } else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        RuntimeValue condition = evaluateExpression(ifStmt->condition.get());
        if (condition.asBool()) {
            pushScope();
            for (auto& inner : ifStmt->thenBranch) {
                executeStatement(inner.get());
            }
            popScope();
        } else if (!ifStmt->elseBranch.empty()) {
            pushScope();
            for (auto& inner : ifStmt->elseBranch) {
                executeStatement(inner.get());
            }
            popScope();
        }
    } else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        while (evaluateExpression(whileStmt->condition.get()).asBool()) {
            pushScope();
            for (auto& inner : whileStmt->body) {
                executeStatement(inner.get());
            }
            popScope();
        }
    } else if (auto returnStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        RuntimeValue value = RuntimeValue::Void();
        if (returnStmt->value) {
            value = evaluateExpression(returnStmt->value.get());
        }
        throw ReturnSignal(value);
    } else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt)) {
        evaluateExpression(exprStmt->expression.get());
    } else if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        executeBlock(blockStmt->statements);
    }
}

Interpreter::RuntimeValue Interpreter::evaluateExpression(Expr* expr) {
    if (auto binary = dynamic_cast<BinaryExpr*>(expr)) {
        return evaluateBinary(binary);
    } else if (auto unary = dynamic_cast<UnaryExpr*>(expr)) {
        return evaluateUnary(unary);
    } else if (auto literal = dynamic_cast<LiteralExpr*>(expr)) {
        return evaluateLiteral(literal);
    } else if (auto variable = dynamic_cast<VariableExpr*>(expr)) {
        return evaluateVariable(variable);
    } else if (auto call = dynamic_cast<CallExpr*>(expr)) {
        return evaluateCall(call);
    } else if (auto sequence = dynamic_cast<SequenceExpr*>(expr)) {
        return evaluateSequence(sequence);
    }
    return RuntimeValue::Void();
}

Interpreter::RuntimeValue Interpreter::evaluateBinary(BinaryExpr* expr) {
    RuntimeValue left = evaluateExpression(expr->left.get());
    RuntimeValue right = evaluateExpression(expr->right.get());
    TokenType op = expr->op.type;
    
    auto performNumeric = [&](auto func) -> RuntimeValue {
        bool useFloat = left.kind == RuntimeValue::Kind::FLOAT || right.kind == RuntimeValue::Kind::FLOAT;
        double leftVal = left.asFloat();
        double rightVal = right.asFloat();
        double value = func(leftVal, rightVal);
        if (useFloat) {
            return RuntimeValue::FromFloat(value);
        }
        return RuntimeValue::FromInt(static_cast<long long>(value));
    };
    
    switch (op) {
        case TokenType::PLUS:
            if (left.kind == RuntimeValue::Kind::SEQUENCE && right.kind == RuntimeValue::Kind::SEQUENCE) {
                std::vector<RuntimeValue> combined = left.sequenceValue;
                combined.insert(combined.end(), right.sequenceValue.begin(), right.sequenceValue.end());
                return RuntimeValue::FromSequence(combined);
            }
            return performNumeric([](double a, double b) { return a + b; });
        case TokenType::MINUS:
            return performNumeric([](double a, double b) { return a - b; });
        case TokenType::MULTIPLY:
            return performNumeric([](double a, double b) { return a * b; });
        case TokenType::DIVIDE:
            return performNumeric([](double a, double b) { return a / b; });
        case TokenType::MODULO: {
            long long l = left.asInt();
            long long r = right.asInt();
            if (r == 0) {
                throw std::runtime_error("Runtime error: division by zero");
            }
            return RuntimeValue::FromInt(l % r);
        }
        case TokenType::EQUALS:
            return RuntimeValue::FromBool(left.toString() == right.toString());
        case TokenType::NOT_EQUALS:
            return RuntimeValue::FromBool(left.toString() != right.toString());
        case TokenType::LESS:
            return RuntimeValue::FromBool(left.asFloat() < right.asFloat());
        case TokenType::LESS_EQUAL:
            return RuntimeValue::FromBool(left.asFloat() <= right.asFloat());
        case TokenType::GREATER:
            return RuntimeValue::FromBool(left.asFloat() > right.asFloat());
        case TokenType::GREATER_EQUAL:
            return RuntimeValue::FromBool(left.asFloat() >= right.asFloat());
        case TokenType::AND:
            return RuntimeValue::FromBool(left.asBool() && right.asBool());
        case TokenType::OR:
            return RuntimeValue::FromBool(left.asBool() || right.asBool());
        default:
            break;
    }
    
    return RuntimeValue::Void();
}

Interpreter::RuntimeValue Interpreter::evaluateUnary(UnaryExpr* expr) {
    RuntimeValue value = evaluateExpression(expr->right.get());
    
    switch (expr->op.type) {
        case TokenType::MINUS:
            ensureNumeric(value, "-");
            if (value.kind == RuntimeValue::Kind::FLOAT) {
                return RuntimeValue::FromFloat(-value.floatValue);
            }
            return RuntimeValue::FromInt(-value.intValue);
        case TokenType::NOT:
            return RuntimeValue::FromBool(!value.asBool());
        default:
            break;
    }
    
    return RuntimeValue::Void();
}

Interpreter::RuntimeValue Interpreter::evaluateLiteral(LiteralExpr* expr) {
    switch (expr->value.type) {
        case TokenType::NUMBER:
            return RuntimeValue::FromInt(std::stoll(expr->value.lexeme));
        case TokenType::FLOAT:
            return RuntimeValue::FromFloat(std::stod(expr->value.lexeme));
        case TokenType::TRUE:
            return RuntimeValue::FromBool(true);
        case TokenType::FALSE:
            return RuntimeValue::FromBool(false);
        case TokenType::STRING:
            return RuntimeValue::FromString(expr->value.lexeme);
        default:
            break;
    }
    return RuntimeValue::Void();
}

Interpreter::RuntimeValue Interpreter::evaluateVariable(VariableExpr* expr) {
    return get(expr->name.lexeme);
}

Interpreter::RuntimeValue Interpreter::evaluateCall(CallExpr* expr) {
    const std::string funcName = expr->callee.lexeme;
    
    if (funcName == "print") {
        return handlePrint(expr);
    } else if (funcName == "length") {
        return handleLength(expr);
    } else if (funcName == "get") {
        return handleGet(expr);
    } else if (funcName == "map") {
        return handleMap(expr);
    } else if (funcName == "filter") {
        return handleFilter(expr);
    } else if (funcName == "generate") {
        return handleGenerate(expr);
    } else if (funcName == "input") {
        return handleInput(expr);
    }
    
    std::vector<RuntimeValue> args;
    args.reserve(expr->arguments.size());
    for (auto& arg : expr->arguments) {
        args.push_back(evaluateExpression(arg.get()));
    }
    return callUserFunction(funcName, args);
}

Interpreter::RuntimeValue Interpreter::evaluateSequence(SequenceExpr* expr) {
    std::vector<RuntimeValue> values;
    values.reserve(expr->elements.size());
    for (auto& element : expr->elements) {
        values.push_back(evaluateExpression(element.get()));
    }
    return RuntimeValue::FromSequence(values);
}

Interpreter::RuntimeValue Interpreter::callUserFunction(const std::string& name, const std::vector<RuntimeValue>& args) {
    auto it = functions.find(name);
    if (it == functions.end()) {
        throw std::runtime_error("Runtime error: Undefined function '" + name + "'");
    }
    return executeFunction(it->second, args);
}

Interpreter::RuntimeValue Interpreter::handlePrint(CallExpr* expr) {
    std::string line;
    for (size_t i = 0; i < expr->arguments.size(); ++i) {
        RuntimeValue value = evaluateExpression(expr->arguments[i].get());
        if (i > 0) line += " ";
        line += value.toString();
    }
    outputLog.push_back(line);
    return RuntimeValue::Void();
}

Interpreter::RuntimeValue Interpreter::handleLength(CallExpr* expr) {
    if (expr->arguments.size() != 1) {
        throw std::runtime_error("Runtime error: length expects 1 argument");
    }
    RuntimeValue sequence = evaluateExpression(expr->arguments[0].get());
    if (sequence.kind != RuntimeValue::Kind::SEQUENCE) {
        throw std::runtime_error("Runtime error: length expects a sequence");
    }
    return RuntimeValue::FromInt(static_cast<long long>(sequence.sequenceValue.size()));
}

Interpreter::RuntimeValue Interpreter::handleGet(CallExpr* expr) {
    if (expr->arguments.size() != 2) {
        throw std::runtime_error("Runtime error: get expects 2 arguments");
    }
    RuntimeValue sequence = evaluateExpression(expr->arguments[0].get());
    RuntimeValue index = evaluateExpression(expr->arguments[1].get());
    
    if (sequence.kind != RuntimeValue::Kind::SEQUENCE) {
        throw std::runtime_error("Runtime error: get expects a sequence as the first argument");
    }
    
    long long idx = index.asInt();
    if (idx < 0 || static_cast<size_t>(idx) >= sequence.sequenceValue.size()) {
        throw std::runtime_error("Runtime error: sequence index out of range");
    }
    
    return sequence.sequenceValue[static_cast<size_t>(idx)];
}

Interpreter::RuntimeValue Interpreter::handleMap(CallExpr* expr) {
    if (expr->arguments.size() != 2) {
        throw std::runtime_error("Runtime error: map expects 2 arguments");
    }
    
    RuntimeValue sequence = evaluateExpression(expr->arguments[0].get());
    if (sequence.kind != RuntimeValue::Kind::SEQUENCE) {
        throw std::runtime_error("Runtime error: map expects a sequence as the first argument");
    }
    
    std::string mapperName = extractFunctionName(expr->arguments[1].get());
    std::vector<RuntimeValue> result;
    result.reserve(sequence.sequenceValue.size());
    
    for (const auto& item : sequence.sequenceValue) {
        result.push_back(callUserFunction(mapperName, {item}));
    }
    
    return RuntimeValue::FromSequence(result);
}

Interpreter::RuntimeValue Interpreter::handleFilter(CallExpr* expr) {
    if (expr->arguments.size() != 2) {
        throw std::runtime_error("Runtime error: filter expects 2 arguments");
    }
    
    RuntimeValue sequence = evaluateExpression(expr->arguments[0].get());
    if (sequence.kind != RuntimeValue::Kind::SEQUENCE) {
        throw std::runtime_error("Runtime error: filter expects a sequence as the first argument");
    }
    
    std::string predicateName = extractFunctionName(expr->arguments[1].get());
    std::vector<RuntimeValue> result;
    
    for (const auto& item : sequence.sequenceValue) {
        RuntimeValue keep = callUserFunction(predicateName, {item});
        if (keep.asBool()) {
            result.push_back(item);
        }
    }
    
    return RuntimeValue::FromSequence(result);
}

Interpreter::RuntimeValue Interpreter::handleGenerate(CallExpr* expr) {
    // Placeholder: return empty sequence if generate is invoked
    std::vector<RuntimeValue> args;
    args.reserve(expr->arguments.size());
    for (auto& arg : expr->arguments) {
        args.push_back(evaluateExpression(arg.get()));
    }
    (void)args;
    return RuntimeValue::FromSequence({});
}

std::string Interpreter::extractFunctionName(Expr* expr) {
    if (auto variable = dynamic_cast<VariableExpr*>(expr)) {
        return variable->name.lexeme;
    }
    throw std::runtime_error("Runtime error: expected function identifier");
}

Interpreter::RuntimeValue Interpreter::handleInput(CallExpr* expr) {
    if (expr->arguments.size() > 1) {
        throw std::runtime_error("Runtime error: input expects at most 1 argument");
    }
    
    if (!expr->arguments.empty()) {
        RuntimeValue prompt = evaluateExpression(expr->arguments[0].get());
        std::string promptText = prompt.toString();
        if (!promptText.empty()) {
            std::cout << promptText << " ";
        }
    }
    std::cout << "> " << std::flush;
    
    std::string line;
    if (!std::getline(std::cin, line)) {
        return RuntimeValue::FromInt(0);
    }
    
    auto trim = [](std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) {
            s.clear();
            return;
        }
        s = s.substr(start, end - start + 1);
    };
    
    trim(line);
    if (line.empty()) {
        return RuntimeValue::FromInt(0);
    }
    
    try {
        size_t idx = 0;
        long long value = std::stoll(line, &idx, 10);
        if (idx == line.length()) {
            return RuntimeValue::FromInt(value);
        }
    } catch (...) {
        // Fall through to floating-point parsing
    }
    
    try {
        double value = std::stod(line);
        return RuntimeValue::FromInt(static_cast<long long>(value));
    } catch (...) {
        return RuntimeValue::FromInt(0);
    }
}

