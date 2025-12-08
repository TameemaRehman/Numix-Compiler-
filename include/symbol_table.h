#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "ast.h"

struct Symbol {
    std::string name;
    DataType type;
    bool isInitialized;
    bool isConstant;
    int scopeDepth;
    
    Symbol(const std::string& name, DataType type, bool initialized = false, bool constant = false, int depth = 0)
        : name(name), type(type), isInitialized(initialized), isConstant(constant), scopeDepth(depth) {}
};

class SymbolTable {
private:
    std::unordered_map<std::string, Symbol> symbols;
    SymbolTable* parent;
    int scopeDepth;
    
public:
    SymbolTable(int depth = 0, SymbolTable* parent = nullptr) 
        : parent(parent), scopeDepth(depth) {}
    
    bool insert(const std::string& name, DataType type, bool initialized = false, bool constant = false) {
        if (symbols.find(name) != symbols.end()) {
            return false; // Symbol already exists in current scope
        }
        symbols.emplace(name, Symbol(name, type, initialized, constant, scopeDepth));
        return true;
    }
    
    Symbol* lookup(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return &it->second;
        }
        if (parent != nullptr) {
            return parent->lookup(name);
        }
        return nullptr;
    }
    
    bool updateInitialization(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            it->second.isInitialized = true;
            return true;
        }
        if (parent != nullptr) {
            return parent->updateInitialization(name);
        }
        return false;
    }
    
    bool isInCurrentScope(const std::string& name) const {
        return symbols.find(name) != symbols.end();
    }
    
    SymbolTable* getParent() const { return parent; }
    int getScopeDepth() const { return scopeDepth; }
    
    std::vector<Symbol> getAllSymbols() const {
        std::vector<Symbol> result;
        for (const auto& pair : symbols) {
            result.push_back(pair.second);
        }
        return result;
    }
};

class SymbolTableManager {
private:
    std::vector<std::unique_ptr<SymbolTable>> tables;
    SymbolTable* current;
    int currentDepth;
    
public:
    SymbolTableManager() : currentDepth(0) {
        enterScope(); // Global scope
    }
    
    void enterScope() {
        auto newTable = std::make_unique<SymbolTable>(currentDepth, current);
        current = newTable.get();
        tables.push_back(std::move(newTable));
        currentDepth++;
    }
    
    void exitScope() {
        if (current && current->getParent()) {
            current = current->getParent();
            currentDepth--;
        }
    }
    
    SymbolTable* getCurrentScope() { return current; }
    
    bool declareSymbol(const std::string& name, DataType type, bool initialized = false, bool constant = false) {
        return current->insert(name, type, initialized, constant);
    }
    
    Symbol* lookupSymbol(const std::string& name) {
        return current->lookup(name);
    }
    
    bool markSymbolInitialized(const std::string& name) {
        return current->updateInitialization(name);
    }
    
    bool isDeclaredInCurrentScope(const std::string& name) {
        return current->isInCurrentScope(name);
    }
};

#endif