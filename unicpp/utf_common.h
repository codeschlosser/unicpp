#pragma once

#include <iterator>
#include <limits>
#include <memory>

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

template <class Container>
class CheckedBackInsertIterator {
public:
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using pointer = void;
  using reference = void;
  using difference_type = void;

  constexpr CheckedBackInsertIterator() noexcept = default;

  constexpr explicit CheckedBackInsertIterator(Container& container) noexcept
      : container_(std::addressof(container)) {}

  constexpr CheckedBackInsertIterator& operator=(char32_t ch) {
    static_assert(sizeof(typename Container::value_type) >= 2);
    static_assert(sizeof(typename Container::value_type) % 2 == 0);
    if constexpr (sizeof(typename Container::value_type) == 2) {
      if (ch > std::numeric_limits<typename Container::value_type>::max()) {
        ch = kReplacementCharacter;
      }
    }
    container_->push_back(ch);
    return *this;
  }

  [[nodiscard]] constexpr CheckedBackInsertIterator& operator*() noexcept {
    return *this;
  }

  constexpr CheckedBackInsertIterator& operator++() noexcept {
    return *this;
  }

  constexpr CheckedBackInsertIterator operator++(int) noexcept {
    return *this;
  }

private:
  Container* container_ = nullptr;
};

template <class Container>
[[nodiscard]] constexpr CheckedBackInsertIterator<Container>
CheckedBackInserter(Container& container) noexcept {
  return CheckedBackInsertIterator<Container>(container);
}

}  // namespace unicpp
