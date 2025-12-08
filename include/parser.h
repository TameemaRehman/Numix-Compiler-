#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    int current;
    
    Token advance();
    Token peek();
    Token peekNext();
    Token previous();
    bool check(TokenType type);
    bool match(TokenType type);
    bool match(const std::vector<TokenType>& types);
    bool isAtEnd();
    Token consume(TokenType type, const std::string& message);
    
    DataType tokenToDataType(Token token);
    
    // Grammar rules
    std::unique_ptr<Program> parseProgram();
    std::unique_ptr<FunctionDecl> parseFunction();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseDeclaration();
    std::unique_ptr<Stmt> parseAssignment();
    std::unique_ptr<Stmt> parsePrintStatement();
    std::unique_ptr<Stmt> parseIfStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<Stmt> parseReturnStatement();
    std::unique_ptr<Stmt> parseExpressionStatement();
    std::vector<std::unique_ptr<Stmt>> parseBlock();
    
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseFactor();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseSequence();
    
    std::vector<std::pair<Token, DataType>> parseParameters();
    std::vector<std::unique_ptr<Expr>> parseArguments();
    
    void synchronize();
    
public:
    Parser(std::vector<Token> tokens);
    std::unique_ptr<Program> parse();
};

#endif