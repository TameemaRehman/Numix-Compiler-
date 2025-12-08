#include "../include/parser.h"
#include <iostream>

#define DEBUG_PARSER 0

Parser::Parser(std::vector<Token> tokens) 
    : tokens(tokens), current(0) {}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::peek() {
    if (static_cast<size_t>(current) >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, "");
    }
    return tokens[current];
}

Token Parser::peekNext() {
    if (static_cast<size_t>(current + 1) >= tokens.size()) return Token(TokenType::END_OF_FILE, "");
    return tokens[current + 1];
}

Token Parser::previous() {
    if (current == 0) {
        return Token(TokenType::END_OF_FILE, "");
    }
    return tokens[current - 1];
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::isAtEnd() {
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw ParseError(message + " at line " + std::to_string(peek().line));
}

DataType Parser::tokenToDataType(Token token) {
    #if DEBUG_PARSER
    std::cout << "DEBUG: tokenToDataType - token: " << token.lexeme 
              << " type: " << static_cast<int>(token.type) << std::endl;
    #endif
    
    // Check both token type and lexeme for type keywords
    if (token.type == TokenType::INT || token.lexeme == "int") {
        #if DEBUG_PARSER
        std::cout << "DEBUG: Matched INT type" << std::endl;
        #endif
        return DataType::INT;
    }
    if (token.type == TokenType::FLOAT_TYPE || token.lexeme == "float") {
        #if DEBUG_PARSER
        std::cout << "DEBUG: Matched FLOAT type" << std::endl;
        #endif
        return DataType::FLOAT;
    }
    if (token.type == TokenType::BOOL || token.lexeme == "bool") {
        #if DEBUG_PARSER
        std::cout << "DEBUG: Matched BOOL type" << std::endl;
        #endif
        return DataType::BOOL;
    }
    if (token.type == TokenType::SEQUENCE || token.lexeme == "sequence") {
        #if DEBUG_PARSER
        std::cout << "DEBUG: Matched SEQUENCE type" << std::endl;
        #endif
        return DataType::SEQUENCE;
    }
    if (token.type == TokenType::PATTERN || token.lexeme == "pattern") {
        #if DEBUG_PARSER
        std::cout << "DEBUG: Matched PATTERN type" << std::endl;
        #endif
        return DataType::PATTERN;
    }
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: Unknown type for token: " << token.lexeme 
              << " with type: " << static_cast<int>(token.type) << std::endl;
    #endif
    return DataType::UNKNOWN;
}

std::unique_ptr<Program> Parser::parse() {
    try {
        return parseProgram();
    } catch (const ParseError& error) {
        std::cerr << "Parse Error: " << error.what() << std::endl;
        return nullptr;
    }
}

std::unique_ptr<Program> Parser::parseProgram() {
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseProgram() - starting, current: " << peek().lexeme << " at line " << peek().line << std::endl;
    #endif
    
    auto program = std::make_unique<Program>();
    
    while (!isAtEnd()) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseProgram() - loop iteration, current: " << peek().lexeme << " type: " << static_cast<int>(peek().type) << std::endl;
        #endif
        
        if (match(TokenType::FUNC)) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parseProgram() - found FUNC, parsing function" << std::endl;
            #endif
            try {
                auto func = parseFunction();
                if (func) {
                    program->functions.push_back(std::move(func));
                    #if DEBUG_PARSER
                    std::cout << "DEBUG: parseProgram() - parsed function, current: " << peek().lexeme << std::endl;
                    #endif
                } else {
                    #if DEBUG_PARSER
                    std::cout << "DEBUG: parseProgram() - parseFunction returned nullptr" << std::endl;
                    #endif
                    break;
                }
            } catch (const ParseError& e) {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parseProgram() - ParseError in parseFunction: " << e.what() << std::endl;
                #endif
                throw;
            } catch (const std::exception& e) {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parseProgram() - Exception in parseFunction: " << e.what() << std::endl;
                #endif
                throw;
            } catch (...) {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parseProgram() - Unknown exception in parseFunction!" << std::endl;
                #endif
                throw;
            }
        } else {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parseProgram() - expected FUNC, got: " << peek().lexeme << " type: " << static_cast<int>(peek().type) << std::endl;
            #endif
            throw ParseError("Expected function declaration");
        }
    }
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseProgram() - finished, parsed " << program->functions.size() << " functions" << std::endl;
    #endif
    
    return program;
}

