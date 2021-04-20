#include "unicpp/utf8.h"

#include "tools/cpp/runfiles/runfiles.h"

#include "gtest/gtest.h"

#include <fstream>
#include <vector>

namespace unicpp {
namespace {

using bazel::tools::cpp::runfiles::Runfiles;

std::vector<char> LoadDataFile(const std::string& filepath) {
  std::string error;
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
  if (runfiles == nullptr) {
    throw std::runtime_error("Couldn't load file: " + filepath);
  }

  std::string text_path = runfiles->Rlocation(filepath);
  std::ifstream fin(text_path, std::ios_base::binary);

  fin.seekg(0, std::ios_base::end);
  std::vector<char> buffer(fin.tellg());
  fin.seekg(0, std::ios_base::beg);
  fin.read(&buffer[0], buffer.size());

  return buffer;
}

TEST(Utf8, Ascii) {
  std::string_view text = "Hello, world!";
  std::u32string plain_copy(text.begin(), text.end());
  std::u32string decoded = Utf8Wstring<std::u32string>(text);

  EXPECT_EQ(plain_copy, decoded);
}

TEST(Utf8, DecodeAndEncode) {
  std::vector<char> data = LoadDataFile("unicpp/tests/data/utf8_text.txt");
  std::string_view view(&data[0], data.size());

  std::u32string decoded = Utf8Wstring<std::u32string>(view);
  ASSERT_FALSE(decoded.empty());

  std::string encoded = Utf8Bytes<std::string>(decoded);

  EXPECT_EQ(view, encoded);
}

TEST(Utf8, DecodeExotic) {
  std::string_view encoded = "\xe0\xa0\x80";

  std::u32string decoded = Utf8Wstring<std::u32string>(encoded);

  std::u32string expected = U"\x800";

  EXPECT_EQ(decoded, expected);

  std::string_view U100000_encoded = "\xF4\x80\x80\x80";

  std::u32string U100000_decoded = Utf8Wstring<std::u32string>(U100000_encoded);

  std::u32string U100000(1, 0x100000);

  EXPECT_EQ(U100000_decoded, U100000);
}

TEST(Utf8, NotBigEnoughChar) {
  std::string_view U100000_encoded = "\xF4\x80\x80\x80";
  std::u16string u16_U100000 = Utf8Wstring<std::u16string>(U100000_encoded);
  EXPECT_EQ(u16_U100000, std::u16string(1, kReplacementCharacter));

  if constexpr (sizeof(wchar_t) <= 2) {
    std::wstring w_U100000 = Utf8Wstring<std::wstring>(U100000_encoded);
    EXPECT_EQ(w_U100000, std::wstring(1, kReplacementCharacter));
  }
}

TEST(Utf8, DecodeInvalid) {
  std::string_view kThreeErrorsEncoded = "\xE0\x90\x80";
  const std::u32string kThreeErrorsDecoded = U"\xFFFD\xFFFD\xFFFD";

  EXPECT_EQ(Utf8Wstring<std::u32string>(kThreeErrorsEncoded),
            kThreeErrorsDecoded);

  std::string_view kTwoErrorsEncoded = "\xC1\x80!";
  const std::u32string kTwoErrorsDecoded = U"\xFFFD\xFFFD!";

  EXPECT_EQ(Utf8Wstring<std::u32string>(kTwoErrorsEncoded), kTwoErrorsDecoded);

  std::string_view kErrorInMidEncoded = "\xE0\xA0\x80\xE0\x90!";
  const std::u32string kErrorInMidDecoded = U"\x800\xFFFD\xFFFD!";

  EXPECT_EQ(Utf8Wstring<std::u32string>(kErrorInMidEncoded),
            kErrorInMidDecoded);

  std::string_view kNotEnoughEncoded = "\xF0";
  const std::u32string kNotEnoughDecoded = U"\xFFFD";

  EXPECT_EQ(Utf8Wstring<std::u32string>(kNotEnoughEncoded), kNotEnoughDecoded);
}

TEST(Utf8, Validation) {
  EXPECT_EQ(Utf8ValidPrefixLength("\xE0\xA0\x80\xE0"), 3);

  EXPECT_EQ(Utf8NumValidChars("\xE0\xA0\x80\xE0"), 1);

  EXPECT_EQ(Utf8NumCharsWithReplacement("\xE0\xA0\x80\xE0"), 2);

  std::string_view valid_ascii_text = "Hello, world!";
  EXPECT_EQ(Utf8NumValidChars(valid_ascii_text), valid_ascii_text.length());
}

TEST(Utf8, DecodeIterator) {
  std::string_view one_char_valid = "A\x80Z";

  Utf8DecodeIterator stopper(one_char_valid.begin(), one_char_valid.end(),
                             ErrorPolicy::kStop);
  ASSERT_TRUE(stopper);
  EXPECT_EQ(*stopper, U'A');
  EXPECT_FALSE(++stopper);

  Utf8DecodeIterator replacer(one_char_valid.begin(), one_char_valid.end(),
                              ErrorPolicy::kReplace);
  ASSERT_TRUE(replacer);
  EXPECT_EQ(*replacer, U'A');
  ASSERT_TRUE(++replacer);
  EXPECT_EQ(*replacer, kReplacementCharacter);
  ASSERT_TRUE(++replacer);
  EXPECT_EQ(*replacer, U'Z');
  EXPECT_FALSE(++replacer);

  Utf8DecodeIterator skipper(one_char_valid.begin(), one_char_valid.end(),
                             ErrorPolicy::kSkip);
  ASSERT_TRUE(skipper);
  EXPECT_EQ(*skipper, U'A');
  ASSERT_TRUE(++skipper);
  EXPECT_EQ(*skipper, U'Z');
  EXPECT_FALSE(++skipper);
}

TEST(Utf8, EncodeIterator) {
  char32_t text[] = {U'A', kInvalidCharacter, U'Z', U'\0'};
  std::u32string_view one_char_valid = text;

  Utf8EncodeIterator stopper(one_char_valid.begin(), one_char_valid.end(),
                             ErrorPolicy::kStop);
  ASSERT_TRUE(stopper);
  EXPECT_EQ(*stopper, 'A');
  EXPECT_FALSE(++stopper);

  Utf8EncodeIterator replacer(one_char_valid.begin(), one_char_valid.end(),
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

  Utf8EncodeIterator skipper(one_char_valid.begin(), one_char_valid.end(),
                             ErrorPolicy::kSkip);
  ASSERT_TRUE(skipper);
  EXPECT_EQ(*skipper, 'A');
  ASSERT_TRUE(++skipper);
  EXPECT_EQ(*skipper, 'Z');
  EXPECT_FALSE(++skipper);
}

TEST(Utf8, Iterators) {
  std::vector<char> data = LoadDataFile("unicpp/tests/data/utf8_text.txt");

  Utf8DecodeIterator decoder(data.begin(), data.end(), ErrorPolicy::kStop);
  Utf8EncodeIterator encoder(decoder, decltype(decoder)(), ErrorPolicy::kStop);

  std::vector<char> buffer;
  std::copy(encoder, decltype(encoder)(), std::back_insert_iterator(buffer));

  ASSERT_EQ(data.size(), buffer.size());

  for (size_t i = 0; i < data.size(); i++) {
    EXPECT_EQ(data[i], buffer[i]) << " i = " << i;
  }
}

}  // namespace
}  // namespace unicpp
