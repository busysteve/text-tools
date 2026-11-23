#include "chunk.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace {

struct TempDir {
  std::filesystem::path path;

  TempDir() {
    static unsigned n = 0;
    const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    path = std::filesystem::temp_directory_path() /
           ("text-tools-splitter-" + std::to_string(ticks) + "-" + std::to_string(n++));
    std::filesystem::create_directories(path);
  }

  ~TempDir() {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
  }

  TempDir(const TempDir&) = delete;
  TempDir& operator=(const TempDir&) = delete;
};

std::string read_file(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("test read " + path.string());
  return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::vector<std::string> filenames(const std::filesystem::path& dir) {
  std::vector<std::string> names;
  for (const auto& ent : std::filesystem::directory_iterator(dir)) {
    names.push_back(ent.path().filename().string());
  }
  std::sort(names.begin(), names.end());
  return names;
}

std::uint64_t split_text(const std::filesystem::path& dir, std::string text,
                         std::uint64_t lines) {
  std::istringstream in(std::move(text), std::ios::in | std::ios::binary);
  return splitter::split(in, dir, lines);
}

template <class Ex, class F>
void expect_throw(F&& fn) {
  try {
    fn();
  } catch (const Ex&) {
    return;
  } catch (const std::exception& error) {
    throw std::runtime_error(std::string("unexpected exception: ") + error.what());
  }
  throw std::runtime_error("expected exception was not thrown");
}

#define CHECK(cond)                                                                          \
  do {                                                                                       \
    if (!(cond)) {                                                                           \
      throw std::runtime_error(std::string("CHECK failed: ") + __FILE__ + ":" +             \
                               std::to_string(__LINE__));                                    \
    }                                                                                        \
  } while (0)

#define CHECK_EQ(got, want)                                                                  \
  do {                                                                                       \
    const std::string _got = (got);                                                          \
    const std::string _want = (want);                                                        \
    if (_got != _want) {                                                                     \
      throw std::runtime_error(std::string("CHECK_EQ failed: ") + __FILE__ + ":" +          \
                               std::to_string(__LINE__) + "\ngot  [" + _got + "]\nwant [" + \
                               _want + "]");                                                 \
    }                                                                                        \
  } while (0)

void test_filename() {
  CHECK_EQ(splitter::filename(0), "0000000000.dat");
  CHECK_EQ(splitter::filename(1), "0000000001.dat");
  CHECK_EQ(splitter::filename(42), "0000000042.dat");
  CHECK_EQ(splitter::filename(9999999999ULL), "9999999999.dat");
  expect_throw<std::invalid_argument>([] { splitter::filename(10000000000ULL); });
}

void test_empty() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  CHECK(split_text(dir, "", 10) == 0);
  CHECK(filenames(dir).empty());
}

void test_exact_chunks() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  CHECK(split_text(dir, "a\nb\nc\nd\n", 2) == 2);
  CHECK_EQ(read_file(dir / "0000000000.dat"), "a\nb\n");
  CHECK_EQ(read_file(dir / "0000000001.dat"), "c\nd\n");
  CHECK(filenames(dir).size() == 2);
}

void test_partial_last_line() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  CHECK(split_text(dir, "a\nb\nc", 2) == 2);
  CHECK_EQ(read_file(dir / "0000000000.dat"), "a\nb\n");
  CHECK_EQ(read_file(dir / "0000000001.dat"), "c");
}

void test_no_newline() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  CHECK(split_text(dir, "abcd", 3) == 1);
  CHECK_EQ(read_file(dir / "0000000000.dat"), "abcd");
}

void test_blank_lines() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  CHECK(split_text(dir, "\n\n", 1) == 2);
  CHECK_EQ(read_file(dir / "0000000000.dat"), "\n");
  CHECK_EQ(read_file(dir / "0000000001.dat"), "\n");
}

void test_keeps_carriage_return() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  CHECK(split_text(dir, "a\r\nb\r\n", 1) == 2);
  CHECK_EQ(read_file(dir / "0000000000.dat"), "a\r\n");
  CHECK_EQ(read_file(dir / "0000000001.dat"), "b\r\n");
}

void test_embedded_nul() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  const std::string text("a\0b\nc\n", 6);
  CHECK(split_text(dir, text, 1) == 2);
  CHECK_EQ(read_file(dir / "0000000000.dat"), std::string("a\0b\n", 4));
  CHECK_EQ(read_file(dir / "0000000001.dat"), "c\n");
}

