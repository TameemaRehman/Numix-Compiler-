#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <stdexcept>

class SemanticError : public std::runtime_error {
public:
    SemanticError(const std::string& message) : std::runtime_error(message) {}
};

class SemanticAnalyzer {
private:
    SymbolTableManager symbolManager;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    
    DataType currentFunctionReturnType;
    bool inFunction;
    bool hasReturnStatement;
    
    void addError(const std::string& message, int line = -1);
    void addWarning(const std::string& message, int line = -1);
    
    // Analysis methods
    void analyzeProgram(Program* program);
    void analyzeFunction(FunctionDecl* function);
    void analyzeStatement(Stmt* stmt);
    void analyzeDeclaration(DeclarationStmt* decl);
    void analyzeAssignment(AssignmentStmt* assign);
    void analyzeIfStatement(IfStmt* ifStmt);
    void analyzeWhileStatement(WhileStmt* whileStmt);
    void analyzeReturnStatement(ReturnStmt* returnStmt);
    void analyzeExpressionStatement(ExpressionStmt* exprStmt);
    
    DataType analyzeExpression(Expr* expr);
    DataType analyzeBinaryExpression(BinaryExpr* binaryExpr);
    DataType analyzeUnaryExpression(UnaryExpr* unaryExpr);
    DataType analyzeLiteralExpression(LiteralExpr* literalExpr);
    DataType analyzeVariableExpression(VariableExpr* varExpr);
    DataType analyzeCallExpression(CallExpr* callExpr);
    DataType analyzeSequenceExpression(SequenceExpr* seqExpr);
    
    bool isTypeCompatible(DataType left, DataType right, TokenType op);
    bool isValidOperation(DataType type, TokenType op);
    bool isNumericType(DataType type);
    bool canCoerce(DataType from, DataType to);
    
    void checkMainFunction(Program* program);
    
public:
    SemanticAnalyzer() : currentFunctionReturnType(DataType::VOID), 
                        inFunction(false), hasReturnStatement(false) {}
    
    bool analyze(Program* program);
    const std::vector<std::string>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }
    void clear();
};

#endif