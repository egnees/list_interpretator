#pragma once

#include <memory>

#include "functional_object.h"
#include "tokenizer.h"

Object* Read(Tokenizer* tokenizer);

Object* ReadAfterQuote(Tokenizer* tokenizer);

Object* ReadList(Tokenizer* tokenizer);