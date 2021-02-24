#pragma once

#include <array>
#include <cassert>
#include <deque>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <stdint.h>

namespace unicpp {

constexpr char32_t kReplacementCharacter = 0xFFFD;
constexpr char32_t kMaxValidCharacter = 0x10FFFF;

enum class ErrorPolicy {
  kReplace,
  kSkip,
  kStop,
};

namespace detail {

inline bool IsAligned64(const uint8_t* ptr) {
  uint64_t address = reinterpret_cast<uint64_t>(ptr);
  return (address & 0x7) == 0;
}

inline const uint64_t* Align64(const uint8_t* ptr) {
  uint64_t address = reinterpret_cast<uint64_t>(ptr);
  address &= ~0x7;
  return reinterpret_cast<const uint64_t*>(address);
}

inline bool IsAllAscii(uint64_t bytes) {
  return (bytes & 0x8080808080808080ULL) == 0;
}

inline bool IsContinuationByte(uint8_t byte) {
  return (byte & 0xC0) == 0x80;
}

constexpr bool IsLittleEndian() {
  // TODO: add actual check when we find different endianness
  return true;
}

template <class OutputIterator>
void PushAsciiChars(uint64_t packed, OutputIterator iterator) {
  if constexpr (IsLittleEndian()) {
    *iterator = packed & 0xFF;
    *++iterator = (packed >> 8) & 0xFF;
    *++iterator = (packed >> 16) & 0xFF;
    *++iterator = (packed >> 24) & 0xFF;
    *++iterator = (packed >> 32) & 0xFF;
    *++iterator = (packed >> 40) & 0xFF;
    *++iterator = (packed >> 48) & 0xFF;
    *++iterator = (packed >> 56) & 0xFF;
  } else {
    *iterator = (packed >> 56) & 0xFF;
    *++iterator = (packed >> 48) & 0xFF;
    *++iterator = (packed >> 40) & 0xFF;
    *++iterator = (packed >> 32) & 0xFF;
    *++iterator = (packed >> 24) & 0xFF;
    *++iterator = (packed >> 16) & 0xFF;
    *++iterator = (packed >> 8) & 0xFF;
    *++iterator = packed & 0xFF;
  }
  ++iterator;
}

}  // namespace detail

inline size_t Utf8ExpectedCodeLength(uint8_t byte0) {
  if (byte0 < 0x80) {
    return 1;
  } else if (byte0 < 0xE0) {
    return 2;
  } else if (byte0 < 0xF0) {
    return 3;
  } else if (byte0 < 0xF5) {
    return 4;
  } else {
    return 0;
  }
}

template <class OutputIterator>
size_t Utf8Decode(const uint8_t* data, const uint8_t* end,
                  OutputIterator iterator) {
  size_t total_size = (size_t)(end - data);
  const uint64_t* end64 = detail::Align64(end);
  while (data < end) {
    uint8_t byte0 = data[0];
    size_t expected_length = Utf8ExpectedCodeLength(byte0);
    if (expected_length == 1) {
      if (detail::IsAligned64(data)) {
        const uint64_t* data64 = reinterpret_cast<const uint64_t*>(data);
        while (data64 < end64 && detail::IsAllAscii(*data64)) {
          detail::PushAsciiChars(*data64++, iterator);
        }
        data = reinterpret_cast<const uint8_t*>(data64);
        if (data == end) {
          break;
        }
      }
      if (*data < 0x80) {
        *iterator = *data++;
      }
    } else if (expected_length == 2) {
      if (byte0 < 0xC2) {
        // should have used 1 byte
        break;
      }
      if (end - data < 2) {
        break;
      }
      int byte1 = data[1];
      if (!detail::IsContinuationByte(byte1)) {
        break;
      }
      data += 2;
      *iterator = ((byte0 & 0x1F) << 6) | (byte1 & 0x3F);
    } else if (expected_length == 3) {
      if (end - data < 3) {
        break;
      }
      int byte1 = data[1];
      if (!detail::IsContinuationByte(byte1)) {
        break;
      }
      if ((byte0 & 0xF) == 0 && (byte1 & 0x20) == 0) {
        // should have used 2 bytes
        break;
      }
      int byte2 = data[2];
      if (!detail::IsContinuationByte(byte2)) {
        break;
      }
      data += 3;
      *iterator =
          ((byte0 & 0xF) << 12) | ((byte1 & 0x3F) << 6) | (byte2 & 0x3F);
    } else if (expected_length == 4) {
      if ((byte0 & 0x7) > 0x4) {
        // bigger than of U+10FFFF
        break;
      }
      if (end - data < 4) {
        break;
      }
      int byte1 = data[1];
      if (!detail::IsContinuationByte(byte1)) {
        break;
      }
      if ((byte0 & 0x7) == 0 && (byte1 & 0x30) == 0) {
        // should have used 3 bytes
        break;
      }
      if ((byte0 & 0x7) == 0x4 && (byte1 & 0x30) > 0) {
        // bigger than of U+10FFFF
        break;
      }

      int byte2 = data[2];
      if (!detail::IsContinuationByte(byte2)) {
        break;
      }
      int byte3 = data[3];
      if (!detail::IsContinuationByte(byte3)) {
        break;
      }
      data += 4;
      *iterator = ((byte0 & 0x7) << 18) | ((byte1 & 0x3F) << 12) |
                  ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
    } else {
      break;
    }
    ++iterator;
  }

  return total_size - (size_t)(end - data);
}

template <class OutputIterator>
size_t Utf8Decode(std::string_view utf8_string, ErrorPolicy policy,
                  OutputIterator iterator) {
  const uint8_t* data = (const uint8_t*)utf8_string.data();
  const uint8_t* end = data + utf8_string.length();
  while (data < end) {
    size_t left = (size_t)(end - data);
    size_t decoded = Utf8Decode(data, end, iterator);
    if (decoded < left) {
      if (policy == ErrorPolicy::kSkip) {
        ++decoded;
      } else if (policy == ErrorPolicy::kStop) {
        data += decoded;
        break;
      } else if (policy == ErrorPolicy::kReplace) {
        *iterator = kReplacementCharacter;
        ++iterator;
        ++decoded;
      } else {
        assert(0);
      }
    }
    data += decoded;
  }

  return data - (const uint8_t*)utf8_string.data();
}

template <class OutputIterator>
OutputIterator Utf8EncodeValidCharacter(char32_t code,
                                        OutputIterator iterator) {
  if (code <= 0x7F) {
    *iterator = static_cast<uint8_t>(code);
  } else if (code <= 0x7FF) {
    *iterator = static_cast<uint8_t>(0xC0 | ((code >> 6) & 0x1F));
    *++iterator = static_cast<uint8_t>(0x80 | (code & 0x3F));
  } else if (code <= 0xFFFF) {
    *iterator = static_cast<uint8_t>(0xE0 | ((code >> 12) & 0xF));
    *++iterator = static_cast<uint8_t>(0x80 | ((code >> 6) & 0x3F));
    *++iterator = static_cast<uint8_t>(0x80 | (code & 0x3F));
  } else {
    *iterator = static_cast<uint8_t>(0xF0 | ((code >> 18) & 0x7));
    *++iterator = static_cast<uint8_t>(0x80 | ((code >> 12) & 0x3F));
    *++iterator = static_cast<uint8_t>(0x80 | ((code >> 6) & 0x3F));
    *++iterator = static_cast<uint8_t>(0x80 | (code & 0x3F));
  }
  return ++iterator;
}

template <class OutputIterator, class CharT>
size_t Utf8Encode(std::basic_string_view<CharT> string, ErrorPolicy policy,
                  OutputIterator iterator) {
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

    Utf8EncodeValidCharacter(ch, iterator);
  }

