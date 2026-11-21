#include "extract.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

#define CHECK(cond)                                                                  \
  do {                                                                               \
    if (!(cond)) {                                                                   \
      std::fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      std::exit(1);                                                                  \
    }                                                                                \
  } while (0)

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
  CHECK_EQ(html2txt::extract(""), "");
  CHECK_EQ(html2txt::extract("hello"), "hello\n");
  CHECK_EQ(html2txt::extract("  foo   bar  "), "foo bar\n");
  CHECK_EQ(html2txt::extract("<p>hello</p>"), "hello\n");
  CHECK_EQ(html2txt::extract("<p>a</p><p>b</p>"), "a\n\nb\n");
  CHECK_EQ(html2txt::extract("<b>bold</b>text"), "boldtext\n");
  CHECK_EQ(html2txt::extract("<b>bold</b> text"), "bold text\n");
  CHECK_EQ(html2txt::extract("a&amp;b"), "a&b\n");
  CHECK_EQ(html2txt::extract("&lt;tag&gt;"), "<tag>\n");
  CHECK_EQ(html2txt::extract("a&nbsp;b"), "a b\n");
  CHECK_EQ(html2txt::extract("&#65;&#x42;"), "AB\n");
  CHECK_EQ(html2txt::extract("&copy; 2020"), "\xC2\xA9 2020\n");
  CHECK_EQ(html2txt::extract("<script>alert(1)</script>ok"), "ok\n");
  CHECK_EQ(html2txt::extract("<SCRIPT>x</SCRIPT>y"), "y\n");
  CHECK_EQ(html2txt::extract("<style>p{color:red}</style>Hi"), "Hi\n");
  CHECK_EQ(html2txt::extract("<!-- comment -->visible"), "visible\n");
  CHECK_EQ(html2txt::extract("a<br>b"), "a\nb\n");
  CHECK_EQ(html2txt::extract("a<br/>b"), "a\nb\n");
  CHECK_EQ(html2txt::extract("<div>a</div><div>b</div>"), "a\n\nb\n");
  CHECK_EQ(html2txt::extract("<li>a</li><li>b</li>"), "a\nb\n");
  CHECK_EQ(html2txt::extract("<pre>  a\n  b</pre>"), "  a\n  b\n");
  CHECK_EQ(html2txt::extract("<pre>  a\r\n  b</pre>"), "  a\n  b\n");
  CHECK_EQ(html2txt::extract("<img alt='hi'>"), "hi\n");
  CHECK_EQ(html2txt::extract("say <img alt=\"cat &amp; dog\">."), "say cat & dog.\n");
  CHECK_EQ(html2txt::extract("1 < 2"), "1 < 2\n");
  CHECK_EQ(html2txt::extract("<table><tr><td>a</td><td>b</td></tr></table>"), "a b\n");
  CHECK_EQ(html2txt::extract("<html><head><title>T</title><style>x</style></head>"
                             "<body><p>Hi</p></body></html>"),
           "T\n\nHi\n");
  CHECK_EQ(html2txt::extract("<noscript>hidden</noscript>ok"), "ok\n");
  CHECK_EQ(html2txt::extract("a<!DOCTYPE html>b"), "ab\n");
  CHECK_EQ(html2txt::extract("<![CDATA[raw <tag>]]>"), "raw <tag>\n");
  CHECK_EQ(html2txt::extract("caf&eacute;"), "caf\xC3\xA9\n");
  CHECK_EQ(html2txt::extract("plain"), "plain\n");
  CHECK_EQ(html2txt::extract("<p></p>"), "");
  CHECK_EQ(html2txt::extract("\xEF\xBB\xBFhello"), "hello\n");
  CHECK_EQ(html2txt::extract("<textarea>a<b>c</textarea>"), "a<b>c\n");
  CHECK_EQ(html2txt::extract("<p>a</P><P>b</p>"), "a\n\nb\n");
  CHECK_EQ(html2txt::extract("&unknown;"), "&unknown;\n");
  CHECK_EQ(html2txt::extract("a<!-- unclosed"), "a\n");
  CHECK_EQ(html2txt::extract("caf\xC3\xA9"), "caf\xC3\xA9\n");
  CHECK_EQ(html2txt::extract("<p>\xE4\xBD\xA0\xE5\xA5\xBD</p>"), "\xE4\xBD\xA0\xE5\xA5\xBD\n");
  CHECK_EQ(html2txt::extract("<p \xE4\xB8\xAD>hello</p>"), "hello\n");
  CHECK_EQ(html2txt::extract("<img alt=\"caf\xC3\xA9\">"), "caf\xC3\xA9\n");
  CHECK_EQ(html2txt::extract("<div \"x\">y</div>"), "y\n");
  CHECK_EQ(html2txt::extract("<span title=\"na\xC3\xAF" "ve\">ok</span>"), "ok\n");

  std::printf("ok\n");
  return 0;
}
