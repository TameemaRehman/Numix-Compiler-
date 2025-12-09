/*semantic.cpp*/

#include "../include/semantic.h"
#include <iostream>

void SemanticAnalyzer::addError(const std::string& message, int line) {
    std::string errorMsg = "Semantic Error";
    if (line != -1) {
        errorMsg += " at line " + std::to_string(line);
    }
    errorMsg += ": " + message;
    errors.push_back(errorMsg);
}

void SemanticAnalyzer::addWarning(const std::string& message, int line) {
    std::string warningMsg = "Semantic Warning";
    if (line != -1) {
        warningMsg += " at line " + std::to_string(line);
    }
    warningMsg += ": " + message;
    warnings.push_back(warningMsg);
}

bool SemanticAnalyzer::analyze(Program* program) {
    clear();
    analyzeProgram(program);
    return errors.empty();
}

void SemanticAnalyzer::clear() {
    errors.clear();
    warnings.clear();
    symbolManager = SymbolTableManager(); // Reset symbol table
    currentFunctionReturnType = DataType::VOID;
    inFunction = false;
    hasReturnStatement = false;
}

void SemanticAnalyzer::analyzeProgram(Program* program) {
    // Add built-in functions to global scope
    symbolManager.declareSymbol("print", DataType::VOID, true);
    symbolManager.declareSymbol("generate", DataType::SEQUENCE, true);
    symbolManager.declareSymbol("map", DataType::SEQUENCE, true);
    symbolManager.declareSymbol("filter", DataType::SEQUENCE, true);
    symbolManager.declareSymbol("length", DataType::INT, true);
    symbolManager.declareSymbol("get", DataType::INT, true);  // For array indexing
    symbolManager.declareSymbol("input", DataType::INT, true);
    
    for (auto& function : program->functions) {
        if (!symbolManager.declareSymbol(function->name.lexeme, function->returnType, true)) {
            addError("Function '" + function->name.lexeme + "' already declared", function->name.line);
        }
    }
    
    for (auto& function : program->functions) {
        analyzeFunction(function.get());
    }
    
    checkMainFunction(program);
}

void SemanticAnalyzer::analyzeFunction(FunctionDecl* function) {
    symbolManager.enterScope();
    inFunction = true;
    hasReturnStatement = false;
    currentFunctionReturnType = function->returnType;

    for (const auto& param : function->parameters) {
        if (!symbolManager.declareSymbol(param.first.lexeme, param.second, true)) {
            addError("Parameter '" + param.first.lexeme + "' already declared", param.first.line);
        }
    }
    
    for (auto& stmt : function->body) {
        analyzeStatement(stmt.get());
    }
    
    if (function->returnType != DataType::VOID && !hasReturnStatement) {
        addWarning("Function '" + function->name.lexeme + "' may not return a value", function->name.line);
    }
    
    symbolManager.exitScope();
    inFunction = false;
}

void SemanticAnalyzer::analyzeStatement(Stmt* stmt) {
    if (auto decl = dynamic_cast<DeclarationStmt*>(stmt)) {
        analyzeDeclaration(decl);
    } else if (auto assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        analyzeAssignment(assign);
    } else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        analyzeIfStatement(ifStmt);
    } else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        analyzeWhileStatement(whileStmt);
    } else if (auto returnStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        analyzeReturnStatement(returnStmt);
    } else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt)) {
        analyzeExpressionStatement(exprStmt);
    } else if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        symbolManager.enterScope();
        for (auto& s : blockStmt->statements) {
            analyzeStatement(s.get());
        }
        symbolManager.exitScope();
    }
}

void SemanticAnalyzer::analyzeDeclaration(DeclarationStmt* decl) {
    if (!symbolManager.declareSymbol(decl->name.lexeme, decl->dataType)) {
        addError("Variable '" + decl->name.lexeme + "' already declared in this scope", decl->name.line);
        return;
    }
    
    if (decl->initializer) {
        DataType initType = analyzeExpression(decl->initializer.get());
        if (!isTypeCompatible(decl->dataType, initType, TokenType::ASSIGN)) {
            addError("Type mismatch in initialization of '" + decl->name.lexeme + 
                    "', expected " + dataTypeToString(decl->dataType) + 
                    " but got " + dataTypeToString(initType), decl->name.line);
        } else {
            symbolManager.markSymbolInitialized(decl->name.lexeme);
        }
    }
}

