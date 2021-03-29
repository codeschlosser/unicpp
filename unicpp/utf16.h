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

template <class BytesIterator, class OutputIterator, Endian kEndian,
          bool kCheckBoundaries>
size_t Utf16DecodeImpl(BytesIterator bytes_beg, BytesIterator bytes_end,
                       OutputIterator output, OutputIterator output_end) {
  if constexpr (!kCheckBoundaries) {
    // supress unused variable warning
    (void)output_end;
  }

  BytesIterator iter = bytes_beg;
  while (iter != bytes_end) {
    if constexpr (kCheckBoundaries) {
      if (output == output_end) {
        break;
      }
    }
    if (std::distance(iter, bytes_end) < 2) {
      break;
    }
    int word0 = detail::ReadWord<kEndian>(iter);
    if (!IsSurrogate(word0)) {
      *output = word0;
      std::advance(iter, 2);
    } else {
      if (std::distance(iter, bytes_end) < 4) {
        break;
      }
      int word1 = detail::ReadWord<kEndian>(std::next(iter, 2));
      if (!IsSurrogate(word1) || word1 < 0xDC00) {
        break;
      }
      std::advance(iter, 4);
      *output = (((word0 - 0xD800) << 10) | (word1 - 0xDC00));
    }
    ++output;
  }

  return std::distance(bytes_beg, iter);
}

}  // namespace detail

template <class BytesIterator, class OutputIterator,
          Endian kEndian = Endian::kLittle>
size_t Utf16Decode(BytesIterator bytes_beg, BytesIterator bytes_end,
                   OutputIterator output) {
  return detail::Utf16DecodeImpl<BytesIterator, OutputIterator, kEndian,
                                 /*kCheckBoundaries = */ false>(
      bytes_beg, bytes_end, output, output);
}

template <class BytesIterator, class OutputIterator,
          Endian kEndian = Endian::kLittle>
size_t Utf16Decode(BytesIterator bytes_beg, BytesIterator bytes_end,
                   OutputIterator output_beg, OutputIterator output_end) {
  return detail::Utf16DecodeImpl<BytesIterator, OutputIterator, kEndian,
                                 /*kCheckBoundaries = */ true>(
      bytes_beg, bytes_end, output_beg, output_end);
}

template <class BytesIterator, class OutputIterator,
          Endian kEndian = Endian::kLittle>
size_t Utf16Decode(BytesIterator bytes_beg, BytesIterator bytes_end,
                   OutputIterator output, ErrorPolicy policy) {
  BytesIterator iter = bytes_beg;
  while (iter != bytes_end) {
    size_t bytes_left = static_cast<size_t>(std::distance(iter, bytes_end));
    size_t decoded = Utf16Decode<BytesIterator, OutputIterator, kEndian>(
        iter, bytes_end, output);
    if (decoded < bytes_left) {
      if (policy == ErrorPolicy::kSkip) {
        ++decoded;
      } else if (policy == ErrorPolicy::kStop) {
        std::advance(iter, decoded);
        break;
      } else if (policy == ErrorPolicy::kReplace) {
        *output = kReplacementCharacter;
        ++output;
        ++decoded;
      }
    }
    std::advance(iter, decoded);
  }

  return std::distance(bytes_beg, iter);
}

template <class BytesIterator, class OutputIterator>
size_t Utf16LeDecode(BytesIterator bytes_beg, BytesIterator bytes_end,
                     OutputIterator output, ErrorPolicy policy) {
  return Utf16Decode<BytesIterator, OutputIterator, Endian::kLittle>(
      bytes_beg, bytes_end, output, policy);
}

template <class BytesIterator, class OutputIterator>
size_t Utf16BeDecode(BytesIterator bytes_beg, BytesIterator bytes_end,
                     OutputIterator output, ErrorPolicy policy) {
  return Utf16Decode<BytesIterator, OutputIterator, Endian::kBig>(
      bytes_beg, bytes_end, output, policy);
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

template <class CharsIterator, class OutputIterator,
          Endian kEndian = Endian::kLittle>
size_t Utf16Encode(CharsIterator input_beg, CharsIterator input_end,
                   OutputIterator output, ErrorPolicy policy) {
  CharsIterator iter = input_beg;
  while (iter != input_end) {
    char32_t ch = *iter;
    if (!IsValidCharacter(ch)) {
      if (policy == ErrorPolicy::kSkip) {
        continue;
      } else if (policy == ErrorPolicy::kStop) {
        break;
      } else if (policy == ErrorPolicy::kReplace) {
        ch = kReplacementCharacter;
      }
    }

    output = Utf16EncodeValidCharacter<OutputIterator, kEndian>(ch, output);
    ++iter;
  }

  return std::distance(input_beg, iter);
}

template <class CharsIterator, class OutputIterator>
size_t Utf16LeEncode(CharsIterator input_beg, CharsIterator input_end,
                     OutputIterator output, ErrorPolicy policy) {
  return Utf16Encode<CharsIterator, OutputIterator, Endian::kLittle>(
      input_beg, input_end, output, policy);
}

template <class CharsIterator, class OutputIterator>
size_t Utf16BeEncode(CharsIterator input_beg, CharsIterator input_end,
                     OutputIterator output, ErrorPolicy policy) {
  return Utf16Encode<CharsIterator, OutputIterator, Endian::kBig>(
      input_beg, input_end, output, policy);
}

template <class Result, class Wstring>
Result Utf16LeBytes(const Wstring& wstring,
                    ErrorPolicy policy = ErrorPolicy::kReplace,
                    size_t* chars_encoded = nullptr) {
  Result result;
  size_t encoded = Utf16LeEncode(wstring.begin(), wstring.end(),
                                 std::back_insert_iterator(result), policy);

  if (chars_encoded != nullptr) {
    *chars_encoded = encoded;
  }

  return result;
}

template <class Result, class Wstring>
Result Utf16BeBytes(const Wstring& wstring,
                    ErrorPolicy policy = ErrorPolicy::kReplace,
                    size_t* chars_encoded = nullptr) {
  Result result;
  size_t encoded = Utf16BeEncode(wstring.begin(), wstring.end(),
                                 std::back_insert_iterator(result), policy);

  if (chars_encoded != nullptr) {
    *chars_encoded = encoded;
  }

  return result;
}

template <class Wstring, class BytesContainer>
Wstring Utf16LeWstring(const BytesContainer& bytes,
                       ErrorPolicy policy = ErrorPolicy::kReplace,
                       size_t* bytes_decoded = nullptr) {
  Wstring result;
  size_t decoded = Utf16LeDecode(bytes.begin(), bytes.end(),
                                 std::back_insert_iterator(result), policy);
  if (bytes_decoded != nullptr) {
    *bytes_decoded = decoded;
  }

  return result;
}

template <class Wstring, class BytesContainer>
Wstring Utf16BeWstring(const BytesContainer& bytes,
                       ErrorPolicy policy = ErrorPolicy::kReplace,
                       size_t* bytes_decoded = nullptr) {
  Wstring result;
  size_t decoded = Utf16BeDecode(bytes.begin(), bytes.end(),
                                 std::back_insert_iterator(result), policy);
  if (bytes_decoded != nullptr) {
    *bytes_decoded = decoded;
  }

  return result;
}

}  // namespace unicpp
