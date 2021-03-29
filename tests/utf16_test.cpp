#include "unicpp/utf16.h"

#include "gtest/gtest.h"

namespace unicpp {
namespace {


TEST(Utf16, Basic) {
  std::u32string_view text = U"\x10437";

  EXPECT_EQ(Utf16LeBytes<std::string>(text), "\x01\xD8\x37\xDC");
  EXPECT_EQ(Utf16BeBytes<std::string>(text), "\xD8\x01\xDC\x37");
}

TEST(Utf16, EncodeAndDecode) {
  std::wstring_view text = L"Hello, world";

  {
    std::string bytes_le = Utf16LeBytes<std::string>(text);
    std::wstring wstr = Utf16LeWstring<std::wstring>(bytes_le);
    EXPECT_EQ(wstr, text);
  }
  {
    std::string bytes_be = Utf16BeBytes<std::string>(text);
    std::wstring wstr = Utf16BeWstring<std::wstring>(bytes_be);
    EXPECT_EQ(wstr, text);
  }
}

}  // namespace
}  // namespace unicpp
