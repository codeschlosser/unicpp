#pragma once

#include <array>
#include <cassert>
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

inline bool IsContinuationByte(uint8_t byte) {
  return (byte & 0xC0) == 0x80;
}

template <class InputIterator, class OutputIterator, bool kCheckBoundaries>
size_t Utf8DecodeImpl(InputIterator input, InputIterator input_end,
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
    uint8_t byte0 = *input;
    if (byte0 < 0x80) {
      *output = *input;
      ++input;
    } else if (byte0 < 0xE0) {
      if (byte0 < 0xC2) {
        // should have used 1 byte
        break;
      }
      if (std::distance(input, input_end) < 2) {
        break;
      }
      int byte1 = *std::next(input, 1);
      if (!IsContinuationByte(byte1)) {
        break;
      }
      std::advance(input, 2);
      *output = ((byte0 & 0x1F) << 6) | (byte1 & 0x3F);
    } else if (byte0 < 0xF0) {
      if (std::distance(input, input_end) < 3) {
        break;
      }
      int byte1 = *std::next(input, 1);
      if (!IsContinuationByte(byte1)) {
        break;
      }
      if ((byte0 & 0xF) == 0 && (byte1 & 0x20) == 0) {
        // should have used 2 bytes
        break;
      }
      int byte2 = *std::next(input, 2);
      if (!IsContinuationByte(byte2)) {
        break;
      }
      std::advance(input, 3);
      *output = ((byte0 & 0xF) << 12) | ((byte1 & 0x3F) << 6) | (byte2 & 0x3F);
    } else if (byte0 < 0xF5) {
      if ((byte0 & 0x7) > 0x4) {
        // bigger than of U+10FFFF
        break;
      }
      if (std::distance(input, input_end) < 4) {
        break;
      }
      int byte1 = *std::next(input, 1);
      if (!IsContinuationByte(byte1)) {
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

      int byte2 = *std::next(input, 2);
      if (!IsContinuationByte(byte2)) {
        break;
      }
      int byte3 = *std::next(input, 3);
      if (!IsContinuationByte(byte3)) {
        break;
      }
      std::advance(input, 4);
      *output = ((byte0 & 0x7) << 18) | ((byte1 & 0x3F) << 12) |
                ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
    } else {
      break;
    }
    ++output;
  }

  return total_size - static_cast<size_t>(std::distance(input, input_end));
}

template <class InputIterator, class OutputIterator>
size_t Utf8Decode(InputIterator input_beg, InputIterator input_end,
                  OutputIterator output) {
  return Utf8DecodeImpl<InputIterator, OutputIterator,
                        /*kCheckBoundaries = */ false>(input_beg, input_end,
                                                       output, output);
}

template <class InputIterator, class OutputIterator>
size_t Utf8Decode(InputIterator input_beg, InputIterator input_end,
                  OutputIterator output_beg, OutputIterator output_end) {
  return Utf8DecodeImpl<InputIterator, OutputIterator,
                        /*kCheckBoundaries = */ true>(input_beg, input_end,
                                                      output_beg, output_end);
}

template <class OutputIterator>
size_t Utf8Decode(std::string_view utf8_string, ErrorPolicy policy,
                  OutputIterator output) {
  std::string_view::iterator iter = utf8_string.begin();
  std::string_view::iterator end = utf8_string.end();
  while (iter != end) {
    size_t left = static_cast<size_t>(end - iter);
    size_t decoded = Utf8Decode(iter, end, output);
    if (decoded < left) {
      if (policy == ErrorPolicy::kSkip) {
        ++decoded;
      } else if (policy == ErrorPolicy::kStop) {
        iter += decoded;
        break;
      } else if (policy == ErrorPolicy::kReplace) {
        *output = kReplacementCharacter;
        ++output;
        ++decoded;
      } else {
        assert(0);
      }
    }
    iter += decoded;
  }

  return iter - utf8_string.begin();
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
  using iterator_category = std::input_iterator_tag;
  using value_type = char32_t;
  using difference_type = std::ptrdiff_t;
  using pointer = char32_t*;
  using reference = char32_t;

  Utf8DecodeIterator()
      : state_(nullptr)
      , current_char_(0) {}

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

  char32_t operator*() const {
    return current_char_;
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
    if (a.state_ != nullptr) {
      return a.state_ == b.state_ && a.current_char_ == b.current_char_;
    }
    return b.state_ == nullptr;
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
      size_t bytes_decoded = Utf8Decode(
          state_->buffer.begin(), state_->buffer.begin() + state_->valid_bytes,
          &current_char_, &current_char_ + 1);
      if (bytes_decoded > 0) {
        Advance(bytes_decoded);
        return;
      }
      if (state_->policy == ErrorPolicy::kSkip) {
        Advance(1);
        continue;
      } else if (state_->policy == ErrorPolicy::kReplace) {
        Advance(1);
        current_char_ = kReplacementCharacter;
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
  char32_t current_char_;
};

template <class CharStreamIterator>
class Utf8EncodeIterator {
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = uint8_t;
  using difference_type = std::ptrdiff_t;
  using pointer = uint8_t*;
  using reference = uint8_t;

  Utf8EncodeIterator()
      : state_(nullptr)
      , current_byte_(0) {}
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

  uint8_t operator*() const {
    return current_byte_;
  }

  Utf8EncodeIterator& operator++() {
    if (++state_->current_offset >= state_->valid_bytes) {
      NextChar();
    } else {
      current_byte_ = state_->buffer[state_->current_offset];
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
    if (a.state_ != nullptr) {
      return a.state_ == b.state_ && a.current_byte_ == b.current_byte_;
    }
    return b.state_ == nullptr;
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

    auto ptr = Utf8EncodeValidCharacter(ch, state_->buffer.begin());
    state_->valid_bytes = ptr - state_->buffer.begin();
    state_->current_offset = 0;
    current_byte_ = *state_->buffer.begin();
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
  uint8_t current_byte_;
};

size_t Utf8ValidPrefixLength(std::string_view utf8_string);
size_t Utf8NumValidChars(std::string_view utf8_string);
size_t Utf8NumCharsWithReplacement(std::string_view utf8_string);

std::string Utf8EncodeReplaceInvalid(std::u32string_view u32_string);
std::string Utf8EncodeSkipInvalid(std::u32string_view u32_string);
std::string Utf8EncodeStopOnInvalid(std::u32string_view u32_string,
                                    size_t* u32chars_encoded = nullptr);
std::u32string Utf8DecodeReplaceInvalid(std::string_view utf8_string);
std::u32string Utf8DecodeSkipInvalid(std::string_view utf8_string);
std::u32string Utf8DecodeStopOnInvalid(std::string_view utf8_string,
                                       size_t* bytes_decoded = nullptr);

}  // namespace unicpp
