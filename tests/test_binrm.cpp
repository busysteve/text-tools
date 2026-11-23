#include "binary.hpp"

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
  CHECK_EQ(binrm::strip(""), "");
  CHECK_EQ(binrm::strip("hello"), "hello");
  CHECK_EQ(binrm::strip(std::string("a\0b", 3)), "ab");
  CHECK_EQ(binrm::strip("ASCII\t\n\r"), "ASCII\t\n\r");
  CHECK_EQ(binrm::strip("a b"), "a b");
  CHECK_EQ(binrm::strip("a\vb\fc"), "a\vb\fc");
  CHECK_EQ(binrm::strip("~"), "~");
  CHECK_EQ(binrm::strip("\x7F"), "");
  CHECK_EQ(binrm::strip("a\x07" "b"), "ab");
  CHECK_EQ(binrm::strip("a\x1B" "b"), "ab");
  CHECK_EQ(binrm::strip("a\x08" "b"), "ab");
  CHECK_EQ(binrm::strip("caf\xC3\xA9"), "caf");
  CHECK_EQ(binrm::strip("\xE2\x82\xAC" "12"), "12");
  CHECK_EQ(binrm::strip("\xF0\x9F\x98\x80" "!"), "!");
  CHECK_EQ(binrm::strip("a\xFF" "b"), "ab");
  CHECK_EQ(binrm::strip("\x80"), "");
  CHECK_EQ(binrm::strip("ok\xC3\xA9!"), "ok!");
  CHECK_EQ(binrm::strip("\xEF\xBB\xBF" "hi"), "hi");
  CHECK_EQ(binrm::strip(" !\"#$%&'()*+,-./0123456789:;<=>?"),
           " !\"#$%&'()*+,-./0123456789:;<=>?");
  CHECK_EQ(binrm::strip("@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"),
           "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_");
  CHECK_EQ(binrm::strip("`abcdefghijklmnopqrstuvwxyz{|}~"),
           "`abcdefghijklmnopqrstuvwxyz{|}~");

  std::printf("ok\n");
  return 0;
}
