#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "symbol_table.h"
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

struct ThreeAddressCode {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
    int line;
    
    ThreeAddressCode(const std::string& op, const std::string& arg1, 
                    const std::string& arg2, const std::string& result, int line = -1)
        : op(op), arg1(arg1), arg2(arg2), result(result), line(line) {}
    
    std::string toString() const {
        if (op == "LABEL") {
            return result + ":";
        } else if (op == "GOTO") {
            return "goto " + result;
        } else if (op == "IF_FALSE") {
            return "ifFalse " + arg1 + " goto " + result;
        } else if (op == "IF") {
            return "if " + arg1 + " goto " + result;
        } else if (op == "PARAM") {
            return "param " + arg1;
        } else if (op == "CALL") {
            if (arg2.empty()) {
                return result + " = call " + arg1;
            } else {
                return result + " = call " + arg1 + ", " + arg2;
            }
        } else if (op == "RETURN") {
            if (arg1.empty()) {
                return "return";
            } else {
                return "return " + arg1;
            }
        } else if (op == "ASSIGN") {
            return result + " = " + arg1;
        } else {
            return result + " = " + arg1 + " " + op + " " + arg2;
        }
    }
};

class CodeGenerator {
private:
    std::vector<ThreeAddressCode> intermediateCode;
    SymbolTableManager symbolManager;
    
    int tempCounter;
    int labelCounter;
    
    std::string newTemp();
    std::string newLabel();
    
    void generateProgram(Program* program);
    void generateFunction(FunctionDecl* function);
    void generateStatement(Stmt* stmt);
    void generateDeclaration(DeclarationStmt* decl);
    void generateAssignment(AssignmentStmt* assign);
    void generateIfStatement(IfStmt* ifStmt);
    void generateWhileStatement(WhileStmt* whileStmt);
    void generateReturnStatement(ReturnStmt* returnStmt);
    void generateExpressionStatement(ExpressionStmt* exprStmt);
    
    std::string generateExpression(Expr* expr);
    std::string generateBinaryExpression(BinaryExpr* binaryExpr);
    std::string generateUnaryExpression(UnaryExpr* unaryExpr);
    std::string generateLiteralExpression(LiteralExpr* literalExpr);
    std::string generateVariableExpression(VariableExpr* varExpr);
    std::string generateCallExpression(CallExpr* callExpr);
    std::string generateSequenceExpression(SequenceExpr* seqExpr);
    
    std::string getOperatorTAC(TokenType op);
    
public:
    CodeGenerator() : tempCounter(0), labelCounter(0) {}
    
    std::vector<ThreeAddressCode> generate(Program* program);
    void printCode(std::ostream& out);
    const std::vector<ThreeAddressCode>& getCode() const { return intermediateCode; }
};

#endif