std::unique_ptr<FunctionDecl> Parser::parseFunction() {
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseFunction() - starting, current: " << peek().lexeme << " at line " << peek().line << std::endl;
    #endif
    
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseFunction() - got name: " << name.lexeme << std::endl;
    #endif
    
    consume(TokenType::LPAREN, "Expected '(' after function name");
    
    auto parameters = parseParameters();
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseFunction() - parsed " << parameters.size() << " parameters" << std::endl;
    #endif
    
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    consume(TokenType::ARROW, "Expected '->' after function parameters");
    
    // Parse return type - can be an identifier OR a type keyword
    std::vector<TokenType> validTypeTokens = {
        TokenType::INT, TokenType::FLOAT_TYPE, TokenType::BOOL, 
        TokenType::SEQUENCE, TokenType::PATTERN, TokenType::IDENTIFIER
    };
    
    bool isValidType = false;
    for (TokenType validType : validTypeTokens) {
        if (check(validType)) {
            isValidType = true;
            break;
        }
    }
    
    if (!isValidType) {
        throw ParseError("Expected return type, got: " + peek().lexeme);
    }
    
    Token returnTypeToken = advance();
    DataType returnType = tokenToDataType(returnTypeToken);
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseFunction() - return type: " << static_cast<int>(returnType) << ", parsing body" << std::endl;
    #endif
    
    auto body = parseBlock();
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseFunction() - parsed body with " << body.size() << " statements, current: " << peek().lexeme << std::endl;
    #endif
    
    auto func = std::make_unique<FunctionDecl>(name, std::move(parameters), returnType, std::move(body));
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseFunction() - created FunctionDecl for " << name.lexeme << std::endl;
    #endif
    
    return func;
}

std::vector<std::pair<Token, DataType>> Parser::parseParameters() {
    std::vector<std::pair<Token, DataType>> parameters;
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseParameters() - current token: " << peek().lexeme 
              << " type: " << static_cast<int>(peek().type) << std::endl;
    #endif
    
    // If we see a closing parenthesis immediately, return empty parameters
    if (check(TokenType::RPAREN)) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: Empty parameters (immediate RPAREN)" << std::endl;
        #endif
        return parameters;
    }
    
    do {
        #if DEBUG_PARSER
        std::cout << "DEBUG: Starting parameter - current: " << peek().lexeme << std::endl;
        #endif
        
        // Parse parameter: name : type
        if (!check(TokenType::IDENTIFIER)) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: Expected IDENTIFIER for param name, got: " << peek().lexeme 
                      << " type: " << static_cast<int>(peek().type) << std::endl;
            #endif
            throw ParseError("Expected parameter name, got: " + peek().lexeme);
        }
        Token name = advance();
        
        #if DEBUG_PARSER
        std::cout << "DEBUG: Got param name: " << name.lexeme << std::endl;
        #endif
        
        if (!match(TokenType::COLON)) {
            throw ParseError("Expected ':' after parameter name '" + name.lexeme + "'");
        }
        
        #if DEBUG_PARSER
        std::cout << "DEBUG: After colon - current: " << peek().lexeme 
                  << " type: " << static_cast<int>(peek().type) << std::endl;
        #endif
        
        // Parse the type - can be an identifier OR a type keyword
        std::vector<TokenType> validTypeTokens = {
            TokenType::INT, TokenType::FLOAT_TYPE, TokenType::BOOL, 
            TokenType::SEQUENCE, TokenType::PATTERN, TokenType::IDENTIFIER
        };
        
        bool isValidType = false;
        for (TokenType validType : validTypeTokens) {
            if (check(validType)) {
                isValidType = true;
                break;
            }
        }
        
        if (!isValidType) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: Expected type token, got: " << peek().lexeme 
                      << " type: " << static_cast<int>(peek().type) << std::endl;
            #endif
            throw ParseError("Expected parameter type after '" + name.lexeme + ":', got: " + peek().lexeme);
        }
        
        Token typeToken = advance();
        
        #if DEBUG_PARSER
        std::cout << "DEBUG: Got type token: " << typeToken.lexeme 
                  << " type: " << static_cast<int>(typeToken.type) << std::endl;
        #endif
        
        DataType type = tokenToDataType(typeToken);
        
        #if DEBUG_PARSER
        std::cout << "DEBUG: Mapped type " << typeToken.lexeme << " to DataType: " << static_cast<int>(type) << std::endl;
        #endif
        
        if (type == DataType::UNKNOWN) {
            throw ParseError("Unknown parameter type: '" + typeToken.lexeme + "'");
        }
        
        parameters.emplace_back(name, type);
        
        #if DEBUG_PARSER
        std::cout << "DEBUG: Successfully added parameter: " << name.lexeme << ":" << typeToken.lexeme << std::endl;
        #endif
        
    } while (match(TokenType::COMMA));
    
    return parameters;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseStatement() - current: " << peek().lexeme << " type: " << static_cast<int>(peek().type) << std::endl;
    #endif
    
    if (match(TokenType::LET)) return parseDeclaration();
    if (match(TokenType::IF)) return parseIfStatement();
    if (match(TokenType::WHILE)) return parseWhileStatement();
    if (match(TokenType::RETURN)) return parseReturnStatement();
    
    // Check for print statements (print followed by one or more expressions)
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "print") {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseStatement() - detected print statement" << std::endl;
        #endif
        return parsePrintStatement();
    }
    
    // Check for assignment statements (identifier followed by =)
    if (check(TokenType::IDENTIFIER)) {
        TokenType nextType = peekNext().type;
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseStatement() - found identifier: " << peek().lexeme << ", next type: " << static_cast<int>(nextType) << std::endl;
        #endif
        if (nextType == TokenType::ASSIGN) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parseStatement() - detected assignment statement" << std::endl;
            #endif
            Token name = advance();
            return parseAssignment();
        }
    }
    
    if (match(TokenType::LBRACE)) {
        auto stmts = parseBlock();
        return std::make_unique<BlockStmt>(std::move(stmts));
    }
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseStatement() - parsing as expression statement" << std::endl;
    #endif
    return parseExpressionStatement();
}

