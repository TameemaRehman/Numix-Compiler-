#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>
#include "token.h"

enum class DataType {
    INT, FLOAT, BOOL, SEQUENCE, PATTERN, VOID, UNKNOWN
};

inline std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::BOOL: return "bool";
        case DataType::SEQUENCE: return "sequence";
        case DataType::PATTERN: return "pattern";
        case DataType::VOID: return "void";
        default: return "unknown";
    }
}

class ASTNode {
public:
    int line = -1;
    virtual ~ASTNode() = default;
    virtual std::string toString() const = 0;
};

class Expr : public ASTNode {
public:
    DataType type = DataType::UNKNOWN;
};

class Stmt : public ASTNode {
};

// Expressions
class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {
        this->line = op.line;
    }
    
    std::string toString() const override {
        return "BinaryExpr(" + left->toString() + " " + op.lexeme + " " + right->toString() + ")";
    }
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;
    
    UnaryExpr(Token op, std::unique_ptr<Expr> right)
        : op(op), right(std::move(right)) {
        this->line = op.line;
    }
    
    std::string toString() const override {
        return "UnaryExpr(" + op.lexeme + " " + right->toString() + ")";
    }
};

class LiteralExpr : public Expr {
public:
    Token value;
    
    LiteralExpr(Token value) : value(value) {
        this->line = value.line;
    }
    
    std::string toString() const override {
        return "LiteralExpr(" + value.lexeme + ")";
    }
};

class VariableExpr : public Expr {
public:
    Token name;
    
    VariableExpr(Token name) : name(name) {
        this->line = name.line;
    }
    
    std::string toString() const override {
        return "VariableExpr(" + name.lexeme + ")";
    }
};

class CallExpr : public Expr {
public:
    Token callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    
    CallExpr(Token callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(callee), arguments(std::move(arguments)) {
        this->line = callee.line;
    }
    
    std::string toString() const override {
        std::string argsStr = "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) argsStr += ", ";
            argsStr += arguments[i]->toString();
        }
        argsStr += ")";
        return "CallExpr(" + callee.lexeme + argsStr + ")";
    }
};

class SequenceExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    
    SequenceExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {
        if (!elements.empty()) {
            this->line = elements[0]->line;
        }
    }
    
    std::string toString() const override {
        std::string elementsStr = "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) elementsStr += ", ";
            elementsStr += elements[i]->toString();
        }
        elementsStr += "]";
        return "SequenceExpr" + elementsStr;
    }
};

// Statements
class BlockStmt : public Stmt {  // ADDED THIS CLASS
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    
    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {
        if (!statements.empty()) {
            this->line = statements[0]->line;
        }
    }
    
    std::string toString() const override {
        std::string stmtsStr = "{";
        for (const auto& stmt : statements) {
            stmtsStr += stmt->toString() + "; ";
        }
        stmtsStr += "}";
        return "BlockStmt" + stmtsStr;
    }
};

class DeclarationStmt : public Stmt {
public:
    Token name;
    DataType dataType;
    std::unique_ptr<Expr> initializer;
    
    DeclarationStmt(Token name, DataType dataType, std::unique_ptr<Expr> initializer)
        : name(name), dataType(dataType), initializer(std::move(initializer)) {
        this->line = name.line;
    }
    
    std::string toString() const override {
        return "DeclarationStmt(" + name.lexeme + ":" + dataTypeToString(dataType) + 
               " = " + (initializer ? initializer->toString() : "null") + ")";
    }
};

class AssignmentStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<Expr> value;
    
    AssignmentStmt(Token name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {
        this->line = name.line;
    }
    
    std::string toString() const override {
        return "AssignmentStmt(" + name.lexeme + " = " + value->toString() + ")";
    }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;
    
    IfStmt(std::unique_ptr<Expr> condition, 
           std::vector<std::unique_ptr<Stmt>> thenBranch,
           std::vector<std::unique_ptr<Stmt>> elseBranch)
        : condition(std::move(condition)), 
          thenBranch(std::move(thenBranch)), 
          elseBranch(std::move(elseBranch)) {
        if (this->condition) {
            this->line = this->condition->line;
        } else if (!thenBranch.empty() && thenBranch[0]) {
            this->line = thenBranch[0]->line;
        } else {
            this->line = -1;
        }
    }
    
    std::string toString() const override {
        std::string thenStr = "{";
        for (const auto& stmt : thenBranch) {
            thenStr += stmt->toString() + "; ";
        }
        thenStr += "}";
        
        std::string elseStr = "{";
        for (const auto& stmt : elseBranch) {
            elseStr += stmt->toString() + "; ";
        }
        elseStr += "}";
        
        return "IfStmt(" + condition->toString() + " then " + thenStr + " else " + elseStr + ")";
    }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
    
    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body)
        : condition(std::move(condition)), body(std::move(body)) {
        if (this->condition) {
            this->line = this->condition->line;
        } else if (!body.empty() && body[0]) {
            this->line = body[0]->line;
        } else {
            this->line = -1;
        }
    }
    
    std::string toString() const override {
        std::string bodyStr = "{";
        for (const auto& stmt : body) {
            bodyStr += stmt->toString() + "; ";
        }
        bodyStr += "}";
        return "WhileStmt(" + condition->toString() + " " + bodyStr + ")";
    }
};

class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;
    
    ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {
        if (value) {
            this->line = value->line;
        }
    }
    
    std::string toString() const override {
        return "ReturnStmt(" + (value ? value->toString() : "void") + ")";
    }
};

class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    
    ExpressionStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {
        if (expression) {
            this->line = expression->line;
        }
    }
    
    std::string toString() const override {
        return "ExpressionStmt(" + expression->toString() + ")";
    }
};

// Function Declaration
class FunctionDecl : public ASTNode {
public:
    Token name;
    std::vector<std::pair<Token, DataType>> parameters;
    DataType returnType;
    std::vector<std::unique_ptr<Stmt>> body;
    
    FunctionDecl(Token name, std::vector<std::pair<Token, DataType>> parameters,
                 DataType returnType, std::vector<std::unique_ptr<Stmt>> body)
        : name(name), parameters(std::move(parameters)), 
          returnType(returnType), body(std::move(body)) {
        this->line = name.line;
    }
    
    std::string toString() const override {
        std::string paramsStr = "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) paramsStr += ", ";
            paramsStr += parameters[i].first.lexeme + ":" + dataTypeToString(parameters[i].second);
        }
        paramsStr += ")";
        
        std::string bodyStr = "{";
        for (const auto& stmt : body) {
            bodyStr += stmt->toString() + "; ";
        }
        bodyStr += "}";
        
        return "FunctionDecl(" + name.lexeme + paramsStr + " -> " + 
               dataTypeToString(returnType) + " " + bodyStr + ")";
    }
};

// Program
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionDecl>> functions;
    
    std::string toString() const override {
        std::string result = "Program[\n";
        for (const auto& func : functions) {
            result += "  " + func->toString() + "\n";
        }
        result += "]";
        return result;
    }
};

#endif