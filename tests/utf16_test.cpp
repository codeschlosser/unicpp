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

TEST(Utf16, NotBigEnoughChar) {
  {
    std::string_view U100000_encoded("\xC0\xDB\x00\xDC", 4);
    std::u16string u16_U100000 =
        Utf16LeWstring<std::u16string>(U100000_encoded);
    EXPECT_EQ(u16_U100000, std::u16string(1, kReplacementCharacter));

    if constexpr (sizeof(wchar_t) <= 2) {
      std::wstring w_U100000 = Utf16LeWstring<std::wstring>(U100000_encoded);
      EXPECT_EQ(w_U100000, std::wstring(1, kReplacementCharacter));
    }
  }
  {
    std::string_view U100000_encoded("\xDB\xC0\xDC\x00", 4);
    std::u16string u16_U100000 =
        Utf16BeWstring<std::u16string>(U100000_encoded);
    EXPECT_EQ(u16_U100000, std::u16string(1, kReplacementCharacter));

    if constexpr (sizeof(wchar_t) <= 2) {
      std::wstring w_U100000 = Utf16BeWstring<std::wstring>(U100000_encoded);
      EXPECT_EQ(w_U100000, std::wstring(1, kReplacementCharacter));
    }
  }
}

}  // namespace
}  // namespace unicpp
