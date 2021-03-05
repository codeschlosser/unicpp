#include "utf16.h"

namespace unicpp {

std::u32string Utf16LeDecodeReplaceInvalid(std::string_view u16_string) {
  std::u32string result;
  Utf16Decode<std::back_insert_iterator<std::u32string>, ErrorPolicy::kReplace,
              Endian::kLittle>(u16_string, std::back_insert_iterator(result));
  return result;
}

std::u32string Utf16BeDecodeReplaceInvalid(std::string_view u16_string) {
  std::u32string result;
  Utf16Decode<std::back_insert_iterator<std::u32string>, ErrorPolicy::kReplace,
              Endian::kBig>(u16_string, std::back_insert_iterator(result));
  return result;
}

std::u32string Utf16LeDecodeSkipInvalid(std::string_view u16_string) {
  std::u32string result;
  Utf16Decode<std::back_insert_iterator<std::u32string>, ErrorPolicy::kSkip,
              Endian::kLittle>(u16_string, std::back_insert_iterator(result));
  return result;
}

std::u32string Utf16BeDecodeSkipInvalid(std::string_view u16_string) {
  std::u32string result;
  Utf16Decode<std::back_insert_iterator<std::u32string>, ErrorPolicy::kSkip,
              Endian::kBig>(u16_string, std::back_insert_iterator(result));
  return result;
}

std::u32string Utf16LeDecodeStopOnInvalid(std::string_view u16_string,
                                          size_t* bytes_decoded) {
  std::u32string result;
  size_t decoded = Utf16Decode<std::back_insert_iterator<std::u32string>,
                               ErrorPolicy::kStop, Endian::kLittle>(
      u16_string, std::back_insert_iterator(result));
  if (bytes_decoded != nullptr) {
    *bytes_decoded = decoded;
  }
  return result;
}

std::u32string Utf16BeDecodeStopOnInvalid(std::string_view u16_string,
                                          size_t* u32chars_encoded) {
  std::u32string result;
  size_t decoded =
      Utf16Decode<std::back_insert_iterator<std::u32string>, ErrorPolicy::kStop,
                  Endian::kBig>(u16_string, std::back_insert_iterator(result));
  if (u32chars_encoded != nullptr) {
    *u32chars_encoded = decoded;
  }
  return result;
}

std::string Utf16LeEncodeReplaceInvalid(std::u32string_view u32_string) {
  std::string result;
  Utf16Encode<std::back_insert_iterator<std::string>, char32_t,
              ErrorPolicy::kReplace, Endian::kLittle>(
      u32_string, std::back_insert_iterator(result));
  return result;
}

std::string Utf16BeEncodeReplaceInvalid(std::u32string_view u32_string) {
  std::string result;
  Utf16Encode<std::back_insert_iterator<std::string>, char32_t,
              ErrorPolicy::kReplace, Endian::kBig>(
      u32_string, std::back_insert_iterator(result));
  return result;
}

std::string Utf16LeEncodeSkipInvalid(std::u32string_view u32_string) {
  std::string result;
  Utf16Encode<std::back_insert_iterator<std::string>, char32_t,
              ErrorPolicy::kSkip, Endian::kLittle>(
      u32_string, std::back_insert_iterator(result));
  return result;
}

std::string Utf16BeEncodeSkipInvalid(std::u32string_view u32_string) {
  std::string result;
  Utf16Encode<std::back_insert_iterator<std::string>, char32_t,
              ErrorPolicy::kSkip, Endian::kBig>(
      u32_string, std::back_insert_iterator(result));
  return result;
}

std::string Utf16LeEncodeStopOnInvalid(std::u32string_view u32_string,
                                       size_t* u32chars_encoded) {
  std::string result;
  size_t encoded = Utf16Encode<std::back_insert_iterator<std::string>, char32_t,
                               ErrorPolicy::kStop, Endian::kLittle>(
      u32_string, std::back_insert_iterator(result));
  if (u32chars_encoded != nullptr) {
    *u32chars_encoded = encoded;
  }
  return result;
}
std::string Utf16BeEncodeStopOnInvalid(std::u32string_view u32_string,
                                       size_t* u32chars_encoded) {
  std::string result;
  size_t encoded = Utf16Encode<std::back_insert_iterator<std::string>, char32_t,
                               ErrorPolicy::kStop, Endian::kBig>(
      u32_string, std::back_insert_iterator(result));
  if (u32chars_encoded != nullptr) {
    *u32chars_encoded = encoded;
  }
  return result;
}

}  // namespace unicpp
