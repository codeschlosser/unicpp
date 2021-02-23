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
};

}  // namespace

size_t Utf8ValidCharacters(std::string_view utf8_string) {
  AssignmentCouterOutputIterator counter;
  Utf8Decode((const uint8_t*)utf8_string.data(),
             (const uint8_t*)utf8_string.data() + utf8_string.length(),
             counter);

  return counter.count();
}

size_t Utf8ValidPrefix(std::string_view utf8_string) {
  return Utf8Decode((const uint8_t*)utf8_string.data(),
                    (const uint8_t*)utf8_string.data() + utf8_string.length(),
                    NopOutputIterator());
}

std::string Utf8EncodeReplaceInvalid(std::wstring_view wide_string) {
  std::string result;
  std::back_insert_iterator<std::string> inserter(result);
  Utf8Encode(wide_string, ErrorPolicy::kReplace, inserter);
  return result;
}

std::string Utf8EncodeSkipInvalid(std::wstring_view wide_string) {
  std::string result;
  std::back_insert_iterator<std::string> inserter(result);
  Utf8Encode(wide_string, ErrorPolicy::kSkip, inserter);
  return result;
}

std::string Utf8EncodeStopOnInvalid(std::wstring_view wide_string,
                                    size_t* wchars_encoded) {
  std::string result;
  std::back_insert_iterator<std::string> inserter(result);
  size_t encoded = Utf8Encode(wide_string, ErrorPolicy::kReplace, inserter);
  if (wchars_encoded != nullptr) {
    *wchars_encoded = encoded;
  }
  return result;
}

std::wstring Utf8DecodeReplaceInvalid(std::string_view utf8_string) {
  std::wstring result;
  std::back_insert_iterator<std::wstring> inserter(result);
  Utf8Decode(utf8_string, ErrorPolicy::kReplace, inserter);

  return result;
}

std::wstring Utf8DecodeSkipInvalid(std::string_view utf8_string) {
  std::wstring result;
  std::back_insert_iterator<std::wstring> inserter(result);
  Utf8Decode(utf8_string, ErrorPolicy::kSkip, inserter);

  return result;
}

std::wstring Utf8DecodeStopOnInvalid(std::string_view utf8_string,
                                     size_t* bytes_decoded) {
  std::wstring result;
  std::back_insert_iterator<std::wstring> inserter(result);
  size_t decoded = Utf8Decode(utf8_string, ErrorPolicy::kSkip, inserter);
  if (bytes_decoded != nullptr) {
    *bytes_decoded = decoded;
  }

  return result;
}

}  // namespace unicpp
