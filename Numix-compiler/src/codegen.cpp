#include "../include/codegen.h"
#include <iostream>

std::string CodeGenerator::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

std::string CodeGenerator::newLabel() {
    return "L" + std::to_string(labelCounter++);
}

std::vector<ThreeAddressCode> CodeGenerator::generate(Program* program) {
    intermediateCode.clear();
    tempCounter = 0;
    labelCounter = 0;
    
    generateProgram(program);
    return intermediateCode;
}

void CodeGenerator::generateProgram(Program* program) {
    // Generate code for each function
    for (auto& function : program->functions) {
        generateFunction(function.get());
    }
}

void CodeGenerator::generateFunction(FunctionDecl* function) {
    // Function label
    intermediateCode.push_back(ThreeAddressCode("LABEL", "", "", function->name.lexeme, function->line));
    
    // Enter function scope
    symbolManager.enterScope();
    
    // Allocate space for parameters
    for (const auto& param : function->parameters) {
        symbolManager.declareSymbol(param.first.lexeme, param.second, true);
        intermediateCode.push_back(ThreeAddressCode("ASSIGN", "param_" + param.first.lexeme, "", param.first.lexeme, param.first.line));
    }
    
    // Generate function body
    for (auto& stmt : function->body) {
        generateStatement(stmt.get());
    }
    
    // Add implicit return for void functions
    if (function->returnType == DataType::VOID) {
        intermediateCode.push_back(ThreeAddressCode("RETURN", "", "", "", function->line));
    }
    
    // Exit function scope
    symbolManager.exitScope();
}

void CodeGenerator::generateStatement(Stmt* stmt) {
    if (auto decl = dynamic_cast<DeclarationStmt*>(stmt)) {
        generateDeclaration(decl);
    } else if (auto assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        generateAssignment(assign);
    } else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        generateIfStatement(ifStmt);
    } else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        generateWhileStatement(whileStmt);
    } else if (auto returnStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        generateReturnStatement(returnStmt);
    } else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt)) {
        generateExpressionStatement(exprStmt);
    }
}

void CodeGenerator::generateDeclaration(DeclarationStmt* decl) {
    symbolManager.declareSymbol(decl->name.lexeme, decl->dataType);
    
    if (decl->initializer) {
        std::string value = generateExpression(decl->initializer.get());
        intermediateCode.push_back(ThreeAddressCode("ASSIGN", value, "", decl->name.lexeme, decl->line));
        symbolManager.markSymbolInitialized(decl->name.lexeme);
    }
}

void CodeGenerator::generateAssignment(AssignmentStmt* assign) {
    std::string value = generateExpression(assign->value.get());
    intermediateCode.push_back(ThreeAddressCode("ASSIGN", value, "", assign->name.lexeme, assign->line));
    symbolManager.markSymbolInitialized(assign->name.lexeme);
}

void CodeGenerator::generateIfStatement(IfStmt* ifStmt) {
    std::string condition = generateExpression(ifStmt->condition.get());
    std::string elseLabel = newLabel();
    std::string endLabel = newLabel();
    
    // If condition is false, jump to else
    intermediateCode.push_back(ThreeAddressCode("IF_FALSE", condition, "", elseLabel, ifStmt->line));
    
    // Then branch
    symbolManager.enterScope();
    for (auto& stmt : ifStmt->thenBranch) {
        generateStatement(stmt.get());
    }
    symbolManager.exitScope();
    
    intermediateCode.push_back(ThreeAddressCode("GOTO", "", "", endLabel, ifStmt->line));
    
    // Else branch
    intermediateCode.push_back(ThreeAddressCode("LABEL", "", "", elseLabel, ifStmt->line));
    symbolManager.enterScope();
    for (auto& stmt : ifStmt->elseBranch) {
        generateStatement(stmt.get());
    }
    symbolManager.exitScope();
    
    intermediateCode.push_back(ThreeAddressCode("LABEL", "", "", endLabel, ifStmt->line));
}

void CodeGenerator::generateWhileStatement(WhileStmt* whileStmt) {
    std::string startLabel = newLabel();
    std::string conditionLabel = newLabel();
    std::string endLabel = newLabel();
    
    intermediateCode.push_back(ThreeAddressCode("GOTO", "", "", conditionLabel, whileStmt->line));
    intermediateCode.push_back(ThreeAddressCode("LABEL", "", "", startLabel, whileStmt->line));
    
    // Loop body
    symbolManager.enterScope();
    for (auto& stmt : whileStmt->body) {
        generateStatement(stmt.get());
    }
    symbolManager.exitScope();
    
    intermediateCode.push_back(ThreeAddressCode("LABEL", "", "", conditionLabel, whileStmt->line));
    std::string condition = generateExpression(whileStmt->condition.get());
    intermediateCode.push_back(ThreeAddressCode("IF", condition, "", startLabel, whileStmt->line));
    
    intermediateCode.push_back(ThreeAddressCode("LABEL", "", "", endLabel, whileStmt->line));
}

