#include "utf8.h"

namespace unicpp {

char32_t DecodeCharacter(const uint8_t*& ptr, const uint8_t* end) {
  int byte1 = *ptr++;
  if ((byte1 & 0x80) == 0) {
    return byte1;
  } else if ((byte1 & 0xE0) == 0xC0) {
    byte1 &= 0x1F;
    if (byte1 <= 1) {
      return kInvalidCharacter;
    }

    int byte2 = *ptr;
    if ((byte2 & 0xC0) != 0x80) {
      return kInvalidCharacter;
    }
    ptr++;
    return (byte1 << 6) | (byte2 & 0x3F);
  } else if ((byte1 & 0xF0) == 0xE0) {
    int byte2 = *ptr;
    if ((byte2 & 0xC0) != 0x80) {
      return kInvalidCharacter;
    }

    if ((byte1 & 0xF) == 0 && (byte2 & 0x20) == 0) {
      return kInvalidCharacter;
    }

    ptr++;

    int byte3 = *ptr;
    if ((byte3 & 0xC0) != 0x80) {
      return kInvalidCharacter;
    }
    ptr++;

    return ((byte1 & 0xF) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
  } else if ((byte1 & 0xF8) == 0xF0) {
    if ((byte1 & 0x7) > 0x4) {
      return kInvalidCharacter;
    }

    int byte2 = *ptr;
    if ((byte2 & 0xC0) != 0x80) {
      return kInvalidCharacter;
    }

    if ((byte1 & 0x7) == 0 && (byte2 & 0x30) == 0) {
      return kInvalidCharacter;
    }

    if ((byte1 & 0x7) == 0x4 && (byte2 & 0x30) > 0) {
      return kInvalidCharacter;
    }

    ptr++;

    int byte3 = *ptr;
    if ((byte3 & 0xC0) != 0x80) {
      return kInvalidCharacter;
    }

    ptr++;

    int byte4 = *ptr;
    if ((byte4 & 0xC0) != 0x80) {
      return kInvalidCharacter;
    }
    ptr++;

    return ((byte1 & 0x7) << 18) | ((byte2 & 0x3F) << 12) |
           ((byte3 & 0x3F) << 6) | (byte4 & 0x3F);
  } else {
    return kInvalidCharacter;
  }
}

}  // namespace unicpp
