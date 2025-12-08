#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "codegen.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>

class Optimizer {
private:
    std::vector<ThreeAddressCode> code;
    
    // Optimization passes
    void constantFolding();
    void constantPropagation();
    void deadCodeElimination();
    void removeRedundantAssignments();
    void algebraicSimplification();
    
    // Helper functions
    bool isConstant(const std::string& value);
    int evaluateConstant(const std::string& op, const std::string& left, const std::string& right);
    bool isDeadTemp(const std::string& temp, size_t currentIndex);
    
public:
    Optimizer(std::vector<ThreeAddressCode> code) : code(std::move(code)) {}
    
    std::vector<ThreeAddressCode> optimize();
    void printOptimizedCode(std::ostream& out);
};

#endif