void CodeGenerator::generateReturnStatement(ReturnStmt* returnStmt) {
    if (returnStmt->value) {
        std::string value = generateExpression(returnStmt->value.get());
        intermediateCode.push_back(ThreeAddressCode("RETURN", value, "", "", returnStmt->line));
    } else {
        intermediateCode.push_back(ThreeAddressCode("RETURN", "", "", "", returnStmt->line));
    }
}

void CodeGenerator::generateExpressionStatement(ExpressionStmt* exprStmt) {
    generateExpression(exprStmt->expression.get()); // Result discarded
}

std::string CodeGenerator::generateExpression(Expr* expr) {
    if (auto binary = dynamic_cast<BinaryExpr*>(expr)) {
        return generateBinaryExpression(binary);
    } else if (auto unary = dynamic_cast<UnaryExpr*>(expr)) {
        return generateUnaryExpression(unary);
    } else if (auto literal = dynamic_cast<LiteralExpr*>(expr)) {
        return generateLiteralExpression(literal);
    } else if (auto variable = dynamic_cast<VariableExpr*>(expr)) {
        return generateVariableExpression(variable);
    } else if (auto call = dynamic_cast<CallExpr*>(expr)) {
        return generateCallExpression(call);
    } else if (auto sequence = dynamic_cast<SequenceExpr*>(expr)) {
        return generateSequenceExpression(sequence);
    }
    return "error";
}

std::string CodeGenerator::generateBinaryExpression(BinaryExpr* binaryExpr) {
    std::string left = generateExpression(binaryExpr->left.get());
    std::string right = generateExpression(binaryExpr->right.get());
    std::string result = newTemp();
    std::string op = getOperatorTAC(binaryExpr->op.type);
    
    intermediateCode.push_back(ThreeAddressCode(op, left, right, result, binaryExpr->line));
    return result;
}

std::string CodeGenerator::generateUnaryExpression(UnaryExpr* unaryExpr) {
    std::string expr = generateExpression(unaryExpr->right.get());
    std::string result = newTemp();
    std::string op = getOperatorTAC(unaryExpr->op.type);
    
    intermediateCode.push_back(ThreeAddressCode(op, expr, "", result, unaryExpr->line));
    return result;
}

std::string CodeGenerator::generateLiteralExpression(LiteralExpr* literalExpr) {
    return literalExpr->value.lexeme;
}

std::string CodeGenerator::generateVariableExpression(VariableExpr* varExpr) {
    return varExpr->name.lexeme;
}

std::string CodeGenerator::generateCallExpression(CallExpr* callExpr) {
    std::string result = newTemp();
    std::string args;
    
    // Generate arguments
    for (size_t i = 0; i < callExpr->arguments.size(); ++i) {
        std::string arg = generateExpression(callExpr->arguments[i].get());
        intermediateCode.push_back(ThreeAddressCode("PARAM", arg, "", "", callExpr->line));
        if (!args.empty()) args += ", ";
        args += arg;
    }
    
    intermediateCode.push_back(ThreeAddressCode("CALL", callExpr->callee.lexeme, args, result, callExpr->line));
    return result;
}

std::string CodeGenerator::generateSequenceExpression(SequenceExpr* seqExpr) {
    std::string result = newTemp();
    intermediateCode.push_back(ThreeAddressCode("ASSIGN", "[]", "", result, seqExpr->line));
    
    for (size_t i = 0; i < seqExpr->elements.size(); ++i) {
        std::string element = generateExpression(seqExpr->elements[i].get());
        intermediateCode.push_back(ThreeAddressCode("STORE", element, std::to_string(i), result, seqExpr->line));
    }
    
    return result;
}

std::string CodeGenerator::getOperatorTAC(TokenType op) {
    switch (op) {
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::MULTIPLY: return "*";
        case TokenType::DIVIDE: return "/";
        case TokenType::MODULO: return "%";
        case TokenType::EQUALS: return "==";
        case TokenType::NOT_EQUALS: return "!=";
        case TokenType::LESS: return "<";
        case TokenType::LESS_EQUAL: return "<=";
        case TokenType::GREATER: return ">";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::AND: return "&&";
        case TokenType::OR: return "||";
        case TokenType::NOT: return "!";
        default: return "?";
    }
}

void CodeGenerator::printCode(std::ostream& out) {
    for (const auto& code : intermediateCode) {
        out << code.toString() << std::endl;
    }
}