std::unique_ptr<Stmt> Parser::parseDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");
    consume(TokenType::COLON, "Expected ':' after variable name");
    
    // Parse the type - can be an identifier OR a type keyword
    std::vector<TokenType> validTypeTokens = {
        TokenType::INT, TokenType::FLOAT_TYPE, TokenType::BOOL, 
        TokenType::SEQUENCE, TokenType::PATTERN, TokenType::IDENTIFIER
    };
    
    bool isValidType = false;
    for (TokenType validType : validTypeTokens) {
        if (check(validType)) {
            isValidType = true;
            break;
        }
    }
    
    if (!isValidType) {
        throw ParseError("Expected variable type after '" + name.lexeme + ":', got: " + peek().lexeme);
    }
    
    Token typeToken = advance();
    DataType type = tokenToDataType(typeToken);
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = parseExpression();
    }
    
    // Optional semicolon - try to match it, but don't require it
    match(TokenType::SEMICOLON);
    
    return std::make_unique<DeclarationStmt>(name, type, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::parseAssignment() {
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseAssignment() - starting, name: " << previous().lexeme << std::endl;
    #endif
    
    Token name = previous(); // We already consumed the identifier
    consume(TokenType::ASSIGN, "Expected '=' after variable name");
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseAssignment() - parsed =, current: " << peek().lexeme << std::endl;
    #endif
    
    auto value = parseExpression();
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseAssignment() - parsed expression, current: " << peek().lexeme << std::endl;
    #endif
    
    // Optional semicolon
    match(TokenType::SEMICOLON);
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseAssignment() - creating AssignmentStmt" << std::endl;
    #endif
    
    try {
        auto assignStmt = std::make_unique<AssignmentStmt>(name, std::move(value));
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseAssignment() - successfully created AssignmentStmt" << std::endl;
        #endif
        return assignStmt;
    } catch (const std::exception& e) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseAssignment() - Exception creating AssignmentStmt: " << e.what() << std::endl;
        #endif
        throw;
    } catch (...) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseAssignment() - Unknown exception creating AssignmentStmt!" << std::endl;
        #endif
        throw;
    }
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseIfStatement() - current: " << peek().lexeme << " at line " << peek().line << std::endl;
    #endif
    
    std::unique_ptr<Expr> condition = parseExpression();
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseIfStatement() - parsed condition, current: " << peek().lexeme << std::endl;
    #endif
    
    auto thenBranch = parseBlock();
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseIfStatement() - parsed then branch, current: " << peek().lexeme << std::endl;
    #endif
    
    std::vector<std::unique_ptr<Stmt>> elseBranch;
    if (match(TokenType::ELSE)) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseIfStatement() - found else clause" << std::endl;
        #endif
        elseBranch = parseBlock();
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseIfStatement() - parsed else branch with " << elseBranch.size() << " statements" << std::endl;
        #endif
    }
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseIfStatement() - creating IfStmt, thenBranch has " << thenBranch.size() << " statements, elseBranch has " << elseBranch.size() << " statements" << std::endl;
    std::cout << "DEBUG: parseIfStatement() - current token before creating IfStmt: " << peek().lexeme << std::endl;
    std::cout << "DEBUG: parseIfStatement() - condition is " << (condition ? "valid" : "null") << std::endl;
    if (condition) {
        std::cout << "DEBUG: parseIfStatement() - condition line: " << condition->line << std::endl;
    }
    #endif
    
    // Verify condition is valid
    if (!condition) {
        throw ParseError("If statement condition is null");
    }
    
    try {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseIfStatement() - calling make_unique<IfStmt>" << std::endl;
        #endif
        
        // Create IfStmt by moving the condition and branches
        auto ifStmt = std::make_unique<IfStmt>(
            std::move(condition), 
            std::move(thenBranch), 
            std::move(elseBranch)
        );
        
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseIfStatement() - successfully created IfStmt" << std::endl;
        #endif
        
        return ifStmt;
    } catch (const std::bad_alloc& e) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseIfStatement() - bad_alloc creating IfStmt: " << e.what() << std::endl;
        #endif
        throw;
    } catch (const std::exception& e) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseIfStatement() - Exception creating IfStmt: " << e.what() << std::endl;
        #endif
        throw;
    } catch (...) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseIfStatement() - Unknown exception creating IfStmt!" << std::endl;
        #endif
        throw;
    }
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseWhileStatement() - current: " << peek().lexeme << " at line " << peek().line << std::endl;
    #endif
    
    std::unique_ptr<Expr> condition = parseExpression();
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseWhileStatement() - parsed condition, current: " << peek().lexeme << std::endl;
    #endif
    
    auto body = parseBlock();
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseWhileStatement() - parsed body with " << body.size() << " statements, current: " << peek().lexeme << std::endl;
    std::cout << "DEBUG: parseWhileStatement() - condition is " << (condition ? "valid" : "null") << std::endl;
    if (condition) {
        std::cout << "DEBUG: parseWhileStatement() - condition line: " << condition->line << std::endl;
    }
    std::cout << "DEBUG: parseWhileStatement() - creating WhileStmt" << std::endl;
    #endif
    
    // Verify condition is valid
    if (!condition) {
        throw ParseError("While statement condition is null");
    }
    
    try {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseWhileStatement() - calling make_unique<WhileStmt>" << std::endl;
        #endif
        
        auto whileStmt = std::make_unique<WhileStmt>(std::move(condition), std::move(body));
        
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseWhileStatement() - successfully created WhileStmt" << std::endl;
        #endif
        
        return whileStmt;
    } catch (const std::bad_alloc& e) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseWhileStatement() - bad_alloc creating WhileStmt: " << e.what() << std::endl;
        #endif
        throw;
    } catch (const std::exception& e) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseWhileStatement() - Exception creating WhileStmt: " << e.what() << std::endl;
        #endif
        throw;
    } catch (...) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseWhileStatement() - Unknown exception creating WhileStmt!" << std::endl;
        #endif
        throw;
    }
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }
    
    // Optional semicolon
    match(TokenType::SEMICOLON);
    
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::parsePrintStatement() {
    #if DEBUG_PARSER
    std::cout << "DEBUG: parsePrintStatement() - current: " << peek().lexeme << " at line " << peek().line << std::endl;
    #endif
    
    // Consume the 'print' identifier
    Token printToken = advance();
    
    // Parse all arguments until we hit a statement terminator
    std::vector<std::unique_ptr<Expr>> arguments;
    
    // Parse arguments one by one until we hit a terminator
    // In print statements, arguments can be: literals, identifiers, function calls, sequences
    // But NOT binary operations (we stop at operators)
    while (true) {
        // Check if we've reached the end of the print statement
        if (isAtEnd()) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parsePrintStatement - reached end of file" << std::endl;
            #endif
            break;
        }
        
        TokenType nextType = peek().type;
        
        // Stop at statement terminators
        if (nextType == TokenType::SEMICOLON || 
            nextType == TokenType::RBRACE ||
            nextType == TokenType::FUNC ||
            nextType == TokenType::LET ||
            nextType == TokenType::IF ||
            nextType == TokenType::WHILE ||
            nextType == TokenType::RETURN ||
            nextType == TokenType::ELSE ||
            nextType == TokenType::END_OF_FILE) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parsePrintStatement - hit terminator: " << static_cast<int>(nextType) << std::endl;
            #endif
            break;
        }
        
        // Try to parse a primary expression (literal, identifier, function call, sequence, or parenthesized)
        // For print statements, we parse primary expressions only - not full expressions with operators
        // This allows: print "text" var func() [1,2,3]
        // But stops at operators that might indicate a new statement
        try {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parsePrintStatement - parsing argument at: " << peek().lexeme << " type: " << static_cast<int>(peek().type) << std::endl;
            #endif
            
            // Save position before parsing (unused for now, but kept for potential rollback)
            // int beforeParse = current;
            
            // Parse just a primary expression, not a full expression
            auto expr = parsePrimary();
            
            if (!expr) {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parsePrintStatement - failed to parse primary expression" << std::endl;
                #endif
                break;
            }
            
            #if DEBUG_PARSER
            std::cout << "DEBUG: parsePrintStatement - successfully parsed primary, current: " << peek().lexeme << std::endl;
            #endif
            
            arguments.push_back(std::move(expr));
            
            // After parsing, check if we should stop
            if (isAtEnd()) {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parsePrintStatement - reached end after parsing" << std::endl;
                #endif
                break;
            }
            
            nextType = peek().type;
            
            // Stop at statement terminators
            if (nextType == TokenType::SEMICOLON || 
                nextType == TokenType::RBRACE ||
                nextType == TokenType::FUNC ||
                nextType == TokenType::LET ||
                nextType == TokenType::IF ||
                nextType == TokenType::WHILE ||
                nextType == TokenType::RETURN ||
                nextType == TokenType::ELSE ||
                nextType == TokenType::END_OF_FILE) {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parsePrintStatement - hit terminator after parsing: " << static_cast<int>(nextType) << std::endl;
                #endif
                break;
            }
            
            // Stop if we see another "print" identifier - this starts a new print statement
            if (nextType == TokenType::IDENTIFIER && peek().lexeme == "print") {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parsePrintStatement - hit another print statement, stopping" << std::endl;
                #endif
                break;
            }
            
            // Stop if we see an assignment operator - this starts a new statement
            if (nextType == TokenType::ASSIGN) {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parsePrintStatement - hit assignment operator, stopping" << std::endl;
                #endif
                break;
            }
            
            // Continue parsing if we see another primary expression start
            // (identifier, literal, function call start, sequence start, etc.)
            // Only continue if the next token looks like it could be a print argument
            bool isPrimaryStart = (nextType == TokenType::IDENTIFIER ||
                                   nextType == TokenType::STRING ||
                                   nextType == TokenType::NUMBER ||
                                   nextType == TokenType::FLOAT ||
                                   nextType == TokenType::TRUE ||
                                   nextType == TokenType::FALSE ||
                                   nextType == TokenType::LPAREN ||
                                   nextType == TokenType::LBRACKET);
            
            if (!isPrimaryStart) {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parsePrintStatement - next token is not a primary expression start, stopping" << std::endl;
                #endif
                break;
            }
            
            // Continue loop to parse next argument
            #if DEBUG_PARSER
            std::cout << "DEBUG: parsePrintStatement - continuing to parse next argument" << std::endl;
            #endif
            
        } catch (const ParseError& e) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parsePrintStatement - ParseError: " << e.what() << " at line " << peek().line << std::endl;
            #endif
            // If parsing failed, stop - this is expected when we've consumed all arguments
            break;
        } catch (const std::exception& e) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parsePrintStatement - Exception: " << e.what() << std::endl;
            #endif
            // Re-throw unexpected exceptions
            throw;
        } catch (...) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parsePrintStatement - Unknown exception!" << std::endl;
            #endif
            throw;
        }
    }
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parsePrintStatement - parsed " << arguments.size() << " arguments" << std::endl;
    #endif
    
    // Create a function call expression for print
    auto callExpr = std::make_unique<CallExpr>(printToken, std::move(arguments));
    
    // Optional semicolon
    match(TokenType::SEMICOLON);
    
    return std::make_unique<ExpressionStmt>(std::move(callExpr));
}

