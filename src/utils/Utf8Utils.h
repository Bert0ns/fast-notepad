#pragma once

namespace Utf8Utils {
  // Returns the byte length of a UTF-8 character given its first byte.
  // Returns 1 for invalid continuation bytes to gracefully skip them.
  int CharLength(unsigned char c);
}
