#pragma once

#include <string>
#include <string_view>

namespace html2txt {

// Visible text of an HTML document. Script, style, comments, and tags are
// dropped. Entities are decoded. Block tags become line breaks.
std::string extract(std::string_view html);

}  // namespace html2txt
