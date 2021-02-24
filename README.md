# unicpp: Just another C++ Unicode library

## Character manipulation and category detection functions (`unicpp/char_type.h`)
```cpp
char32_t toupper(char32_t);
char32_t tolower(char32_t);
bool isalpha(char32_t);
bool isdigit(char32_t);
```

## UTF-8 encode/decode functions (`unicpp/utf8.h`)
U+FFFD is used as replacement character when invalid character/byte sequence is encountered

### Strings validation/stats functions
```cpp
size_t Utf8ValidPrefixLength(std::string_view);
size_t Utf8NumValidChars(std::string_view);
size_t Utf8NumCharsWithReplacement(std::string_view);
```

### Encoding/decoding functions
```cpp
std::string Utf8EncodeReplaceInvalid(std::wstring_view);
std::string Utf8EncodeSkipInvalid(std::wstring_view);
std::string Utf8EncodeStopOnInvalid(std::wstring_view, size_t* wchars_encoded);

std::wstring Utf8DecodeReplaceInvalid(std::string_view);
std::wstring Utf8DecodeSkipInvalid(std::string_view);
std::wstring Utf8DecodeStopOnInvalid(std::string_view, size_t* bytes_decoded);
```
