#include "ascii.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

#define CHECK_EQ(got, want)                                                                   \
  do {                                                                                        \
    const std::string _got = (got);                                                           \
    const std::string _want = (want);                                                         \
    if (_got != _want) {                                                                      \
      std::fprintf(stderr, "CHECK_EQ failed: %s:%d\ngot  [%s]\nwant [%s]\n", __FILE__,        \
                   __LINE__, _got.c_str(), _want.c_str());                                    \
      std::exit(1);                                                                           \
    }                                                                                         \
  } while (0)

int main() {
  CHECK_EQ(utf8rm::to_ascii(""), "");
  CHECK_EQ(utf8rm::to_ascii("hello"), "hello");
  CHECK_EQ(utf8rm::to_ascii(std::string("a\0b", 3)), std::string("a\0b", 3));
  CHECK_EQ(utf8rm::to_ascii("caf\xC3\xA9"), "caf");
  CHECK_EQ(utf8rm::to_ascii("\xE2\x82\xAC" "12"), "12");
  CHECK_EQ(utf8rm::to_ascii("\xF0\x9F\x98\x80" "!"), "!");
  CHECK_EQ(utf8rm::to_ascii("\xEF\xBB\xBF" "hi"), "hi");
  CHECK_EQ(utf8rm::to_ascii("a\xFF" "b"), "ab");
  CHECK_EQ(utf8rm::to_ascii("\x80"), "");
  CHECK_EQ(utf8rm::to_ascii("ASCII\t\n\r"), "ASCII\t\n\r");
  CHECK_EQ(utf8rm::to_ascii("~"), "~");
  CHECK_EQ(utf8rm::to_ascii("\x7F"), "\x7F");
  CHECK_EQ(utf8rm::to_ascii("ok\xC3\xA9!"), "ok!");
  CHECK_EQ(utf8rm::to_ascii("\xC0\x80" "a"), "a");
  CHECK_EQ(utf8rm::to_ascii("\xED\xA0\x80" "x"), "x");
  CHECK_EQ(utf8rm::to_ascii("na\xC3\xAF" "ve"), "nave");

  std::printf("ok\n");
  return 0;
}
