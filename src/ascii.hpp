#pragma once

#include <string>
#include <string_view>

namespace utf8rm {

// Keep ASCII characters (U+0000..U+007F). Each well-formed non-ASCII UTF-8
// character is dropped as a unit. Ill-formed bytes are dropped.
std::string to_ascii(std::string_view bytes);

}  // namespace utf8rm