  return i;
}

template <class ByteStreamIterator>
class Utf8DecodeIterator {
public:
  static const size_t kBufferSize = 64;
  static const size_t kPaddingBytes = 4;

  Utf8DecodeIterator(ByteStreamIterator input_beg, ByteStreamIterator input_end,
                     ErrorPolicy policy = ErrorPolicy::kReplace)
      : state_(std::make_shared<State>()) {
    state_->input_beg = input_beg;
    state_->input_end = input_end;
    // set fake value to kick off
    state_->valid_bytes = state_->buffer.size();
    state_->policy = policy;
    Advance(state_->buffer.size());
    Next();
  }

  char32_t operator*() {
    return current_char;
  }

  Utf8DecodeIterator& operator++() {
    Next();

    return *this;
  }

  Utf8DecodeIterator operator++(int) {
    Utf8DecodeIterator tmp(*this);
    operator++();

    return tmp;
  }

  friend bool operator==(const Utf8DecodeIterator& a,
                         const Utf8DecodeIterator& b) {
    return a.state_ == b.state_;
  }

  operator bool() const {
    return state_ != nullptr;
  }

private:
  void Advance(size_t count) {
    if (state_ == nullptr) {
      return;
    }
    assert(count <= state_->valid_bytes);
    size_t bytes_to_keep = state_->valid_bytes - count;
    for (size_t i = 0; i < bytes_to_keep; i++) {
      state_->buffer[i] = state_->buffer[i + count];
    }
    state_->valid_bytes = bytes_to_keep;
    while (state_->input_beg != state_->input_end &&
           state_->valid_bytes < state_->buffer.size()) {
      state_->buffer[state_->valid_bytes++] = (*state_->input_beg);
      ++state_->input_beg;
    }
  }

