#include "parser.h"

Object* Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("there is no tokens to read");
    }
    Token current_token = tokenizer->GetToken();
    tokenizer->Next();
    if (current_token == Token{QuoteToken()}) {
        return ReadAfterQuote(tokenizer);
    } else if (current_token == Token{BracketToken::OPEN}) {
        return ReadList(tokenizer);
    } else if (current_token == Token{BracketToken::CLOSE}) {
        throw SyntaxError("unmatched close bracket");
    } else if (std::holds_alternative<ConstantToken>(current_token)) {
        return Hp().Make<Number>(std::get<ConstantToken>(current_token).value);
    } else if (std::holds_alternative<SymbolToken>(current_token)) {
        return Hp().Make<Symbol>(std::get<SymbolToken>(current_token).name);
    } else if (current_token == Token{DotToken()}) {
        throw SyntaxError("dot can not be outside of list");
    } else if (std::holds_alternative<BooleanToken>(current_token)) {
        return Hp().Make<Boolean>(std::get<BooleanToken>(current_token).value);
    } else {
        throw SyntaxError("unsupported token, may be tokenizer broken?");
    }
}

Object* ReadAfterQuote(Tokenizer* tokenizer) {
    Cell* first_cell_ptr = Hp().Make<Cell>(Hp().Make<Symbol>("quote"));
    Object* read_object = Read(tokenizer);
    Cell* next_cell_ptr = Hp().Make<Cell>(nullptr);
    next_cell_ptr->SetFirst(read_object);
    first_cell_ptr->SetSecond(next_cell_ptr);
    return first_cell_ptr;
}

Object* ReadList(Tokenizer* tokenizer) {
    Cell* first_cell_ptr = Hp().Make<Cell>(nullptr);
    Cell* current_cell_ptr(first_cell_ptr);

    bool meet_dot = false;
    bool meet_last_after_dot = false;
    bool read_at_least_one = false;

    while (true) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("unmatched open bracket");
        }

        if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
            tokenizer->Next();
            if (!read_at_least_one) {
                return nullptr;
            }
            break;
        }

        if (tokenizer->GetToken() == Token{DotToken()}) {
            if (!current_cell_ptr->GetFirst()) {
                throw SyntaxError("there is nothing before dot in the list");
            }
            meet_dot = true;
            tokenizer->Next();
        } else if (meet_last_after_dot) {
            throw SyntaxError("there should be close bracket after element next to dot");
        } else if (std::holds_alternative<ConstantToken>(tokenizer->GetToken()) ||
                   std::holds_alternative<SymbolToken>(tokenizer->GetToken()) ||
                   std::holds_alternative<BooleanToken>(tokenizer->GetToken()) ||
                   tokenizer->GetToken() == Token{BracketToken::OPEN} ||
                   tokenizer->GetToken() == Token{QuoteToken()}) {
            if (meet_dot) {
                meet_last_after_dot = true;
                current_cell_ptr->SetSecond(Read(tokenizer));
            } else {
                if (read_at_least_one) {
                    Cell* new_cell_ptr = Hp().Make<Cell>(nullptr);
                    current_cell_ptr->SetSecond(new_cell_ptr);
                    current_cell_ptr = new_cell_ptr;
                }
                current_cell_ptr->SetFirst(Read(tokenizer));
            }
        } else {
            throw SyntaxError("unsupported token, may be tokenizer broken?");
        }
        read_at_least_one = true;
    }

    if (meet_dot && !meet_last_after_dot) {
        throw SyntaxError("did not meet token after dot");
    }

    return first_cell_ptr;
}