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
}

TEST(Utf8, DecodeInvalid) {
  const char kThreeErrorsEncoded[] = "\xE0\x90\x80";
  const std::wstring kThreeErrorsDecoded = L"\xFFFD\xFFFD\xFFFD";

  EXPECT_EQ(Utf8DecodeReplaceInvalid(kThreeErrorsEncoded), kThreeErrorsDecoded);

  const char kTwoErrorsEncoded[] = "\xc1\x80!";
  const std::wstring kTwoErrorsDecoded = L"\xFFFD\xFFFD!";

  EXPECT_EQ(Utf8DecodeReplaceInvalid(kTwoErrorsEncoded), kTwoErrorsDecoded);

  const char kErrorInMidEncoded[] = "\xe0\xa0\x80\xE0\x90!";
  const std::wstring kErrorInMidDecoded = L"\x800\xFFFD\xFFFD!";

  EXPECT_EQ(Utf8DecodeReplaceInvalid(kErrorInMidEncoded), kErrorInMidDecoded);
}

}  // namespace
}  // namespace unicpp
