#include "utf8.h"

namespace unicpp {
namespace {

class AssignmentCouterOutputIterator {
public:
  AssignmentCouterOutputIterator()
      : counter_(std::make_shared<size_t>(0)) {}
  AssignmentCouterOutputIterator& operator=(char32_t) {
    (*counter_)++;
    return *this;
  }
  AssignmentCouterOutputIterator& operator*() {
    return *this;
  }
  AssignmentCouterOutputIterator& operator++() {
    return *this;
  }
  AssignmentCouterOutputIterator operator++(int) {
    AssignmentCouterOutputIterator tmp(*this);
    operator++();
    return tmp;
  }

  size_t count() const {
    return *counter_;
  }

  friend bool operator==(const AssignmentCouterOutputIterator& a,
                         const AssignmentCouterOutputIterator& b) {
    return &a == &b;
  }
  friend bool operator!=(const AssignmentCouterOutputIterator& a,
                         const AssignmentCouterOutputIterator& b) {
    return !(a == b);
  }

private:
  std::shared_ptr<size_t> counter_;
};

class NopOutputIterator {
public:
  NopOutputIterator& operator=(char32_t) {
    return *this;
  }
  NopOutputIterator& operator*() {
    return *this;
  }
  NopOutputIterator& operator++() {
    return *this;
  }
  NopOutputIterator operator++(int) {
    NopOutputIterator tmp(*this);
    operator++();
    return tmp;
  }

  friend bool operator==(const NopOutputIterator& a,
                         const NopOutputIterator& b) {
    return &a == &b;
  }
  friend bool operator!=(const NopOutputIterator& a,
                         const NopOutputIterator& b) {
    return !(a == b);
  }
};

}  // namespace

size_t Utf8ValidPrefixLength(std::string_view utf8_string) {
  return Utf8Decode(utf8_string.begin(), utf8_string.end(),
                    NopOutputIterator());
}

size_t Utf8NumValidChars(std::string_view utf8_string) {
  AssignmentCouterOutputIterator counter;
  Utf8Decode<AssignmentCouterOutputIterator, ErrorPolicy::kStop>(utf8_string,
                                                                 counter);

  return counter.count();
}

size_t Utf8NumCharsWithReplacement(std::string_view utf8_string) {
  AssignmentCouterOutputIterator counter;
  Utf8Decode<AssignmentCouterOutputIterator, ErrorPolicy::kReplace>(utf8_string,
                                                                    counter);

  return counter.count();
}

std::string Utf8EncodeReplaceInvalid(std::u32string_view u32_string) {
  std::string result;
  Utf8Encode<char32_t, std::back_insert_iterator<std::string>,
             ErrorPolicy::kReplace>(u32_string,
                                    std::back_insert_iterator(result));
  return result;
}

std::string Utf8EncodeSkipInvalid(std::u32string_view u32_string) {
  std::string result;
  Utf8Encode<char32_t, std::back_insert_iterator<std::string>,
             ErrorPolicy::kSkip>(u32_string, std::back_insert_iterator(result));
  return result;
}

std::string Utf8EncodeStopOnInvalid(std::u32string_view u32_string,
                                    size_t* u32chars_encoded) {
  std::string result;
  size_t encoded = Utf8Encode<char32_t, std::back_insert_iterator<std::string>,
                              ErrorPolicy::kStop>(
      u32_string, std::back_insert_iterator(result));
  if (u32chars_encoded != nullptr) {
    *u32chars_encoded = encoded;
  }
  return result;
}

std::u32string Utf8DecodeReplaceInvalid(std::string_view utf8_string) {
  std::u32string result;
  Utf8Decode<std::back_insert_iterator<std::u32string>, ErrorPolicy::kReplace>(
      utf8_string, std::back_insert_iterator(result));

  return result;
}

std::u32string Utf8DecodeSkipInvalid(std::string_view utf8_string) {
  std::u32string result;
  Utf8Decode<std::back_insert_iterator<std::u32string>, ErrorPolicy::kSkip>(
      utf8_string, std::back_insert_iterator(result));

  return result;
}

std::u32string Utf8DecodeStopOnInvalid(std::string_view utf8_string,
                                       size_t* bytes_decoded) {
  std::u32string result;
  std::back_insert_iterator<std::u32string> inserter(result);
  size_t decoded =
      Utf8Decode<std::back_insert_iterator<std::u32string>, ErrorPolicy::kStop>(
          utf8_string, inserter);
  if (bytes_decoded != nullptr) {
    *bytes_decoded = decoded;
  }

  return result;
}

}  // namespace unicpp
