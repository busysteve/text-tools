#include "ascii.hpp"

#include <string>
#include <string_view>

namespace utf8rm {
namespace {

// Bytes in the next well-formed UTF-8 character, or 0 if data[0] cannot start
// one. A truncated tail is ill-formed.
int sequence_length(const unsigned char* data, size_t n) {
  if (n == 0) return 0;
  const unsigned char b0 = data[0];
  if (b0 <= 0x7f) return 1;
  if (b0 <= 0xbf) return 0;
  if (b0 <= 0xdf) {
    if (n < 2) return 0;
    const unsigned char b1 = data[1];
    if ((b1 & 0xc0) != 0x80) return 0;
    if (b0 < 0xc2) return 0;
    return 2;
  }
  if (b0 <= 0xef) {
    if (n < 3) return 0;
    const unsigned char b1 = data[1];
    const unsigned char b2 = data[2];
    if ((b1 & 0xc0) != 0x80 || (b2 & 0xc0) != 0x80) return 0;
    if (b0 == 0xe0 && b1 < 0xa0) return 0;
    if (b0 == 0xed && b1 >= 0xa0) return 0;
    return 3;
  }
  if (b0 <= 0xf4) {
    if (n < 4) return 0;
    const unsigned char b1 = data[1];
    const unsigned char b2 = data[2];
    const unsigned char b3 = data[3];
    if ((b1 & 0xc0) != 0x80 || (b2 & 0xc0) != 0x80 || (b3 & 0xc0) != 0x80) return 0;
    if (b0 == 0xf0 && b1 < 0x90) return 0;
    if (b0 == 0xf4 && b1 >= 0x90) return 0;
    return 4;
  }
  return 0;
}

}  // namespace

std::string to_ascii(std::string_view bytes) {
  std::string out;
  out.reserve(bytes.size());
  const auto* data = reinterpret_cast<const unsigned char*>(bytes.data());
  size_t i = 0;
  const size_t n = bytes.size();
  while (i < n) {
    const int len = sequence_length(data + i, n - i);
    if (len == 1 && data[i] <= 0x7f) {
      out.push_back(static_cast<char>(data[i]));
      ++i;
      continue;
    }
    // Non-ASCII character or an ill-formed byte: drop it.
    i += len == 0 ? 1 : static_cast<size_t>(len);
  }
  return out;
}

}  // namespace utf8rm
