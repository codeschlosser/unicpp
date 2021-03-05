#pragma once

#include "utf_common.h"

#include <cassert>
#include <iterator>
#include <string>

namespace unicpp {

enum class Endian {
  kLittle,
  kBig,
};

namespace detail {

template <Endian kEndian, class Iterator>
uint16_t ReadWord(Iterator iter) {
  uint8_t lo = *iter;
  uint8_t hi = *++iter;
  ++iter;
  if constexpr (kEndian == Endian::kBig) {
    std::swap(lo, hi);
  }

  return static_cast<uint16_t>(lo | (hi << 8));
}

template <Endian kEndian, class Iterator>
Iterator WriterWord(uint16_t word, Iterator iter) {
  uint8_t first_byte = word & 0xFF;
  uint8_t second_byte = (word >> 8) & 0xFF;
  if constexpr (kEndian == Endian::kBig) {
    std::swap(first_byte, second_byte);
  }

  *iter = first_byte;
  *++iter = second_byte;

  return ++iter;
}

template <class InputIterator, class OutputIterator, Endian kEndian,
          bool kCheckBoundaries>
size_t Utf16DecodeImpl(InputIterator input, InputIterator input_end,
                       OutputIterator output, OutputIterator output_end) {
  if constexpr (!kCheckBoundaries) {
    // supress unused variable warning
    (void)output_end;
  }

  size_t total_size = static_cast<size_t>(std::distance(input, input_end));
  while (input != input_end) {
    if constexpr (kCheckBoundaries) {
      if (output == output_end) {
        break;
      }
    }
    if (std::distance(input, input_end) < 2) {
      break;
    }
    int word0 = detail::ReadWord<kEndian>(input);
    if (!IsSurrogate(word0)) {
      *output = *input;
      ++input;
    } else {
      if (std::distance(input, input_end) < 4) {
        break;
      }
      int word1 = detail::ReadWord<kEndian>(std::next(input, 2));
      if (!IsSurrogate(word1) || word1 < 0xDC00) {
        break;
      }
      std::advance(input, 4);
      *output = (((word0 - 0xD800) << 10) | (word1 - 0xDC00));
    }
    ++output;
  }

  return total_size - static_cast<size_t>(std::distance(input, input_end));
}

}  // namespace detail

template <class InputIterator, class OutputIterator,
          Endian kEndian = Endian::kLittle>
size_t Utf16Decode(InputIterator input_beg, InputIterator input_end,
                   OutputIterator output) {
  return detail::Utf16DecodeImpl<InputIterator, OutputIterator, kEndian,
                                 /*kCheckBoundaries = */ false>(
      input_beg, input_end, output, output);
}

template <class InputIterator, class OutputIterator,
          Endian kEndian = Endian::kLittle>
size_t Utf16Decode(InputIterator input_beg, InputIterator input_end,
                   OutputIterator output_beg, OutputIterator output_end) {
  return detail::Utf16DecodeImpl<InputIterator, OutputIterator, kEndian,
                                 /*kCheckBoundaries = */ true>(
      input_beg, input_end, output_beg, output_end);
}

template <class OutputIterator, ErrorPolicy kPolicy,
          Endian kEndian = Endian::kLittle>
size_t Utf16Decode(std::string_view string, OutputIterator output) {
  std::string_view::iterator iter = string.begin();
  std::string_view::iterator end = string.end();
  while (iter != end) {
    size_t left = static_cast<size_t>(end - iter);
    size_t decoded =
        Utf16Decode<std::string_view::iterator, OutputIterator, kEndian>(
            iter, end, output);
    if (decoded < left) {
      if constexpr (kPolicy == ErrorPolicy::kSkip) {
        ++decoded;
      } else if constexpr (kPolicy == ErrorPolicy::kStop) {
        std::advance(iter, decoded);
        break;
      } else if constexpr (kPolicy == ErrorPolicy::kReplace) {
        *output = kReplacementCharacter;
        ++output;
        ++decoded;
      }
    }
    std::advance(iter, decoded);
  }

  return iter - string.begin();
}

template <class OutputIterator, Endian kEndian = Endian::kLittle>
OutputIterator Utf16EncodeValidCharacter(char32_t code,
                                         OutputIterator iterator) {
  if (code <= 0xFFFF) {
    iterator =
        detail::WriterWord<kEndian>(static_cast<uint16_t>(code), iterator);
  } else {
    uint32_t sur = code - 0x10000;
    iterator = detail::WriterWord<kEndian>(
        static_cast<uint16_t>((sur >> 10) + 0xD800), iterator);
    iterator = detail::WriterWord<kEndian>(
        static_cast<uint16_t>((sur & 0x3FF) + 0xDC00), iterator);
  }

  return iterator;
}

template <class OutputIterator, class CharT, ErrorPolicy kPolicy,
          Endian kEndian = Endian::kLittle>
size_t Utf16Encode(std::basic_string_view<CharT> string,
                   OutputIterator iterator) {
  size_t i = 0;
  for (; i < string.length(); i++) {
    char32_t ch = string[i];
    if (!IsValidCharacter(ch)) {
      if constexpr (kPolicy == ErrorPolicy::kSkip) {
        continue;
      } else if constexpr (kPolicy == ErrorPolicy::kStop) {
        break;
      } else if constexpr (kPolicy == ErrorPolicy::kReplace) {
        ch = kReplacementCharacter;
      }
    }

    Utf16EncodeValidCharacter<OutputIterator, kEndian>(ch, iterator);
  }

  return i;
}

std::u32string Utf16LeDecodeReplaceInvalid(std::string_view u16_string);
std::u32string Utf16BeDecodeReplaceInvalid(std::string_view u16_string);
std::u32string Utf16LeDecodeSkipInvalid(std::string_view u16_string);
std::u32string Utf16BeDecodeSkipInvalid(std::string_view u16_string);
std::u32string Utf16LeDecodeStopOnInvalid(std::string_view u16_string,
                                          size_t* bytes_decoded = nullptr);
std::u32string Utf16BeDecodeStopOnInvalid(std::string_view u16_string,
                                          size_t* bytes_decoded = nullptr);

std::string Utf16LeEncodeReplaceInvalid(std::u32string_view u32_string);
std::string Utf16BeEncodeReplaceInvalid(std::u32string_view u32_string);
std::string Utf16LeEncodeSkipInvalid(std::u32string_view u32_string);
std::string Utf16BeEncodeSkipInvalid(std::u32string_view u32_string);
std::string Utf16LeEncodeStopOnInvalid(std::u32string_view u32_string,
                                       size_t* u32chars_encoded = nullptr);
std::string Utf16BeEncodeStopOnInvalid(std::u32string_view u32_string,
                                       size_t* u32chars_encoded = nullptr);

}  // namespace unicpp
