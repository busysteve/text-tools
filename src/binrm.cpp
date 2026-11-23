#include "binary.hpp"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr char kUsage[] =
    "usage: binrm\n"
    "       binrm IN\n"
    "       binrm IN OUT\n"
    "\n"
    "Read bytes from stdin or IN and write ASCII text to stdout or OUT.\n"
    "Binary bytes are dropped. Carriage return, line feed, whitespace, and\n"
    "printable ASCII are kept. A dash for IN or OUT is stdin or stdout.\n";

std::string read_all(std::istream& in) {
  std::string bytes;
  char buf[1 << 16];
  while (in) {
    in.read(buf, sizeof(buf));
    bytes.append(buf, static_cast<size_t>(in.gcount()));
  }
  if (in.bad()) throw std::runtime_error("read");
  return bytes;
}

std::string read_input(const char* path) {
  if (path == nullptr || std::strcmp(path, "-") == 0) {
    return read_all(std::cin);
  }
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("read");
  return read_all(in);
}

void write_all(std::ostream& out, std::string_view text) {
  if (!text.empty()) {
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
  }
  out.flush();
  if (!out) throw std::runtime_error("write");
}

void write_output(const char* path, std::string_view text) {
  if (path == nullptr || std::strcmp(path, "-") == 0) {
    write_all(std::cout, text);
    return;
  }
  const std::filesystem::path dest(path);
  const std::filesystem::path tmp = dest.string() + ".tmp";
  {
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("write");
    try {
      write_all(out, text);
    } catch (...) {
      out.close();
      std::filesystem::remove(tmp);
      throw;
    }
  }
  try {
    std::filesystem::rename(tmp, dest);
  } catch (...) {
    std::filesystem::remove(tmp);
    throw std::runtime_error("write");
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc == 2 && (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0)) {
    std::fputs(kUsage, stdout);
    return 0;
  }
  if (argc > 3) {
    std::fputs(kUsage, stderr);
    return 1;
  }
  try {
    const char* in_path = argc >= 2 ? argv[1] : nullptr;
    const char* out_path = argc >= 3 ? argv[2] : nullptr;
    const std::string text = binrm::strip(read_input(in_path));
    write_output(out_path, text);
  } catch (const std::runtime_error& error) {
    std::fprintf(stderr, "binrm: %s\n", error.what());
    return 1;
  }
  return 0;
}
