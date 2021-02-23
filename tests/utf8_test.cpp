#include "unicpp/utf8.h"

#include "tools/cpp/runfiles/runfiles.h"

#include "gtest/gtest.h"

#include <fstream>
#include <vector>

namespace unicpp {
namespace {

using bazel::tools::cpp::runfiles::Runfiles;

TEST(Utf8, Ascii) {
  std::string_view text = "Hello, world!";
  std::wstring plain_copy(text.begin(), text.end());
  std::wstring decoded = Utf8DecodeStopOnInvalid(text, nullptr);

  EXPECT_EQ(plain_copy, decoded);
}

TEST(Utf8, DecodeAndEncode) {
  std::string error;
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
  ASSERT_NE(runfiles, nullptr);

  std::string text_path =
      runfiles->Rlocation("unicpp/tests/data/utf8_text.txt");
  std::ifstream fin(text_path, std::ios_base::binary);

  fin.seekg(0, std::ios_base::end);
  size_t byte_count = fin.tellg();
  fin.seekg(0, std::ios_base::beg);
  std::vector<char> buffer(byte_count + 1);
  fin.read(&buffer[0], byte_count);

  std::wstring decoded = Utf8DecodeStopOnInvalid(&buffer[0], nullptr);
  ASSERT_FALSE(decoded.empty());

  std::string encoded = Utf8EncodeReplaceInvalid(decoded);

  std::string_view initial = &buffer[0];
  std::string_view changed = encoded.c_str();

  EXPECT_EQ(initial, changed);
}

TEST(Utf8, DecodeExotic) {
  const char encoded[] = "\xe0\xa0\x80";

  std::wstring decoded = Utf8DecodeReplaceInvalid(encoded);

  std::wstring expected = L"\x800";

  EXPECT_EQ(decoded, expected);

  if constexpr (sizeof(wchar_t) > 2) {
    const char U100000_encoded[] = "\xF4\x80\x80\x80";

    std::wstring U100000_decoded = Utf8DecodeReplaceInvalid(U100000_encoded);

    std::wstring U100000(1, 0x100000);

    EXPECT_EQ(U100000_decoded, U100000);
  }
}

TEST(Utf8, DecodeInvalid) {
  const char kThreeErrorsEncoded[] = "\xE0\x90\x80";
  const std::wstring kThreeErrorsDecoded = L"\xFFFD\xFFFD\xFFFD";

  EXPECT_EQ(Utf8DecodeReplaceInvalid(kThreeErrorsEncoded), kThreeErrorsDecoded);

  const char kTwoErrorsEncoded[] = "\xC1\x80!";
  const std::wstring kTwoErrorsDecoded = L"\xFFFD\xFFFD!";

  EXPECT_EQ(Utf8DecodeReplaceInvalid(kTwoErrorsEncoded), kTwoErrorsDecoded);

  const char kErrorInMidEncoded[] = "\xE0\xA0\x80\xE0\x90!";
  const std::wstring kErrorInMidDecoded = L"\x800\xFFFD\xFFFD!";

  EXPECT_EQ(Utf8DecodeReplaceInvalid(kErrorInMidEncoded), kErrorInMidDecoded);

  const char kNotEnoughEncoded[] = "\xF0";
  const std::wstring kNotEnoughDecoded = L"\xFFFD";

  EXPECT_EQ(Utf8DecodeReplaceInvalid(kNotEnoughEncoded), kNotEnoughDecoded);
}

TEST(Utf8, Validation) {
  EXPECT_EQ(Utf8ValidCharacters("\xE0\xA0\x80\xE0"), 1);

  std::string_view valid_ascii_text =
      "All human beings are born free and equal in dignity and rights.";
  EXPECT_EQ(Utf8ValidCharacters(valid_ascii_text), valid_ascii_text.length());

  EXPECT_EQ(Utf8ValidPrefix("\xE0\xA0\x80\xE0"), 3);
}

TEST(Utf8, DecodeIterator) {
  std::string_view one_char_valid = "A\x80Z";
  DecodeIterator stopper(one_char_valid.begin(), one_char_valid.end(),
                         ErrorPolicy::kStop);
  ASSERT_TRUE(stopper);
  EXPECT_EQ(*stopper, L'A');
  EXPECT_FALSE(++stopper);

  DecodeIterator replacer(one_char_valid.begin(), one_char_valid.end(),
                          ErrorPolicy::kReplace);
  ASSERT_TRUE(replacer);
  EXPECT_EQ(*replacer, L'A');
  ASSERT_TRUE(++replacer);
  EXPECT_EQ(*replacer, kReplacementCharacter);
  ASSERT_TRUE(++replacer);
  EXPECT_EQ(*replacer, L'Z');
  EXPECT_FALSE(++replacer);

  DecodeIterator skipper(one_char_valid.begin(), one_char_valid.end(),
                         ErrorPolicy::kSkip);
  ASSERT_TRUE(skipper);
  EXPECT_EQ(*skipper, L'A');
  ASSERT_TRUE(++skipper);
  EXPECT_EQ(*skipper, L'Z');
  EXPECT_FALSE(++skipper);
}

TEST(Utf8, EncodeIterator) {
  char32_t text[] = {L'A', kMaxValidCharacter + 1, L'Z', L'\0'};
  std::u32string_view one_char_valid = text;
  EncodeIterator stopper(one_char_valid.begin(), one_char_valid.end(),
                         ErrorPolicy::kStop);
  ASSERT_TRUE(stopper);
  EXPECT_EQ(*stopper, 'A');
  EXPECT_FALSE(++stopper);

  EncodeIterator replacer(one_char_valid.begin(), one_char_valid.end(),
                          ErrorPolicy::kReplace);
  ASSERT_TRUE(replacer);
  EXPECT_EQ(*replacer, 'A');
  // encoded kReplacementCharacter == {0xEF,0xBF,0xBD}
  ASSERT_TRUE(++replacer);
  EXPECT_EQ(*replacer, 0xEF);
  ASSERT_TRUE(++replacer);
  EXPECT_EQ(*replacer, 0xBF);
  ASSERT_TRUE(++replacer);
  EXPECT_EQ(*replacer, 0xBD);
  ASSERT_TRUE(++replacer);
  EXPECT_EQ(*replacer, 'Z');
  EXPECT_FALSE(++replacer);

  EncodeIterator skipper(one_char_valid.begin(), one_char_valid.end(),
                         ErrorPolicy::kSkip);
  ASSERT_TRUE(skipper);
  EXPECT_EQ(*skipper, 'A');
  ASSERT_TRUE(++skipper);
  EXPECT_EQ(*skipper, 'Z');
  EXPECT_FALSE(++skipper);
}

}  // namespace
}  // namespace unicpp
