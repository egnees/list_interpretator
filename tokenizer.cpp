#include <tokenizer.h>

bool Tokenizer::IsEnd() {
    return is_end_;
}

Token Tokenizer::GetToken() {
    if (!token_o_.has_value()) {
        Next();
    }
    if (token_o_.has_value()) {
        return token_o_.value();
    } else {
        throw SyntaxError("bad syntax");
    }
}

Tokenizer::Tokenizer(std::istream* in) : in_(in), is_end_(false) {
    while (in_->peek() == ' ' || in_->peek() == '\n') {
        in_->get();
    }
    if (in_->peek() == EOF && in_->eof()) {
        is_end_ = true;
    }
};

void Tokenizer::Next() {
    while (in_->peek() == ' ' || in_->peek() == '\n') {
        in_->get();
    }
    char next = in_->peek();
    if (next == '#') {
        Boolean();
    } else if ('0' <= next && next <= '9') {
        Constant();
    } else if (next == '.') {
        return Dot();
    } else if (next == '\'') {
        return Quote();
    } else if (next == '(' || next == ')') {
        return Bracket();
    } else if (next == '+' || next == '-') {
        in_->get();
        char next_1 = in_->peek();
        in_->unget();
        if ('0' <= next_1 && next_1 <= '9') {
            Constant();
        } else {
            Symbol();
        }
    } else if (SymbolToken::StartsWith(in_->peek())) {
        Symbol();
    } else if (next == EOF && in_->eof()) {
        is_end_ = true;
    } else {
        throw SyntaxError("unknown symbol");
    }
    CheckException();
}

void Tokenizer::CheckException() const {
    if (!GoodChar(in_->peek())) {
        throw SyntaxError("unexpected char");
    }
}

void Tokenizer::Symbol() {
    std::string symbol;
    while (SymbolToken::Contains(in_->peek())) {
        symbol.push_back(in_->peek());
        in_->get();
    }
    token_o_ = Token{SymbolToken{symbol}};
}

void Tokenizer::Constant() {
    bool is_minus = false;
    if (in_->peek() == '-' || in_->peek() == '+') {
        if (in_->get() == '-') {
            is_minus = true;
        }
    }
    int value{0};
    bool picked = false;
    while ('0' <= in_->peek() && in_->peek() <= '9') {
        picked = true;
        value = 10 * value + (in_->peek() - '0');
        in_->get();
    }
    if (!picked) {
        throw SyntaxError("number must have at least one digit");
    }
    token_o_ = Token{ConstantToken{is_minus ? -value : value}};
}

void Tokenizer::Boolean() {
    in_->get();
    char next = in_->get();
    if (next != 't' && next != 'f') {
        throw SyntaxError("there should be f or t after #");
    }
    token_o_ = Token{BooleanToken{next == 't'}};
}

void Tokenizer::Bracket() {
    token_o_ = Token{in_->get() == '(' ? BracketToken::OPEN : BracketToken::CLOSE};
}

void Tokenizer::Dot() {
    in_->get();
    token_o_ = Token{DotToken()};
}

void Tokenizer::Quote() {
    in_->get();
    token_o_ = Token{QuoteToken()};
}

bool Tokenizer::GoodChar(char c) const {
    return c == EOF || c == ' ' || c == '\n' || c == ')' || c == '(' || SymbolToken::Contains(c);
}

// Token's definitions

bool SymbolToken::StartsWith(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '<' || c == '=' || c == '>' ||
           c == '*' || c == '/' || c == '#' || c == '-' || c == '+';
}

bool SymbolToken::Contains(char c) {
    return StartsWith(c) || ('0' <= c && c <= '9') || c == '?' || c == '!';
}

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool BooleanToken::operator==(const BooleanToken& other) const {
    return value == other.value;
}
