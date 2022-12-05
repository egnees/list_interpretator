#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <regex>
#include <string>

#include "error.h"

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;

    static bool StartsWith(char);
    static bool Contains(char);
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

struct BooleanToken {
    bool value;
    bool operator==(const BooleanToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const;
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, BooleanToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    void Constant();
    void Symbol();
    void Bracket();
    void Dot();
    void Quote();
    void Boolean();

    bool GoodChar(char) const;

    void CheckException() const;

    std::istream* in_;

    std::optional<Token> token_o_;

    bool is_end_;
};