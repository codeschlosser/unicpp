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
std::string Utf8EncodeReplaceInvalid(std::u32string_view u32_string);
std::string Utf8EncodeSkipInvalid(std::u32string_view u32_string);
std::string Utf8EncodeStopOnInvalid(std::u32string_view u32_string,
                                    size_t* u32chars_encoded);

std::u32string Utf8DecodeReplaceInvalid(std::string_view utf8_string);
std::u32string Utf8DecodeSkipInvalid(std::string_view utf8_string);
std::u32string Utf8DecodeStopOnInvalid(std::string_view utf8_string,
                                       size_t* bytes_decoded);
```
