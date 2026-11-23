#include "chunk.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr char kUsage[] =
    "usage: splitter LINES\n"
    "       splitter LINES IN\n"
    "       splitter LINES IN DIR\n"
    "\n"
    "Read text from stdin or IN and write up to LINES lines to each file in DIR.\n"
    "DIR defaults to the current directory and is created when missing.\n"
    "A dash for IN is stdin.\n"
    "Files are named 0000000000.dat, 0000000001.dat, and so on.\n"
    "A line ends at a newline. A final partial line is kept.\n"
    "LINES is a positive integer. A file of the same name is replaced.\n"
    "Other files in DIR are left in place.\n";

bool parse_lines(const char* text, std::uint64_t* lines) {
  if (text == nullptr || *text < '0' || *text > '9') return false;
  errno = 0;
  char* end = nullptr;
  const unsigned long long value = std::strtoull(text, &end, 10);
  if (end == text || *end != '\0' || errno == ERANGE || value == 0) return false;
  *lines = static_cast<std::uint64_t>(value);
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
      std::fputs(kUsage, stdout);
      return 0;
    }
  }
  if (argc < 2 || argc > 4) {
    std::fputs(kUsage, stderr);
    return 1;
  }
  std::uint64_t lines = 0;
  if (!parse_lines(argv[1], &lines)) {
    std::fputs(kUsage, stderr);
    return 1;
  }
  try {
    const char* in_path = argc >= 3 ? argv[2] : nullptr;
    const std::filesystem::path dir = argc >= 4 ? argv[3] : ".";
    if (in_path == nullptr || std::strcmp(in_path, "-") == 0) {
      splitter::split(std::cin, dir, lines);
    } else {
      std::ifstream in(in_path, std::ios::binary);
      if (!in) throw std::runtime_error("read");
      splitter::split(in, dir, lines);
    }
  } catch (const std::exception& error) {
    std::fprintf(stderr, "splitter: %s\n", error.what());
    return 1;
  }
  return 0;
}
