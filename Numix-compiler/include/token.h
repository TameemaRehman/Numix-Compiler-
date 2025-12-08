#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <unordered_map>
#include <vector>

enum class TokenType {
    // Literals
    IDENTIFIER, NUMBER, FLOAT, STRING,
    
    // Keywords
    FUNC, LET, IF, ELSE, WHILE, RETURN, 
    TRUE, FALSE, AND, OR, NOT,
    INT, FLOAT_TYPE, BOOL, SEQUENCE, PATTERN,
    
    // Operators
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    ASSIGN, EQUALS, NOT_EQUALS,
    LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
    
    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, COLON, SEMICOLON, ARROW,
    
    // Special
    END_OF_FILE, ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    
    Token(TokenType t, const std::string& l, int ln = 1, int col = 1) 
        : type(t), lexeme(l), line(ln), column(col) {}
    
    std::string toString() const {
        return "Token(" + std::to_string(static_cast<int>(type)) + 
               ", '" + lexeme + "', line=" + std::to_string(line) + 
               ", col=" + std::to_string(column) + ")";
    }
};

class Lexer {
private:
    std::string source;
    int start;
    int current;
    int line;
    int column;
    
    static std::unordered_map<std::string, TokenType> keywords;
    
    char advance();
    char peek();
    char peekNext();
    bool match(char expected);
    bool isAtEnd();
    void skipWhitespace();
    Token stringLiteral();
    Token number();
    Token identifier();
    TokenType checkKeyword(int start, int length, const char* rest, TokenType type);
    
public:
    Lexer(const std::string& source);
    Token nextToken();
    std::vector<Token> tokenize();
};

#endif