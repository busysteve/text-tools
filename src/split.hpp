#pragma once

#include <string>
#include <string_view>

namespace breakout {

struct SplitOptions {
  bool period = false;
  bool comma = false;
  bool colon = false;
  bool semicolon = false;
  bool double_space = false;
};

// Put each sentence on its own line. Newlines always split. With no options
// every mark is used: period, comma, colon, semicolon, and two spaces. If any
// option is set, only those marks split. Punctuation is kept. Blank lines and
// surrounding whitespace are dropped.
std::string split(std::string_view text, SplitOptions opt = {});

}  // namespace breakout
