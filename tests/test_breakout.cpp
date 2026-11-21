#include "split.hpp"

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
  CHECK_EQ(breakout::split(""), "");
  CHECK_EQ(breakout::split("Hello."), "Hello.\n");
  CHECK_EQ(breakout::split("Hello. World."), "Hello.\nWorld.\n");
  CHECK_EQ(breakout::split("Hello.  World."), "Hello.\nWorld.\n");
  CHECK_EQ(breakout::split("one, two, three"), "one,\ntwo,\nthree\n");
  CHECK_EQ(breakout::split("Note: value: x"), "Note:\nvalue:\nx\n");
  CHECK_EQ(breakout::split("a; b; c"), "a;\nb;\nc\n");
  CHECK_EQ(breakout::split("Hello  World  Next"), "Hello\nWorld\nNext\n");
  CHECK_EQ(breakout::split("a: b; c, d.  e"), "a:\nb;\nc,\nd.\ne\n");
  CHECK_EQ(breakout::split("Hello.\nWorld."), "Hello.\nWorld.\n");
  CHECK_EQ(breakout::split("Hello.\n\nWorld."), "Hello.\nWorld.\n");
  CHECK_EQ(breakout::split("Hello\nWorld"), "Hello\nWorld\n");
  CHECK_EQ(breakout::split("No period"), "No period\n");
  CHECK_EQ(breakout::split("Wait..."), "Wait...\n");
  CHECK_EQ(breakout::split("Wait... Go."), "Wait...\nGo.\n");
  CHECK_EQ(breakout::split("."), ".\n");
  CHECK_EQ(breakout::split("..."), "...\n");
  CHECK_EQ(breakout::split("\n\n"), "");
  CHECK_EQ(breakout::split("\r\n"), "");
  CHECK_EQ(breakout::split("Hello.\r\nWorld."), "Hello.\nWorld.\n");
  CHECK_EQ(breakout::split("A.\tB."), "A.\nB.\n");
  CHECK_EQ(breakout::split("caf\xC3\xA9. Next."), "caf\xC3\xA9.\nNext.\n");
  CHECK_EQ(breakout::split("3.14"), "3.\n14\n");
  CHECK_EQ(breakout::split("Mr. Smith."), "Mr.\nSmith.\n");

  breakout::SplitOptions period;
  period.period = true;
  CHECK_EQ(breakout::split("Hello. World.", period), "Hello.\nWorld.\n");
  CHECK_EQ(breakout::split("Mr. Smith.", period), "Mr.\nSmith.\n");
  CHECK_EQ(breakout::split("3.14", period), "3.\n14\n");
  CHECK_EQ(breakout::split("one, two", period), "one, two\n");
  CHECK_EQ(breakout::split("a: b; c", period), "a: b; c\n");
  CHECK_EQ(breakout::split("Hello  World", period), "Hello  World\n");

  breakout::SplitOptions comma;
  comma.comma = true;
  CHECK_EQ(breakout::split("one, two, three", comma), "one,\ntwo,\nthree\n");
  CHECK_EQ(breakout::split("Hello.  World, next.", comma), "Hello.  World,\nnext.\n");
  CHECK_EQ(breakout::split(",", comma), ",\n");

  breakout::SplitOptions colon;
  colon.colon = true;
  CHECK_EQ(breakout::split("Note: value: x", colon), "Note:\nvalue:\nx\n");
  CHECK_EQ(breakout::split("Hello. World.", colon), "Hello. World.\n");

  breakout::SplitOptions semi;
  semi.semicolon = true;
  CHECK_EQ(breakout::split("a; b; c", semi), "a;\nb;\nc\n");

  breakout::SplitOptions spaces;
  spaces.double_space = true;
  CHECK_EQ(breakout::split("Hello  World  Next", spaces), "Hello\nWorld\nNext\n");
  CHECK_EQ(breakout::split("Hello.  World", spaces), "Hello.\nWorld\n");
  CHECK_EQ(breakout::split("a   b", spaces), "a\nb\n");
  CHECK_EQ(breakout::split("one, two", spaces), "one, two\n");

  breakout::SplitOptions mix;
  mix.comma = true;
  mix.colon = true;
  mix.semicolon = true;
  CHECK_EQ(breakout::split("a: b; c, d.  e", mix), "a:\nb;\nc,\nd.  e\n");

  std::printf("ok\n");
  return 0;
}
