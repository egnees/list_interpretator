#pragma once

#include "tokenizer.h"
#include "parser.h"

class Interpreter {
public:
    std::string Run(const std::string&);

    Interpreter();
    ~Interpreter();

private:
    Scope* base_scope_;

    void ClearMemory();
    void Init();

    std::map<std::string, Object*> functions_;
};
