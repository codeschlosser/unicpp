#pragma once

#include "unicode_data.h"

namespace unicpp {

inline char32_t toupper(char32_t code) {
  auto it = kUpperMap.find(code);
  return it != kUpperMap.end() ? it->second : code;
}

inline char32_t tolower(char32_t code) {
  auto it = kLowerMap.find(code);
  return it != kLowerMap.end() ? it->second : code;
}

inline bool isalpha(char32_t code) {
  auto it = kGeneralCategoryRangeMap.upper_bound(code);
  if (it == kGeneralCategoryRangeMap.end()) {
    return false;
  }

  return (static_cast<uint64_t>(it->second) &
          static_cast<uint64_t>(GeneralCategory::L)) != 0;
}

inline bool isdigit(char32_t code) {
  auto it = kNumericTypeMap.find(code);
  if (it == kNumericTypeMap.end()) {
    return false;
  }

  return it->second == NumericType::Decimal || it->second == NumericType::Digit;
}

}  // namespace unicpp
