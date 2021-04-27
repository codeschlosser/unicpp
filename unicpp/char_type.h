#pragma once

#include "unicode_data.h"

namespace unicpp {

char32_t toupper(char32_t code);
char32_t tolower(char32_t code);

bool isalpha(char32_t code);
bool isdigit(char32_t code);
bool isspace(char32_t code);

const Decomposition* decomposition(char32_t code);

}  // namespace unicpp
