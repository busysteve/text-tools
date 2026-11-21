#include "split.hpp"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr char kUsage[] =
    "usage: breakout [options] [IN [OUT]]\n"
    "\n"
    "Read text from stdin or IN and write one sentence per line to stdout or OUT.\n"
    "Newlines always split. With no options every mark is used. If any option is\n"
    "given, only those marks split. Punctuation is kept. A dash for IN or OUT is\n"
    "stdin or stdout.\n"
    "\n"
    "  --period         break after a period\n"
    "  --comma          break after a comma\n"
    "  --colon          break after a colon\n"
    "  --semicolon      break after a semicolon\n"
    "  --double-space   break on two spaces\n";

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
  breakout::SplitOptions opt;
  std::vector<const char*> paths;
  for (int i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--help") == 0) {
      std::fputs(kUsage, stdout);
      return 0;
    }
    if (std::strcmp(arg, "--period") == 0) {
      opt.period = true;
    } else if (std::strcmp(arg, "--comma") == 0) {
      opt.comma = true;
    } else if (std::strcmp(arg, "--colon") == 0) {
      opt.colon = true;
    } else if (std::strcmp(arg, "--semicolon") == 0) {
      opt.semicolon = true;
    } else if (std::strcmp(arg, "--double-space") == 0) {
      opt.double_space = true;
    } else if (arg[0] == '-' && arg[1] != '\0') {
      std::fputs(kUsage, stderr);
      return 1;
    } else {
      paths.push_back(arg);
    }
  }
  if (paths.size() > 2) {
    std::fputs(kUsage, stderr);
    return 1;
  }
  try {
    const char* in_path = paths.size() >= 1 ? paths[0] : nullptr;
    const char* out_path = paths.size() >= 2 ? paths[1] : nullptr;
    const std::string text = breakout::split(read_input(in_path), opt);
    write_output(out_path, text);
  } catch (const std::runtime_error& error) {
    std::fprintf(stderr, "breakout: %s\n", error.what());
    return 1;
  }
  return 0;
}