std::unique_ptr<Stmt> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    
    // Optional semicolon
    match(TokenType::SEMICOLON);
    
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::vector<std::unique_ptr<Stmt>> Parser::parseBlock() {
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseBlock() - current: " << peek().lexeme << " at line " << peek().line << std::endl;
    #endif
    
    consume(TokenType::LBRACE, "Expected '{' before block");
    
    std::vector<std::unique_ptr<Stmt>> statements;
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseBlock() - entered block, current: " << peek().lexeme << std::endl;
    #endif
    
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        #if DEBUG_PARSER
        std::cout << "DEBUG: parseBlock() - parsing statement, current: " << peek().lexeme << " type: " << static_cast<int>(peek().type) << std::endl;
        #endif
        
        try {
            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
                #if DEBUG_PARSER
                std::cout << "DEBUG: parseBlock() - parsed statement, current: " << peek().lexeme << std::endl;
                #endif
            } else {
                #if DEBUG_PARSER
                std::cout << "DEBUG: parseBlock() - parseStatement returned nullptr" << std::endl;
                #endif
                break;
            }
        } catch (const ParseError& e) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parseBlock() - ParseError: " << e.what() << std::endl;
            #endif
            throw;
        } catch (const std::exception& e) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parseBlock() - Exception: " << e.what() << std::endl;
            #endif
            throw;
        } catch (...) {
            #if DEBUG_PARSER
            std::cout << "DEBUG: parseBlock() - Unknown exception!" << std::endl;
            #endif
            throw;
        }
    }
    
    #if DEBUG_PARSER
    std::cout << "DEBUG: parseBlock() - exiting block, current: " << peek().lexeme << ", parsed " << statements.size() << " statements" << std::endl;
    #endif
    
    consume(TokenType::RBRACE, "Expected '}' after block");
    return statements;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseLogicalOr();
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (match(TokenType::OR)) {
        Token op = previous();
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (match(TokenType::AND)) {
        Token op = previous();
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (match({TokenType::EQUALS, TokenType::NOT_EQUALS})) {
        Token op = previous();
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match({TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL})) {
        Token op = previous();
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match({TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MODULO})) {
        Token op = previous();
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match({TokenType::MINUS, TokenType::NOT})) {
        Token op = previous();
        auto right = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    
    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenType::TRUE)) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match(TokenType::FALSE)) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match({TokenType::NUMBER, TokenType::FLOAT, TokenType::STRING})) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match(TokenType::IDENTIFIER)) {
        Token name = previous();
        
        if (match(TokenType::LPAREN)) {
            auto arguments = parseArguments();
            return std::make_unique<CallExpr>(name, std::move(arguments));
        }
        
        // Check for array indexing
        if (match(TokenType::LBRACKET)) {
            auto index = parseExpression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            // For now, treat this as a function call to simplify
            std::vector<std::unique_ptr<Expr>> args;
            args.push_back(std::make_unique<VariableExpr>(name));
            args.push_back(std::move(index));
            return std::make_unique<CallExpr>(Token(TokenType::IDENTIFIER, "get", name.line, name.column), std::move(args));
        }
        
        return std::make_unique<VariableExpr>(name);
    }
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    if (match(TokenType::LBRACKET)) {
        return parseSequence();
    }
    
    throw ParseError("Expected expression");
}

