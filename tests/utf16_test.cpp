#include "unicpp/utf16.h"

#include "gtest/gtest.h"

namespace unicpp {
namespace {


TEST(Utf16, Basic) {
  std::u32string_view text = U"\x10437";

  EXPECT_EQ(Utf16LeEncodeReplaceInvalid(text), "\x01\xD8\x37\xDC");
  EXPECT_EQ(Utf16BeEncodeReplaceInvalid(text), "\xD8\x01\xDC\x37");
}

}  // namespace
}  // namespace unicpp
