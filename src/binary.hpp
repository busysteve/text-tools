#pragma once

#include <string>
#include <string_view>

namespace binrm {

// Keep carriage return, line feed, whitespace, and printable ASCII
// (U+0020..U+007E). Other bytes are dropped.
std::string strip(std::string_view bytes);

}  // namespace binrm
