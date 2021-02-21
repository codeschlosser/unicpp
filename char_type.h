#pragma once

#include "unicode_data.h"

namespace unicpp {

inline CharCode toupper(CharCode code) {
  auto it = kUpperMap.find(code);
  return it != kUpperMap.end() ? it->second : code;
}

inline CharCode tolower(CharCode code) {
  auto it = kLowerMap.find(code);
  return it != kLowerMap.end() ? it->second : code;
}

inline bool isalpha(CharCode code) {
  auto it = kGeneralCategoryRangeMap.upper_bound(code);
  if (it == kGeneralCategoryRangeMap.end()) {
    return false;
  }

  return (static_cast<uint64_t>(it->second) &
          static_cast<uint64_t>(GeneralCategory::L)) != 0;
}

inline bool isdigit(CharCode code) {
  auto it = kNumericTypeMap.find(code);
  if (it == kNumericTypeMap.end()) {
    return false;
  }

  return it->second == NumericType::Decimal || it->second == NumericType::Digit;
}

}  // namespace unicpp