void test_one_line_each() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  CHECK(split_text(dir, "a\nb\nc\n", 1) == 3);
  CHECK_EQ(read_file(dir / "0000000000.dat"), "a\n");
  CHECK_EQ(read_file(dir / "0000000001.dat"), "b\n");
  CHECK_EQ(read_file(dir / "0000000002.dat"), "c\n");
}

void test_zero_lines() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  expect_throw<std::invalid_argument>([&] { split_text(dir, "a\n", 0); });
  CHECK(!std::filesystem::exists(dir));
}

void test_creates_directory() {
  TempDir tmp;
  const auto dir = tmp.path / "nested" / "out";
  CHECK(split_text(dir, "hi\n", 5) == 1);
  CHECK_EQ(read_file(dir / "0000000000.dat"), "hi\n");
}

void test_dir_is_file() {
  TempDir tmp;
  const auto file = tmp.path / "notdir";
  {
    std::ofstream out(file, std::ios::binary);
    out << "keep";
  }
  expect_throw<std::runtime_error>([&] { split_text(file, "a\n", 1); });
  CHECK_EQ(read_file(file), "keep");
}

void test_replaces_same_name_only() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  std::filesystem::create_directories(dir);
  {
    std::ofstream old_chunk(dir / "0000000000.dat", std::ios::binary);
    old_chunk << "old";
    std::ofstream extra(dir / "0000000001.dat", std::ios::binary);
    extra << "stale";
    std::ofstream note(dir / "keep.txt", std::ios::binary);
    note << "note";
  }
  CHECK(split_text(dir, "new\n", 10) == 1);
  CHECK_EQ(read_file(dir / "0000000000.dat"), "new\n");
  CHECK_EQ(read_file(dir / "0000000001.dat"), "stale");
  CHECK_EQ(read_file(dir / "keep.txt"), "note");
  for (const auto& name : filenames(dir)) {
    CHECK(name.find(".tmp") == std::string::npos);
  }
}

void test_long_line_crosses_buffer() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  std::string text(70000, 'a');
  text.push_back('\n');
  text.push_back('z');
  CHECK(split_text(dir, text, 1) == 2);
  CHECK_EQ(read_file(dir / "0000000000.dat"), std::string(70000, 'a') + "\n");
  CHECK_EQ(read_file(dir / "0000000001.dat"), "z");
}

void test_exact_buffer_size() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  std::string text;
  text.reserve(65536);
  for (int i = 0; i < 32768; ++i) text += "a\n";
  CHECK(text.size() == 65536);
  CHECK(split_text(dir, text, 16384) == 2);
  CHECK_EQ(read_file(dir / "0000000000.dat"), text.substr(0, 32768));
  CHECK_EQ(read_file(dir / "0000000001.dat"), text.substr(32768));
}

void test_many_chunks() {
  TempDir tmp;
  const auto dir = tmp.path / "out";
  constexpr int kLines = 7000;
  constexpr int kPer = 100;
  std::string text;
  text.reserve(static_cast<std::size_t>(kLines) * 11);
  for (int i = 0; i < kLines; ++i) {
    text += "0123456789\n";
  }
  CHECK(split_text(dir, text, kPer) == static_cast<std::uint64_t>(kLines / kPer));
  CHECK(filenames(dir).size() == static_cast<std::size_t>(kLines / kPer));
  std::string expect;
  expect.reserve(100 * 11);
  for (int i = 0; i < kPer; ++i) expect += "0123456789\n";
  CHECK_EQ(read_file(dir / "0000000000.dat"), expect);
  CHECK_EQ(read_file(dir / "0000000069.dat"), expect);
  CHECK(!std::filesystem::exists(dir / "0000000070.dat"));
}

}  // namespace

int main() {
  try {
    test_filename();
    test_empty();
    test_exact_chunks();
    test_partial_last_line();
    test_no_newline();
    test_blank_lines();
    test_keeps_carriage_return();
    test_embedded_nul();
    test_one_line_each();
    test_zero_lines();
    test_creates_directory();
    test_dir_is_file();
    test_replaces_same_name_only();
    test_long_line_crosses_buffer();
    test_exact_buffer_size();
    test_many_chunks();
  } catch (const std::exception& error) {
    std::fprintf(stderr, "splitter tests: %s\n", error.what());
    return 1;
  }
  std::printf("ok\n");
  return 0;
}
