#include "chunk.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace splitter {
namespace {

constexpr std::uint64_t kNameMax = 9999999999ULL;

class ChunkWriter {
 public:
  ChunkWriter(std::filesystem::path dir, std::uint64_t lines_per_file)
      : dir_(std::move(dir)), lines_per_file_(lines_per_file) {}

  ChunkWriter(const ChunkWriter&) = delete;
  ChunkWriter& operator=(const ChunkWriter&) = delete;

  ~ChunkWriter() {
    if (!open_) return;
    out_.close();
    std::error_code ec;
    std::filesystem::remove(tmp_, ec);
  }

  void write(const char* data, std::streamsize n) {
    std::streamsize i = 0;
    while (i < n) {
      if (!open_) open_file();
      const std::uint64_t room = lines_per_file_ - lines_in_file_;
      std::streamsize j = i;
      std::uint64_t newlines = 0;
      while (j < n && newlines < room) {
        if (data[j] == '\n') ++newlines;
        ++j;
      }
      out_.write(data + i, j - i);
      if (!out_) fail_write();
      lines_in_file_ += newlines;
      i = j;
      if (lines_in_file_ == lines_per_file_) close_file();
    }
  }

  std::uint64_t finish() {
    if (open_) close_file();
    return index_;
  }

 private:
  void open_file() {
    if (index_ > kNameMax) throw std::runtime_error("write");
    const std::string name = filename(index_);
    dest_ = dir_ / name;
    tmp_ = std::filesystem::path(dest_.string() + ".tmp");
    out_.open(tmp_, std::ios::binary | std::ios::trunc);
    if (!out_) {
      std::error_code ec;
      std::filesystem::remove(tmp_, ec);
      throw std::runtime_error("write");
    }
    open_ = true;
    lines_in_file_ = 0;
  }

  void close_file() {
    out_.flush();
    if (!out_) fail_write();
    out_.close();
    if (out_.fail()) {
      open_ = false;
      std::error_code ec;
      std::filesystem::remove(tmp_, ec);
      throw std::runtime_error("write");
    }
    open_ = false;
    std::error_code ec;
    // Rename into place so a failed write does not leave a short .dat.
    std::filesystem::rename(tmp_, dest_, ec);
    if (ec) {
      std::filesystem::remove(tmp_, ec);
      throw std::runtime_error("write");
    }
    lines_in_file_ = 0;
    ++index_;
  }

  [[noreturn]] void fail_write() {
    if (out_.is_open()) out_.close();
    open_ = false;
    std::error_code ec;
    std::filesystem::remove(tmp_, ec);
    throw std::runtime_error("write");
  }

  std::filesystem::path dir_;
  std::filesystem::path dest_;
  std::filesystem::path tmp_;
  std::ofstream out_;
  std::uint64_t lines_per_file_ = 0;
  std::uint64_t index_ = 0;
  std::uint64_t lines_in_file_ = 0;
  bool open_ = false;
};

}  // namespace

std::string filename(std::uint64_t index) {
  if (index > kNameMax) throw std::invalid_argument("index");
  std::ostringstream out;
  out << std::setfill('0') << std::setw(10) << index << ".dat";
  return out.str();
}

std::uint64_t split(std::istream& in, const std::filesystem::path& dir,
                    std::uint64_t lines_per_file) {
  if (lines_per_file == 0) throw std::invalid_argument("lines");
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (ec) throw std::runtime_error("write");
  if (!std::filesystem::is_directory(dir, ec) || ec) throw std::runtime_error("write");

  ChunkWriter writer(dir, lines_per_file);
  char buf[1 << 16];
  while (in) {
    in.read(buf, sizeof(buf));
    const std::streamsize n = in.gcount();
    if (n > 0) writer.write(buf, n);
  }
  if (in.bad()) throw std::runtime_error("read");
  return writer.finish();
}

}  // namespace splitter
