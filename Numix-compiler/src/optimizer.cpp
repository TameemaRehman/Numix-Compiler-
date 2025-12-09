#include "../include/optimizer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>



std::vector<ThreeAddressCode> Optimizer::optimize() {
    // Apply optimization passes
    constantFolding();
    constantPropagation();
    algebraicSimplification();
    removeRedundantAssignments();
    deadCodeElimination();
    
    return code;
}

void Optimizer::constantFolding() {
    for (auto& instr : code) {
        if ((instr.op == "+" || instr.op == "-" || instr.op == "*" || instr.op == "/") &&
            isConstant(instr.arg1) && isConstant(instr.arg2)) {
            
            int result = evaluateConstant(instr.op, instr.arg1, instr.arg2);
            instr.op = "ASSIGN";
            instr.arg1 = std::to_string(result);
            instr.arg2 = "";
        }
    }
}

void Optimizer::constantPropagation() {
    std::unordered_map<std::string, std::string> constantMap;
    
    for (auto& instr : code) {
        // Update uses with known constants
        if (constantMap.find(instr.arg1) != constantMap.end()) {
            instr.arg1 = constantMap[instr.arg1];
        }
        if (constantMap.find(instr.arg2) != constantMap.end()) {
            instr.arg2 = constantMap[instr.arg2];
        }
        
        // Track new constants
        if (instr.op == "ASSIGN" && isConstant(instr.arg1)) {
            constantMap[instr.result] = instr.arg1;
        } else if (instr.op != "ASSIGN") {
            // If result is reassigned, remove from constant map
            constantMap.erase(instr.result);
        }
    }
}

void Optimizer::deadCodeElimination() {
    std::unordered_set<std::string> usedTemps;
    
    // First pass: mark all used temporaries
    for (const auto& instr : code) {
        if (!instr.arg1.empty() && instr.arg1[0] == 't') {
            usedTemps.insert(instr.arg1);
        }
        if (!instr.arg2.empty() && instr.arg2[0] == 't') {
            usedTemps.insert(instr.arg2);
        }
    }
    
    // Second pass: remove dead assignments
    std::vector<ThreeAddressCode> optimized;
    for (const auto& instr : code) {
        if (instr.op == "ASSIGN" && !instr.result.empty() && instr.result[0] == 't' && 
            usedTemps.find(instr.result) == usedTemps.end()) {
            // Skip this dead assignment
            continue;
        }
        optimized.push_back(instr);
    }
    
    code = std::move(optimized);
}

void Optimizer::removeRedundantAssignments() {
    std::vector<ThreeAddressCode> optimized;
    std::unordered_map<std::string, std::string> valueMap;
    
    for (const auto& instr : code) {
        if (instr.op == "ASSIGN") {
            // Check if this is a redundant assignment (x = x)
            if (instr.arg1 == instr.result) {
                continue; // Skip redundant assignment
            }
            
            // Check if we can replace uses of result with arg1
            valueMap[instr.result] = instr.arg1;
        } else {
            // Clear value map for non-assignment instructions
            valueMap.clear();
        }
        optimized.push_back(instr);
    }
    
    code = std::move(optimized);
}

void Optimizer::algebraicSimplification() {
    for (auto& instr : code) {
        // x + 0 → x
        if (instr.op == "+" && instr.arg2 == "0") {
            instr.op = "ASSIGN";
            instr.arg2 = "";
        }
        // x - 0 → x
        else if (instr.op == "-" && instr.arg2 == "0") {
            instr.op = "ASSIGN";
            instr.arg2 = "";
        }
        // x * 1 → x
        else if (instr.op == "*" && instr.arg2 == "1") {
            instr.op = "ASSIGN";
            instr.arg2 = "";
        }
        // x * 0 → 0
        else if (instr.op == "*" && (instr.arg1 == "0" || instr.arg2 == "0")) {
            instr.op = "ASSIGN";
            instr.arg1 = "0";
            instr.arg2 = "";
        }
        // 0 + x → x
        else if (instr.op == "+" && instr.arg1 == "0") {
            instr.op = "ASSIGN";
            instr.arg1 = instr.arg2;
            instr.arg2 = "";
        }
        // 1 * x → x
        else if (instr.op == "*" && instr.arg1 == "1") {
            instr.op = "ASSIGN";
            instr.arg1 = instr.arg2;
            instr.arg2 = "";
        }
    }
}

bool Optimizer::isConstant(const std::string& value) {
    if (value.empty()) return false;
    
    size_t start = 0;
    if (value[0] == '-' || value[0] == '+') {
        if (value.length() == 1) return false;
        start = 1;
    }
    
    for (size_t i = start; i < value.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(value[i]))) {
            return false;
        }
    }
    
    return true;
}

int Optimizer::evaluateConstant(const std::string& op, const std::string& left, const std::string& right) {
    int leftVal = std::stoi(left);
    int rightVal = std::stoi(right);
    
    if (op == "+") return leftVal + rightVal;
    if (op == "-") return leftVal - rightVal;
    if (op == "*") return leftVal * rightVal;
    if (op == "/" && rightVal != 0) return leftVal / rightVal;
    if (op == "%" && rightVal != 0) return leftVal % rightVal;
    
    return 0;
}

bool Optimizer::isDeadTemp(const std::string& temp, size_t currentIndex) {
    if (temp.empty() || temp[0] != 't') return false;
    
    for (size_t i = currentIndex + 1; i < code.size(); ++i) {
        if (code[i].arg1 == temp || code[i].arg2 == temp) {
            return false; // Temp is used later
        }
        if (code[i].result == temp) {
            break; // Temp is redefined
        }
    }
    return true;
}

void Optimizer::printOptimizedCode(std::ostream& out) {
    out << "Optimized Intermediate Code:" << std::endl;
    out << "============================" << std::endl;
    for (const auto& instr : code) {
        out << instr.toString() << std::endl;
    }

}
