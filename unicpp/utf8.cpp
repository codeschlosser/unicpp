#include "utf8.h"

namespace unicpp {

Utf8DecodeCharacterResult Utf8DecodeCharacter(const uint8_t* data,
                                              size_t size) noexcept {
  if (size == 0) {
    return {kInvalidCharacter, 0};
  }
  int byte0 = data[0];
  if ((byte0 & 0x80) == 0) {
    return {(char32_t)byte0, 1};
  } else if ((byte0 & 0xE0) == 0xC0) {
    byte0 &= 0x1F;
    if (byte0 <= 1) {
      return {kInvalidCharacter, 0};
    }

    if (size < 2) {
      return {kInvalidCharacter, 1};
    }

    int byte1 = data[1];
    if ((byte1 & 0xC0) != 0x80) {
      return {kInvalidCharacter, 1};
    }

    return {(char32_t)((byte0 << 6) | (byte1 & 0x3F)), 2};
  } else if ((byte0 & 0xF0) == 0xE0) {
    if (size < 3) {
      return {kInvalidCharacter, 1};
    }
    int byte1 = data[1];
    if ((byte1 & 0xC0) != 0x80) {
      return {kInvalidCharacter, 1};
    }

    if ((byte0 & 0xF) == 0 && (byte1 & 0x20) == 0) {
      return {kInvalidCharacter, 1};
    }

    int byte2 = data[2];
    if ((byte2 & 0xC0) != 0x80) {
      return {kInvalidCharacter, 2};
    }

    return {(char32_t)(((byte0 & 0xF) << 12) | ((byte1 & 0x3F) << 6) |
                       (byte2 & 0x3F)),
            3};
  } else if ((byte0 & 0xF8) == 0xF0) {
    if ((byte0 & 0x7) > 0x4) {
      return {kInvalidCharacter, 0};
    }
    if (size < 4) {
      return {kInvalidCharacter, 1};
    }

    int byte1 = data[1];
    if ((byte1 & 0xC0) != 0x80) {
      return {kInvalidCharacter, 1};
    }

    if ((byte0 & 0x7) == 0 && (byte1 & 0x30) == 0) {
      return {kInvalidCharacter, 1};
    }

    if ((byte0 & 0x7) == 0x4 && (byte1 & 0x30) > 0) {
      return {kInvalidCharacter, 1};
    }

    int byte2 = data[2];
    if ((byte2 & 0xC0) != 0x80) {
      return {kInvalidCharacter, 2};
    }

    int byte3 = data[3];
    if ((byte3 & 0xC0) != 0x80) {
      return {kInvalidCharacter, 3};
    }

    return {(char32_t)(((byte0 & 0x7) << 18) | ((byte1 & 0x3F) << 12) |
                       ((byte2 & 0x3F) << 6) | (byte3 & 0x3F)),
            4};
  } else {
    return {kInvalidCharacter, 0};
  }
}

}  // namespace unicpp
