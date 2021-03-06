#pragma once

#include <map>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <stdint.h>

namespace unicpp {

enum class GeneralCategory : uint64_t {
  None = 0LL,
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

enum class BidiClass {
  None,
  L,
  R,
  AL,
  EN,
  ES,
  ET,
  AN,
  CS,
  NSM,
  BN,
  B,
  S,
  WS,
  ON,
  LRE,
  LRO,
  RLE,
  RLO,
  PDF,
  LRI,
  RLI,
  FSI,
  PDI,
};

enum class NumericType {
  None,
  Decimal,
  Digit,
  Numeric,
};

enum class DecompositionTag {
  None,
  Font,      // A font variant (e.g. a blackletter form).
  NoBreak,   // A no-break version of a space or hyphen.
  Initial,   // An initial presentation form (Arabic).
  Medial,    // A medial presentation form (Arabic).
  Final,     // A final presentation form (Arabic).
  Isolated,  // An isolated presentation form (Arabic).
  Circle,    // An encircled form.
  Super,     // A superscript form.
  Sub,       // A subscript form.
  Vertical,  // A vertical layout presentation form.
  Wide,      // A wide (or zenkaku) compatibility character.
  Narrow,    // A narrow (or hankaku) compatibility character.
  Small,     // A small variant form (CNS compatibility).
  Square,    // A CJK squared font variant.
  Fraction,  // A vulgar fraction form.
  Compat,    // Otherwise unspecified compatibility character.
};

struct Decomposition {
  DecompositionTag tag;
  std::u32string_view characters;
};

extern const std::unordered_map<char32_t, char32_t> kUpperMap;
extern const std::unordered_map<char32_t, char32_t> kLowerMap;
extern const std::map<char32_t, GeneralCategory> kGeneralCategoryRangeMap;
extern const std::map<char32_t, BidiClass> kBidiClassRangeMap;
extern const std::unordered_map<char32_t, NumericType> kNumericTypeMap;
extern const std::unordered_map<char32_t, Decomposition> kDecompositionMap;

}  // namespace unicpp
