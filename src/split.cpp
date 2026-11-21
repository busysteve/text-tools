#include "split.hpp"

namespace breakout {
namespace {

bool is_space(unsigned char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
}

bool two_spaces_at(std::string_view text, size_t i) {
  return i + 1 < text.size() && text[i] == ' ' && text[i + 1] == ' ';
}

size_t run_end(std::string_view text, size_t i, char mark) {
  size_t j = i;
  while (j < text.size() && text[j] == mark) ++j;
  return j;
}

void emit(std::string& out, std::string& body, std::string_view mark) {
  size_t a = 0;
  size_t b = body.size();
  while (a < b && is_space(static_cast<unsigned char>(body[a]))) ++a;
  while (b > a && is_space(static_cast<unsigned char>(body[b - 1]))) --b;
  if (a == b && mark.empty()) {
    body.clear();
    return;
  }
  out.append(body, a, b - a);
  out.append(mark);
  out.push_back('\n');
  body.clear();
}

}  // namespace

std::string split(std::string_view text, SplitOptions opt) {
  if (!opt.period && !opt.comma && !opt.colon && !opt.semicolon && !opt.double_space) {
    opt.period = true;
    opt.comma = true;
    opt.colon = true;
    opt.semicolon = true;
    opt.double_space = true;
  }
  std::string out;
  std::string body;
  size_t i = 0;
  while (i < text.size()) {
    const char ch = text[i];
    if (opt.period && ch == '.') {
      const size_t j = run_end(text, i, '.');
      emit(out, body, text.substr(i, j - i));
      i = j;
      continue;
    }
    if (ch == '.') {
      const size_t j = run_end(text, i, '.');
      body.append(text.substr(i, j - i));
      i = j;
      continue;
    }
    if (opt.comma && ch == ',') {
      const size_t j = run_end(text, i, ',');
      emit(out, body, text.substr(i, j - i));
      i = j;
      continue;
    }
    if (opt.colon && ch == ':') {
      const size_t j = run_end(text, i, ':');
      emit(out, body, text.substr(i, j - i));
      i = j;
      continue;
    }
    if (opt.semicolon && ch == ';') {
      const size_t j = run_end(text, i, ';');
      emit(out, body, text.substr(i, j - i));
      i = j;
      continue;
    }
    if (opt.double_space && two_spaces_at(text, i)) {
      emit(out, body, std::string_view());
      i += 2;
      continue;
    }
    if (ch == '\n') {
      emit(out, body, std::string_view());
      ++i;
      continue;
    }
    if (ch == '\r') {
      ++i;
      continue;
    }
    body.push_back(ch);
    ++i;
  }
  emit(out, body, std::string_view());
  return out;
}

}  // namespace breakout
