// Phase 1 of the Compiler

#include "../include/token.h"
#include <cctype>
#include <iostream>

std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"func", TokenType::FUNC},
    {"let", TokenType::LET},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
    {"int", TokenType::INT},
    {"float", TokenType::FLOAT_TYPE},
    {"bool", TokenType::BOOL},
    {"sequence", TokenType::SEQUENCE},
    {"pattern", TokenType::PATTERN}
};

Lexer::Lexer(const std::string& source) 
    : source(source), start(0), current(0), line(1), column(1) {}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    current++;
    column++;
    return source[current - 1];
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}


char Lexer::peekNext() {
    if (static_cast<size_t>(current + 1) >= source.length()) return '\0';
    return source[current + 1];
}

bool Lexer::isAtEnd() {
    return static_cast<size_t>(current) >= source.length();
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    column++;
    return true;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            line++;
            column = 1;
            advance();
        } else if (c == '#') {
            // Skip comments until end of line
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::stringLiteral() {
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            line++;
            column = 1;
        }
        advance();
    }
    
    if (isAtEnd()) {
        return Token(TokenType::ERROR, "Unterminated string", line, column);
    }
    
    advance(); // Consume closing "
    std::string value = source.substr(start + 1, current - start - 2);
    return Token(TokenType::STRING, value, line, column);
}

Token Lexer::number() {
    bool isFloat = false;
    
    while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.')) {
        if (peek() == '.') {
            if (isFloat) break; // Multiple decimal points
            isFloat = true;
        }
        advance();
    }
    
    std::string value = source.substr(start, current - start);
    return Token(isFloat ? TokenType::FLOAT : TokenType::NUMBER, value, line, column);
}

TokenType Lexer::checkKeyword(int start, int length, const char* rest, TokenType type) {
    if (current - this->start == start + length &&
        source.compare(this->start + start, length, rest) == 0) {
        return type;
    }
    return TokenType::IDENTIFIER;
}

Token Lexer::identifier() {
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        advance();
    }
    
    std::string text = source.substr(start, current - start);
    TokenType type = TokenType::IDENTIFIER;
    
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        type = it->second;
    }
    
    return Token(type, text, line, column);
}

Token Lexer::nextToken() {
    skipWhitespace();
    start = current;
    
    if (isAtEnd()) {
        return Token(TokenType::END_OF_FILE, "", line, column);
    }
    
    char c = advance();
    
    if (std::isalpha(c) || c == '_') {
        return identifier();
    }
    
    if (std::isdigit(c)) {
        return number();
    }
    
    switch (c) {
        case '"': return stringLiteral();
        case '(': return Token(TokenType::LPAREN, "(", line, column);
        case ')': return Token(TokenType::RPAREN, ")", line, column);
        case '{': return Token(TokenType::LBRACE, "{", line, column);
        case '}': return Token(TokenType::RBRACE, "}", line, column);
        case '[': return Token(TokenType::LBRACKET, "[", line, column);
        case ']': return Token(TokenType::RBRACKET, "]", line, column);
        case ',': return Token(TokenType::COMMA, ",", line, column);
        case ':': return Token(TokenType::COLON, ":", line, column);
        case ';': return Token(TokenType::SEMICOLON, ";", line, column);
        case '+': return Token(TokenType::PLUS, "+", line, column);
        case '-': 
            if (match('>')) {
                return Token(TokenType::ARROW, "->", line, column);
            }
            return Token(TokenType::MINUS, "-", line, column);
        case '*': return Token(TokenType::MULTIPLY, "*", line, column);
        case '/': return Token(TokenType::DIVIDE, "/", line, column);
        case '%': return Token(TokenType::MODULO, "%", line, column);
        case '=': 
            if (match('=')) {
                return Token(TokenType::EQUALS, "==", line, column);
            }
            return Token(TokenType::ASSIGN, "=", line, column);
        case '!':
            if (match('=')) {
                return Token(TokenType::NOT_EQUALS, "!=", line, column);
            }
            break;
        case '<':
            if (match('=')) {
                return Token(TokenType::LESS_EQUAL, "<=", line, column);
            }
            return Token(TokenType::LESS, "<", line, column);
        case '>':
            if (match('=')) {
                return Token(TokenType::GREATER_EQUAL, ">=", line, column);
            }
            return Token(TokenType::GREATER, ">", line, column);
    }
    
    return Token(TokenType::ERROR, std::string(1, c), line, column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = nextToken();
    while (token.type != TokenType::END_OF_FILE && token.type != TokenType::ERROR) {
        tokens.push_back(token);
        token = nextToken();
    }
    if (token.type == TokenType::ERROR) {
        tokens.push_back(token);
    } else {
        // Always add END_OF_FILE token so parser knows when to stop
        tokens.push_back(token);
    }
    return tokens;

}
