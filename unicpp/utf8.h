#pragma once

#include <cassert>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>

#include <stdint.h>

namespace unicpp {

constexpr char32_t kReplacementCharacter = 0xFFFD;
constexpr char32_t kMaxValidCharacter = 0x10FFFF;
constexpr char32_t kInvalidCharacter = (char32_t)-1;

enum class ErrorPolicy {
  kReplace,
  kSkip,
  kStop,
};

char32_t DecodeCharacter(const uint8_t*& ptr, const uint8_t* end);

template <typename Result>
Result Utf8Decode(std::string_view utf8_string, ErrorPolicy policy,
                  size_t* bytes_decoded) {
  Result result;
  const uint8_t* ptr = (const uint8_t*)utf8_string.data();
  const uint8_t* end = ptr + utf8_string.length();
  while (ptr < end) {
    char32_t ch = DecodeCharacter(ptr, end);
    if (ch == kInvalidCharacter) {
      if (policy == ErrorPolicy::kSkip) {
        continue;
      } else if (policy == ErrorPolicy::kStop) {
        break;
      } else if (policy == ErrorPolicy::kReplace) {
        ch = kReplacementCharacter;
      } else {
        assert(0);
      }
    }
    result.push_back(ch);
  }

  if (bytes_decoded != nullptr) {
    *bytes_decoded = ptr - (const uint8_t*)utf8_string.data();
  }

  return result;
}

template <class Container>
void EncodeValidCharacter(char32_t code, Container& container) {
  if (code <= 0x7F) {
    container.push_back(static_cast<char>(code));
  } else if (code <= 0x7FF) {
    container.push_back(static_cast<char>(0xC0 | ((code >> 6) & 0x1F)));
    container.push_back(static_cast<char>(0x80 | (code & 0x3F)));
  } else if (code <= 0xFFFF) {
    container.push_back(static_cast<char>(0xE0 | ((code >> 12) & 0xF)));
    container.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
    container.push_back(static_cast<char>(0x80 | (code & 0x3F)));
  } else {
    container.push_back(static_cast<char>(0xF0 | ((code >> 18) & 0x7)));
    container.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
    container.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
    container.push_back(static_cast<char>(0x80 | (code & 0x3F)));
  }
}

template <typename CharT>
std::string Utf8Encode(std::basic_string_view<CharT> string, ErrorPolicy policy,
                       size_t* chars_encoded) {
  std::string result;
  size_t i = 0;
  for (; i < string.length(); i++) {
    char32_t ch = string[i];
    if (ch > kMaxValidCharacter) {
      if (policy == ErrorPolicy::kSkip) {
        continue;
      } else if (policy == ErrorPolicy::kStop) {
        break;
      } else if (policy == ErrorPolicy::kReplace) {
        ch = kReplacementCharacter;
      } else {
        assert(0);
      }
    }

    EncodeValidCharacter(ch, result);
  }

  if (chars_encoded != nullptr) {
    *chars_encoded = i;
  }

  return result;
}

inline std::string Utf8EncodeReplaceInvalid(std::wstring_view wide_string) {
  return Utf8Encode(wide_string, ErrorPolicy::kReplace, nullptr);
}

inline std::string Utf8EncodeSkipInvalid(std::wstring_view wide_string) {
  return Utf8Encode(wide_string, ErrorPolicy::kSkip, nullptr);
}

inline std::string Utf8EncodeStopOnInvalid(std::wstring_view wide_string,
                                           size_t* wchars_encoded) {
  return Utf8Encode(wide_string, ErrorPolicy::kStop, wchars_encoded);
}

inline std::wstring Utf8DecodeReplaceInvalid(std::string_view utf8_string) {
  return Utf8Decode<std::wstring>(utf8_string, ErrorPolicy::kReplace, nullptr);
}

inline std::wstring Utf8DecodeSkipInvalid(std::string_view utf8_string) {
  return Utf8Decode<std::wstring>(utf8_string, ErrorPolicy::kSkip, nullptr);
}

inline std::wstring Utf8DecodeStopOnInvalid(std::string_view utf8_string,
                                            size_t* bytes_decoded) {
  return Utf8Decode<std::wstring>(utf8_string, ErrorPolicy::kStop,
                                  bytes_decoded);
}

}  // namespace unicpp