std::unique_ptr<Expr> Parser::parseSequence() {
    std::vector<std::unique_ptr<Expr>> elements;
    
    // We already consumed the LBRACKET in parsePrimary()
    // Now parse the elements or check for empty sequence
    if (!check(TokenType::RBRACKET)) {
        do {
            elements.push_back(parseExpression());
        } while (match(TokenType::COMMA) && !check(TokenType::RBRACKET));
    }
    
    // Ensure we have a closing bracket
    if (!match(TokenType::RBRACKET)) {
        throw ParseError("Expected ']' after sequence elements");
    }
    
    return std::make_unique<SequenceExpr>(std::move(elements));
}

std::vector<std::unique_ptr<Expr>> Parser::parseArguments() {
    std::vector<std::unique_ptr<Expr>> arguments;
    
    // We already consumed the LPAREN in parsePrimary()
    // Now parse the arguments or check for empty arguments
    if (!check(TokenType::RPAREN)) {
        do {
            arguments.push_back(parseExpression());
        } while (match(TokenType::COMMA) && !check(TokenType::RPAREN));
    }
    
    // Ensure we have a closing parenthesis
    if (!match(TokenType::RPAREN)) {
        throw ParseError("Expected ')' after function arguments");
    }
    
    return arguments;
}

void Parser::synchronize() {
    advance();
    
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        
        switch (peek().type) {
            case TokenType::FUNC:
            case TokenType::LET:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        
        advance();
    }
}