void SemanticAnalyzer::analyzeAssignment(AssignmentStmt* assign) {
    Symbol* symbol = symbolManager.lookupSymbol(assign->name.lexeme);
    if (!symbol) {
        addError("Undefined variable '" + assign->name.lexeme + "'", assign->name.line);
        return;
    }
    
    if (symbol->isConstant) {
        addError("Cannot assign to constant '" + assign->name.lexeme + "'", assign->name.line);
        return;
    }
    
    DataType valueType = analyzeExpression(assign->value.get());
    if (!isTypeCompatible(symbol->type, valueType, TokenType::ASSIGN)) {
        addError("Type mismatch in assignment to '" + assign->name.lexeme + 
                "', expected " + dataTypeToString(symbol->type) + 
                " but got " + dataTypeToString(valueType), assign->name.line);
    } else {
        symbolManager.markSymbolInitialized(assign->name.lexeme);
    }
}

void SemanticAnalyzer::analyzeIfStatement(IfStmt* ifStmt) {
    DataType condType = analyzeExpression(ifStmt->condition.get());
    if (condType != DataType::BOOL && condType != DataType::UNKNOWN) {
        addError("Condition expression must be boolean", ifStmt->line);
    }
    
    symbolManager.enterScope();
    for (auto& stmt : ifStmt->thenBranch) {
        analyzeStatement(stmt.get());
    }
    symbolManager.exitScope();
    
    if (!ifStmt->elseBranch.empty()) {
        symbolManager.enterScope();
        for (auto& stmt : ifStmt->elseBranch) {
            analyzeStatement(stmt.get());
        }
        symbolManager.exitScope();
    }
}

void SemanticAnalyzer::analyzeWhileStatement(WhileStmt* whileStmt) {
    DataType condType = analyzeExpression(whileStmt->condition.get());
    if (condType != DataType::BOOL && condType != DataType::UNKNOWN) {
        addError("Condition expression must be boolean", whileStmt->line);
    }
    
    symbolManager.enterScope();
    for (auto& stmt : whileStmt->body) {
        analyzeStatement(stmt.get());
    }
    symbolManager.exitScope();
}

void SemanticAnalyzer::analyzeReturnStatement(ReturnStmt* returnStmt) {
    if (!inFunction) {
        addError("Return statement outside function", returnStmt->line);
        return;
    }
    
    hasReturnStatement = true;
    
    if (returnStmt->value) {
        DataType returnType = analyzeExpression(returnStmt->value.get());
        if (!isTypeCompatible(currentFunctionReturnType, returnType, TokenType::ASSIGN)) {
            addError("Return type mismatch, expected " + 
                    dataTypeToString(currentFunctionReturnType) + 
                    " but got " + dataTypeToString(returnType), returnStmt->line);
        }
    } else if (currentFunctionReturnType != DataType::VOID) {
        addError("Function must return a value of type " + 
                dataTypeToString(currentFunctionReturnType), returnStmt->line);
    }
}

void SemanticAnalyzer::analyzeExpressionStatement(ExpressionStmt* exprStmt) {
    analyzeExpression(exprStmt->expression.get());
}

DataType SemanticAnalyzer::analyzeExpression(Expr* expr) {
    if (auto binary = dynamic_cast<BinaryExpr*>(expr)) {
        return analyzeBinaryExpression(binary);
    } else if (auto unary = dynamic_cast<UnaryExpr*>(expr)) {
        return analyzeUnaryExpression(unary);
    } else if (auto literal = dynamic_cast<LiteralExpr*>(expr)) {
        return analyzeLiteralExpression(literal);
    } else if (auto variable = dynamic_cast<VariableExpr*>(expr)) {
        return analyzeVariableExpression(variable);
    } else if (auto call = dynamic_cast<CallExpr*>(expr)) {
        return analyzeCallExpression(call);
    } else if (auto sequence = dynamic_cast<SequenceExpr*>(expr)) {
        return analyzeSequenceExpression(sequence);
    }
    return DataType::UNKNOWN;
}

