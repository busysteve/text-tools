#pragma once

#include <cstdint>
#include <filesystem>
#include <istream>
#include <string>

namespace splitter {

// "0000000000.dat" through "9999999999.dat".
std::string filename(std::uint64_t index);

// Split `in` into files of at most `lines_per_file` lines in `dir`.
// Files are named filename(0), filename(1), and so on. A line ends at
// '\n'. A final partial line counts as one line. Bytes are copied
// unchanged. `lines_per_file` must be greater than zero. Returns the
// number of files written. Empty input writes none.
std::uint64_t split(std::istream& in, const std::filesystem::path& dir,
                    std::uint64_t lines_per_file);

}  // namespace splitter
