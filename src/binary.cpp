#include "binary.hpp"

#include <string>
#include <string_view>

namespace binrm {
namespace {

bool keep(unsigned char c) {
  if (c >= 0x20 && c <= 0x7e) return true;
  return c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

}  // namespace

std::string strip(std::string_view bytes) {
  std::string out;
  out.reserve(bytes.size());
  for (unsigned char c : bytes) {
    if (keep(c)) out.push_back(static_cast<char>(c));
  }
  return out;
}

}  // namespace binrm
