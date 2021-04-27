#include "char_type.h"

namespace unicpp {

char32_t toupper(char32_t code) {
  auto it = kUpperMap.find(code);
  return it != kUpperMap.end() ? it->second : code;
}

char32_t tolower(char32_t code) {
  auto it = kLowerMap.find(code);
  return it != kLowerMap.end() ? it->second : code;
}

bool isalpha(char32_t code) {
  auto it = kGeneralCategoryRangeMap.upper_bound(code);
  if (it == kGeneralCategoryRangeMap.end()) {
    return false;
  }

  return (static_cast<uint64_t>(it->second) &
          static_cast<uint64_t>(GeneralCategory::L)) != 0;
}

bool isdigit(char32_t code) {
  auto it = kNumericTypeMap.find(code);
  if (it == kNumericTypeMap.end()) {
    return false;
  }

  return it->second == NumericType::Decimal || it->second == NumericType::Digit;
}

bool isspace(char32_t code) {
  auto git = kGeneralCategoryRangeMap.upper_bound(code);
  if (git != kGeneralCategoryRangeMap.end() &&
      git->second == GeneralCategory::Zs) {
    return true;
  }

  auto bidit = kBidiClassRangeMap.upper_bound(code);
  if (bidit == kBidiClassRangeMap.end()) {
    return false;
  }

  BidiClass bidi_class = bidit->second;

  return bidi_class == BidiClass::B || bidi_class == BidiClass::S ||
         bidi_class == BidiClass::WS;
}

const Decomposition* decomposition(char32_t code) {
  auto it = kDecompositionMap.find(code);
  if (it == kDecompositionMap.end()) {
    return nullptr;
  }

  return &it->second;
}

}  // namespace unicpp
