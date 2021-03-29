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

size_t Utf8ValidPrefixLength(std::string_view utf8_string) {
  return Utf8Decode(utf8_string.begin(), utf8_string.end(),
                    NopOutputIterator());
}

size_t Utf8NumValidChars(std::string_view utf8_string) {
  AssignmentCouterOutputIterator counter;
  Utf8Decode(utf8_string.begin(), utf8_string.end(), counter,
             ErrorPolicy::kStop);

  return counter.count();
}

size_t Utf8NumCharsWithReplacement(std::string_view utf8_string) {
  AssignmentCouterOutputIterator counter;
  Utf8Decode(utf8_string.begin(), utf8_string.end(), counter,
             ErrorPolicy::kReplace);

  return counter.count();
}

}  // namespace unicpp
