#pragma once

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

template <class OutputIterator>
size_t Utf8Decode(const uint8_t* data, const uint8_t* end,
                  OutputIterator iterator) {
  size_t total_size = (size_t)(end - data);
  const uint64_t* end64 = detail::Align64(end);
  while (data < end) {
    uint8_t byte0 = data[0];
    if (byte0 < 0x80) {
      if (detail::IsAligned64(data)) {
        const uint64_t* data64 = reinterpret_cast<const uint64_t*>(data);
        while (data64 < end64 && detail::IsAllAscii(*data64)) {
          detail::PushAsciiChars(*data64++, iterator);
        }
        data = reinterpret_cast<const uint8_t*>(data64);
      }
      if (*data < 0x80) {
        *iterator = *data++;
      }
    } else if (byte0 < 0xE0) {
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
    } else if (byte0 < 0xF0) {
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
    } else if (byte0 < 0xF5) {
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
void EncodeValidCharacter(char32_t code, OutputIterator iterator) {
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
  ++iterator;
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

    EncodeValidCharacter(ch, iterator);
  }

  return i;
}

template <class ByteStreamIterator>
class DecodeIterator {
public:
  static const size_t kBufferSize = 64;
  static const size_t kPaddingBytes = 4;

  DecodeIterator(ByteStreamIterator input_beg, ByteStreamIterator input_end,
                 ErrorPolicy policy = ErrorPolicy::kReplace)
      : state_(std::make_shared<State>()) {
    state_->input_beg = input_beg;
    state_->input_end = input_end;
    state_->input_buffer.reserve(kBufferSize);
    state_->policy = policy;
    MaybePreload();
  }

  char32_t operator*() {
    return state_->output_buffer.front();
  }

  DecodeIterator& operator++() {
    state_->output_buffer.pop_front();
    MaybePreload();

    return *this;
  }

  DecodeIterator operator++(int) {
    DecodeIterator tmp(state_->input_end, state_->input_end);
    tmp.state_->output_buffer.push_back(state_->output_buffer.front());
    operator++();

    return tmp;
  }

  void MaybePreload() {
    if (state_ == nullptr || !state_->output_buffer.empty()) {
      return;
    }
    if (state_->input_beg == state_->input_end) {
      state_.reset();
      return;
    }

    while (state_->input_beg != state_->input_end &&
           state_->input_buffer.size() < kBufferSize) {
      state_->input_buffer.push_back(*state_->input_beg);
      ++state_->input_beg;
    }

    size_t bytes_decoded = 0;
    size_t bytes_to_decode = state_->input_buffer.size() < kBufferSize
                                 ? state_->input_buffer.size()
                                 : kBufferSize - kPaddingBytes;
    std::back_insert_iterator<std::deque<char32_t>> output_iter(
        state_->output_buffer);
    while (bytes_decoded < bytes_to_decode) {
      bytes_decoded +=
          Utf8Decode(&state_->input_buffer[0] + bytes_decoded,
                     &state_->input_buffer[0] + bytes_to_decode, output_iter);
      if (bytes_decoded < bytes_to_decode) {
        // kReplace
        if (state_->policy == ErrorPolicy::kReplace) {
          bytes_decoded++;
          *output_iter = kReplacementCharacter;
          ++output_iter;
        } else if (state_->policy == ErrorPolicy::kSkip) {
          bytes_decoded++;
        } else if (state_->policy == ErrorPolicy::kStop) {
          if (bytes_decoded == 0) {
            state_.reset();
            return;
          } else {
            break;
          }
        }
      }
    }
    for (size_t i = 0; i < state_->input_buffer.size() - bytes_decoded; i++) {
      state_->input_buffer[i] = state_->input_buffer[i + bytes_decoded];
    }
    state_->input_buffer.resize(state_->input_buffer.size() - bytes_decoded);
  }

  friend bool operator==(const DecodeIterator& a, const DecodeIterator& b) {
    return a.state_ == b.state_;
  }

  operator bool() const {
    return state_ != nullptr;
  }

private:
  struct State {
    ByteStreamIterator input_beg;
    ByteStreamIterator input_end;
    std::vector<uint8_t> input_buffer;
    std::deque<char32_t> output_buffer;
    ErrorPolicy policy;
  };

  std::shared_ptr<State> state_;
};

template <class CharStreamIterator>
class EncodeIterator {
public:
  EncodeIterator(CharStreamIterator input_beg, CharStreamIterator input_end,
                 ErrorPolicy policy = ErrorPolicy::kReplace)
      : state_(std::make_shared<State>()) {
    state_->input_beg = input_beg;
    state_->input_end = input_end;
    state_->policy = policy;
    MaybePreload();
  }

  uint8_t operator*() {
    return state_->output_buffer.front();
  }

  EncodeIterator& operator++() {
    state_->output_buffer.pop_front();
    MaybePreload();

    return *this;
  }

  EncodeIterator operator++(int) {
    EncodeIterator tmp(state_->input_end, state_->input_end);
    tmp.state_->output_buffer.push_back(state_->output_buffer.front());
    operator++();

    return tmp;
  }

  void MaybePreload() {
    if (state_ == nullptr || !state_->output_buffer.empty()) {
      return;
    }
    if (state_->input_beg == state_->input_end) {
      state_.reset();
      return;
    }

    std::back_insert_iterator<std::deque<uint8_t>> iter(state_->output_buffer);
    char32_t ch = *(state_->input_beg);
    ++state_->input_beg;
    if (ch > kMaxValidCharacter) {
      if (state_->policy == ErrorPolicy::kReplace) {
        ch = kReplacementCharacter;
      } else if (state_->policy == ErrorPolicy::kStop) {
        state_.reset();
        return;
      } else if (state_->policy == ErrorPolicy::kSkip) {
        while (state_->input_beg != state_->input_end &&
               ch > kMaxValidCharacter) {
          ch = *(state_->input_beg);
          ++state_->input_beg;
        }
        if (ch > kMaxValidCharacter) {
          state_.reset();
          return;
        }
      } else {
        assert(0);
      }
    }
    EncodeValidCharacter(ch, iter);
  }

  friend bool operator==(const EncodeIterator& a, const EncodeIterator& b) {
    return a.state_ == b.state_;
  }

  operator bool() const {
    return state_ != nullptr;
  }

private:
  struct State {
    CharStreamIterator input_beg;
    CharStreamIterator input_end;
    std::deque<uint8_t> output_buffer;
    ErrorPolicy policy;
  };

  std::shared_ptr<State> state_;
};

size_t Utf8ValidCharacters(std::string_view utf8_string);
size_t Utf8ValidPrefix(std::string_view utf8_string);

std::string Utf8EncodeReplaceInvalid(std::wstring_view wide_string);
std::string Utf8EncodeSkipInvalid(std::wstring_view wide_string);
std::string Utf8EncodeStopOnInvalid(std::wstring_view wide_string,
                                    size_t* wchars_encoded);
std::wstring Utf8DecodeReplaceInvalid(std::string_view utf8_string);
std::wstring Utf8DecodeSkipInvalid(std::string_view utf8_string);
std::wstring Utf8DecodeStopOnInvalid(std::string_view utf8_string,
                                     size_t* bytes_decoded);

}  // namespace unicpp
