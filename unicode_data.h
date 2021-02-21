#pragma once

#include <map>
#include <unordered_map>

#include <stdint.h>

namespace unicpp {

using CharCode = char32_t;

enum class GeneralCategory : uint64_t {
  Unknown = 0LL,
  Lu = 1LL << 1,
  Ll = 1LL << 2,
  Lt = 1LL << 3,
  Lm = 1LL << 4,
  Lo = 1LL << 5,
  Mn = 1LL << 6,
  Mc = 1LL << 7,
  Me = 1LL << 8,
  Nd = 1LL << 9,
  Nl = 1LL << 10,
  No = 1LL << 11,
  Pc = 1LL << 12,
  Pd = 1LL << 13,
  Ps = 1LL << 14,
  Pe = 1LL << 15,
  Pi = 1LL << 16,
  Pf = 1LL << 17,
  Po = 1LL << 18,
  Sm = 1LL << 19,
  Sc = 1LL << 20,
  Sk = 1LL << 21,
  So = 1LL << 22,
  Zs = 1LL << 23,
  Zl = 1LL << 24,
  Zp = 1LL << 25,
  Cc = 1LL << 26,
  Cf = 1LL << 27,
  Cs = 1LL << 28,
  Co = 1LL << 29,
  Cn = 1LL << 30,
  LC = Lu | Ll | Lt,
  L = Lu | Ll | Lt | Lm | Lo,
  M = Mn | Mc | Me,
  N = Nd | Nl | No,
  P = Pc | Pd | Ps | Pe | Pi | Pf | Po,
  S = Sm | Sc | Sk | So,
  Z = Zs | Zl | Zp,
  C = Cc | Cf | Cs | Co | Cn,
};

enum class NumericType {
  None,
  Decimal,
  Digit,
  Numeric,
};

extern const std::unordered_map<CharCode, CharCode> kUpperMap;
extern const std::unordered_map<CharCode, CharCode> kLowerMap;
extern const std::map<CharCode, GeneralCategory> kGeneralCategoryRangeMap;
extern const std::unordered_map<CharCode, NumericType> kNumericTypeMap;

}  // namespace unicpp