DataType SemanticAnalyzer::analyzeBinaryExpression(BinaryExpr* binaryExpr) {
    DataType leftType = analyzeExpression(binaryExpr->left.get());
    DataType rightType = analyzeExpression(binaryExpr->right.get());
    
    if (!isTypeCompatible(leftType, rightType, binaryExpr->op.type)) {
        addError("Type mismatch in binary operation '" + binaryExpr->op.lexeme + 
                "', left: " + dataTypeToString(leftType) + 
                ", right: " + dataTypeToString(rightType), binaryExpr->op.line);
        return DataType::UNKNOWN;
    }
    
    if (!isValidOperation(leftType, binaryExpr->op.type)) {
        addError("Invalid operation '" + binaryExpr->op.lexeme + 
                "' for type " + dataTypeToString(leftType), binaryExpr->op.line);
        return DataType::UNKNOWN;
    }
    
    switch (binaryExpr->op.type) {
        case TokenType::PLUS:
            if (leftType == DataType::SEQUENCE && rightType == DataType::SEQUENCE) {
                return DataType::SEQUENCE;
            }
            [[fallthrough]];
        case TokenType::MINUS:
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
            if (leftType == DataType::FLOAT || rightType == DataType::FLOAT) {
                return DataType::FLOAT;
            }
            return DataType::INT;
            
        case TokenType::MODULO:
            return DataType::INT;
            
        case TokenType::EQUALS:
        case TokenType::NOT_EQUALS:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::AND:
        case TokenType::OR:
            return DataType::BOOL;
            
        default:
            return DataType::UNKNOWN;
    }
}

DataType SemanticAnalyzer::analyzeUnaryExpression(UnaryExpr* unaryExpr) {
    DataType exprType = analyzeExpression(unaryExpr->right.get());
    
    if (!isValidOperation(exprType, unaryExpr->op.type)) {
        addError("Invalid unary operation '" + unaryExpr->op.lexeme + 
                "' for type " + dataTypeToString(exprType), unaryExpr->op.line);
        return DataType::UNKNOWN;
    }
    
    if (unaryExpr->op.type == TokenType::NOT) {
        return DataType::BOOL;
    }
    
    return exprType;
}

DataType SemanticAnalyzer::analyzeLiteralExpression(LiteralExpr* literalExpr) {
    switch (literalExpr->value.type) {
        case TokenType::NUMBER:
            return DataType::INT;
        case TokenType::FLOAT:
            return DataType::FLOAT;
        case TokenType::STRING:
            return DataType::SEQUENCE;
        case TokenType::TRUE:
        case TokenType::FALSE:
            return DataType::BOOL;
        default:
            return DataType::UNKNOWN;
    }
}

DataType SemanticAnalyzer::analyzeVariableExpression(VariableExpr* varExpr) {
    Symbol* symbol = symbolManager.lookupSymbol(varExpr->name.lexeme);
    if (!symbol) {
        addError("Undefined variable '" + varExpr->name.lexeme + "'", varExpr->name.line);
        return DataType::UNKNOWN;
    }
    
    if (!symbol->isInitialized) {
        addWarning("Variable '" + varExpr->name.lexeme + "' may be uninitialized", varExpr->name.line);
    }
    
    return symbol->type;
}

