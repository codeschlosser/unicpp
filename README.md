# unicpp: Just another C++ Unicode library

## Character manipulation and category detection functions (`unicpp/char_type.h`)
```cpp
bool isalpha(char32_t);
bool isdigit(char32_t);
bool isspace(char32_t);

// one-to-one case mappings
char32_t toupper(char32_t);
char32_t tolower(char32_t);
```

## UTF-8 encode/decode functions (`unicpp/utf8.h`)
&#xFFFD; (U+FFFD) is used as a replacement character when invalid character/byte sequence is encountered

### Strings validation/stats functions
```cpp
size_t Utf8ValidPrefixLength(std::string_view);
size_t Utf8NumValidChars(std::string_view);
size_t Utf8NumCharsWithReplacement(std::string_view);
```

### Encoding/decoding functions
```cpp
// UTF-8
const std::wstring wide_string = L"Some string";

std::vector<uint8_t> encoded_utf8 = Utf8Bytes<std::vector<uint8_t>>(wide_string);
std::wstring decoded_utf8 = Utf8Wstring<std::wstring>(encoded_utf8);
assert(wide_string == decoded_utf8);

// UTF-16LE
std::vector<uint8_t> encoded_utf16le = Utf16LeBytes<std::vector<uint8_t>>(wide_string);
std::wstring decoded_utf16le = Utf16LeWstring<std::wstring>(encoded_utf16le);
assert(wide_string == decoded_utf16le);

// UTF-16BE
std::vector<uint8_t> encoded_utf16be = Utf16BeBytes<std::vector<uint8_t>>(wide_string);
std::wstring decoded_utf16be = Utf16BeWstring<std::wstring>(encoded_utf16be);
assert(wide_string == decoded_utf16be);
```