  void Next() {
    if (state_ == nullptr) {
      return;
    }

    while (state_->valid_bytes > 0) {
      size_t bytes_expected = Utf8ExpectedCodeLength(state_->buffer[0]);
      size_t bytes_to_decode = std::min(state_->valid_bytes, bytes_expected);
      size_t bytes_decoded =
          Utf8Decode(&state_->buffer[0], &state_->buffer[0] + bytes_to_decode,
                     &current_char);
      if (bytes_decoded > 0) {
        assert(bytes_decoded == bytes_expected);
        Advance(bytes_decoded);
        return;
      }
      if (state_->policy == ErrorPolicy::kSkip) {
        Advance(1);
        continue;
      } else if (state_->policy == ErrorPolicy::kReplace) {
        Advance(1);
        current_char = kReplacementCharacter;
        return;
      } else if (state_->policy == ErrorPolicy::kStop) {
        state_.reset();
        return;
      } else {
        assert(0);
      }
    }
    state_.reset();
  }

  struct State {
    ByteStreamIterator input_beg;
    ByteStreamIterator input_end;
    std::array<uint8_t, 4> buffer;
    size_t valid_bytes;
    ErrorPolicy policy;
  };

  std::shared_ptr<State> state_;
  char32_t current_char;
};

template <class CharStreamIterator>
class Utf8EncodeIterator {
public:
  Utf8EncodeIterator(CharStreamIterator input_beg, CharStreamIterator input_end,
                     ErrorPolicy policy = ErrorPolicy::kReplace)
      : state_(std::make_shared<State>()) {
    state_->input_beg = input_beg;
    state_->input_end = input_end;
    state_->valid_bytes = 0;
    state_->current_offset = 0;
    state_->policy = policy;
    NextChar();
  }

  uint8_t operator*() {
    return current_byte;
  }

  Utf8EncodeIterator& operator++() {
    if (++state_->current_offset >= state_->valid_bytes) {
      NextChar();
    } else {
      current_byte = state_->buffer[state_->current_offset];
    }

    return *this;
  }

  Utf8EncodeIterator operator++(int) {
    Utf8EncodeIterator tmp(*this);
    operator++();

    return tmp;
  }

  friend bool operator==(const Utf8EncodeIterator& a,
                         const Utf8EncodeIterator& b) {
    return a.state_ == b.state_ && a.current_byte == b.current_byte;
  }

  operator bool() const {
    return state_ != nullptr;
  }

private:
  void NextChar() {
    if (state_ == nullptr || state_->current_offset < state_->valid_bytes) {
      return;
    }
    if (state_->input_beg == state_->input_end) {
      state_.reset();
      return;
    }

    char32_t ch = kMaxValidCharacter + 1;
    while (state_->input_beg != state_->input_end) {
      ch = *(state_->input_beg);
      ++state_->input_beg;
      if (ch <= kMaxValidCharacter) {
        break;
      }
      if (state_->policy == ErrorPolicy::kReplace) {
        ch = kReplacementCharacter;
        break;
      } else if (state_->policy == ErrorPolicy::kStop) {
        state_.reset();
        return;
      } else if (state_->policy == ErrorPolicy::kSkip) {
        continue;
      } else {
        assert(0);
      }
    }

    if (ch > kMaxValidCharacter) {
      state_.reset();
      return;
    }

    uint8_t* ptr = Utf8EncodeValidCharacter(ch, &state_->buffer[0]);
    state_->valid_bytes = ptr - &state_->buffer[0];
    state_->current_offset = 0;
    current_byte = state_->buffer[0];
  }

  struct State {
    CharStreamIterator input_beg;
    CharStreamIterator input_end;
    std::array<uint8_t, 4> buffer;
    size_t valid_bytes;
    size_t current_offset;
    ErrorPolicy policy;
  };

  std::shared_ptr<State> state_;
  uint8_t current_byte;
};

size_t Utf8ValidPrefixLength(std::string_view utf8_string);
size_t Utf8NumValidChars(std::string_view utf8_string);
size_t Utf8NumCharsWithReplacement(std::string_view utf8_string);

std::string Utf8EncodeReplaceInvalid(std::wstring_view wide_string);
std::string Utf8EncodeSkipInvalid(std::wstring_view wide_string);
std::string Utf8EncodeStopOnInvalid(std::wstring_view wide_string,
                                    size_t* wchars_encoded = nullptr);
std::wstring Utf8DecodeReplaceInvalid(std::string_view utf8_string);
std::wstring Utf8DecodeSkipInvalid(std::string_view utf8_string);
std::wstring Utf8DecodeStopOnInvalid(std::string_view utf8_string,
                                     size_t* bytes_decoded = nullptr);

}  // namespace unicpp