DataType SemanticAnalyzer::analyzeCallExpression(CallExpr* callExpr) {
    std::string funcName = callExpr->callee.lexeme;
    
    if (funcName == "print") {
        for (auto& arg : callExpr->arguments) {
            analyzeExpression(arg.get());
        }
        return DataType::VOID;
    } else if (funcName == "length") {
        if (callExpr->arguments.size() != 1) {
            addError("Function 'length' expects 1 argument", callExpr->callee.line);
            return DataType::INT;
        }
        DataType argType = analyzeExpression(callExpr->arguments[0].get());
        if (argType != DataType::SEQUENCE && argType != DataType::UNKNOWN) {
            addError("Function 'length' expects a sequence argument", callExpr->callee.line);
        }
        return DataType::INT;
    } else if (funcName == "get") {
        // Array indexing: get(array, index)
        if (callExpr->arguments.size() != 2) {
            addError("Array indexing requires array and index", callExpr->callee.line);
            return DataType::INT;
        }
        DataType arrayType = analyzeExpression(callExpr->arguments[0].get());
        DataType indexType = analyzeExpression(callExpr->arguments[1].get());
        
        if (arrayType != DataType::SEQUENCE && arrayType != DataType::UNKNOWN) {
            addError("Cannot index non-sequence type", callExpr->callee.line);
        }
        if (indexType != DataType::INT && indexType != DataType::UNKNOWN) {
            addError("Array index must be an integer", callExpr->callee.line);
        }
        return DataType::INT; // Assume sequences contain integers
    } else if (funcName == "map") {
        if (callExpr->arguments.size() != 2) {
            addError("Function 'map' expects 2 arguments", callExpr->callee.line);
        } else {
            analyzeExpression(callExpr->arguments[0].get());
            analyzeExpression(callExpr->arguments[1].get());
        }
        return DataType::SEQUENCE;
    } else if (funcName == "filter") {
        if (callExpr->arguments.size() != 2) {
            addError("Function 'filter' expects 2 arguments", callExpr->callee.line);
        } else {
            analyzeExpression(callExpr->arguments[0].get());
            analyzeExpression(callExpr->arguments[1].get());
        }
        return DataType::SEQUENCE;
    } else if (funcName == "generate") {
        for (auto& arg : callExpr->arguments) {
            analyzeExpression(arg.get());
        }
        return DataType::SEQUENCE;
    } else if (funcName == "input") {
        if (callExpr->arguments.size() > 1) {
            addError("Function 'input' expects 0 or 1 argument", callExpr->callee.line);
        } else if (callExpr->arguments.size() == 1) {
            DataType promptType = analyzeExpression(callExpr->arguments[0].get());
            if (promptType != DataType::SEQUENCE && promptType != DataType::UNKNOWN) {
                addError("Function 'input' expects a string literal prompt", callExpr->callee.line);
            }
        }
        return DataType::INT;
    }
    
    Symbol* symbol = symbolManager.lookupSymbol(funcName);
    if (!symbol) {
        addError("Undefined function '" + funcName + "'", callExpr->callee.line);
        return DataType::UNKNOWN;
    }
    
    for (auto& arg : callExpr->arguments) {
        analyzeExpression(arg.get());
    }
    
    return symbol->type;
}

DataType SemanticAnalyzer::analyzeSequenceExpression(SequenceExpr* seqExpr) {
    if (seqExpr->elements.empty()) {
        return DataType::SEQUENCE;
    }
    
    DataType firstType = analyzeExpression(seqExpr->elements[0].get());
    for (size_t i = 1; i < seqExpr->elements.size(); ++i) {
        DataType elemType = analyzeExpression(seqExpr->elements[i].get());
        if (!isTypeCompatible(firstType, elemType, TokenType::ASSIGN) && 
            firstType != DataType::UNKNOWN && elemType != DataType::UNKNOWN) {
            addWarning("Inconsistent types in sequence", seqExpr->elements[i]->line);
        }
    }
    
    return DataType::SEQUENCE;
}

bool SemanticAnalyzer::isTypeCompatible(DataType left, DataType right, TokenType op) {
    if (left == DataType::UNKNOWN || right == DataType::UNKNOWN) {
        return true; 
    }
    
    switch (op) {
        case TokenType::ASSIGN:
            return left == right || canCoerce(right, left);
            
        case TokenType::PLUS:
            if (left == DataType::SEQUENCE && right == DataType::SEQUENCE) {
                return true;
            }
            [[fallthrough]];
        case TokenType::MINUS:
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
            return (isNumericType(left) && isNumericType(right));
            
        case TokenType::MODULO:
            return left == DataType::INT && right == DataType::INT;
            
        case TokenType::EQUALS:
        case TokenType::NOT_EQUALS:
            return left == right || (isNumericType(left) && isNumericType(right));
            
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
            return isNumericType(left) && isNumericType(right);
            
        case TokenType::AND:
        case TokenType::OR:
            return left == DataType::BOOL && right == DataType::BOOL;
            
        default:
            return false;
    }
}

bool SemanticAnalyzer::isValidOperation(DataType type, TokenType op) {
    switch (op) {
        case TokenType::MINUS:
            return isNumericType(type);
        case TokenType::NOT:
            return type == DataType::BOOL;
        default:
            return true;
    }
}

bool SemanticAnalyzer::isNumericType(DataType type) {
    return type == DataType::INT || type == DataType::FLOAT;
}

bool SemanticAnalyzer::canCoerce(DataType from, DataType to) {
    if (from == to) return true;
    if (from == DataType::INT && to == DataType::FLOAT) return true;
    return false;
}

void SemanticAnalyzer::checkMainFunction(Program* program) {
    bool hasMain = false;
    for (const auto& func : program->functions) {
        if (func->name.lexeme == "main" && 
            func->returnType == DataType::INT && 
            func->parameters.empty()) {
            hasMain = true;
            break;
        }
    }
    
    if (!hasMain) {
        addWarning("Program should have a 'main' function with signature: func main() -> int");
    }
}