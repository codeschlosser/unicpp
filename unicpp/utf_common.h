#pragma once

namespace unicpp {

constexpr char32_t kReplacementCharacter = 0xFFFD;

constexpr char32_t kMinSurrogate = 0xD800;
constexpr char32_t kMaxSurrogate = 0xDFFF;
constexpr char32_t kMaxValidCharacter = 0x10FFFF;
constexpr char32_t kInvalidCharacter = kMaxValidCharacter + 1;

enum class ErrorPolicy {
  kReplace,
  kSkip,
  kStop,
};

inline bool IsSurrogate(char32_t ch) {
  return ch >= kMinSurrogate && ch <= kMaxSurrogate;
}

inline bool IsValidCharacter(char32_t ch) {
  return ch <= kMaxValidCharacter && !IsSurrogate(ch);
}

}  // namespace unicpp
