#include "extract.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace html2txt {
namespace {

struct Entity {
  const char* name;
  uint32_t cp;
};

// Sorted by name. nbsp and the other width spaces decode to ASCII space so
// they collapse with ordinary whitespace.
constexpr Entity kEntities[] = {
    {"AElig", 0x00C6},  {"Aacute", 0x00C1}, {"Acirc", 0x00C2},  {"Agrave", 0x00C0},
    {"Aring", 0x00C5},  {"Atilde", 0x00C3}, {"Auml", 0x00C4},   {"Ccedil", 0x00C7},
    {"Dagger", 0x2021}, {"ETH", 0x00D0},    {"Eacute", 0x00C9}, {"Ecirc", 0x00CA},
    {"Egrave", 0x00C8}, {"Euml", 0x00CB},   {"Iacute", 0x00CD}, {"Icirc", 0x00CE},
    {"Igrave", 0x00CC}, {"Iuml", 0x00CF},   {"Ntilde", 0x00D1}, {"OElig", 0x0152},
    {"Oacute", 0x00D3}, {"Ocirc", 0x00D4},  {"Ograve", 0x00D2}, {"Oslash", 0x00D8},
    {"Otilde", 0x00D5}, {"Ouml", 0x00D6},   {"Scaron", 0x0160}, {"THORN", 0x00DE},
    {"Uacute", 0x00DA}, {"Ucirc", 0x00DB},  {"Ugrave", 0x00D9}, {"Uuml", 0x00DC},
    {"Yacute", 0x00DD}, {"Yuml", 0x0178},   {"aacute", 0x00E1}, {"acirc", 0x00E2},
    {"acute", 0x00B4},  {"aelig", 0x00E6},  {"agrave", 0x00E0}, {"amp", 0x0026},
    {"apos", 0x0027},   {"aring", 0x00E5},  {"atilde", 0x00E3}, {"auml", 0x00E4},
    {"bdquo", 0x201E},  {"brvbar", 0x00A6}, {"bull", 0x2022},   {"ccedil", 0x00E7},
    {"cedil", 0x00B8},  {"cent", 0x00A2},   {"circ", 0x02C6},   {"copy", 0x00A9},
    {"curren", 0x00A4}, {"dagger", 0x2020}, {"deg", 0x00B0},    {"divide", 0x00F7},
    {"eacute", 0x00E9}, {"ecirc", 0x00EA},  {"egrave", 0x00E8}, {"emsp", 0x0020},
    {"ensp", 0x0020},   {"eth", 0x00F0},    {"euml", 0x00EB},   {"euro", 0x20AC},
    {"frac12", 0x00BD}, {"frac14", 0x00BC}, {"frac34", 0x00BE}, {"gt", 0x003E},
    {"harr", 0x2194},   {"hellip", 0x2026}, {"iacute", 0x00ED}, {"icirc", 0x00EE},
    {"iexcl", 0x00A1},  {"igrave", 0x00EC}, {"iquest", 0x00BF}, {"iuml", 0x00EF},
    {"laquo", 0x00AB},  {"larr", 0x2190},   {"ldquo", 0x201C},  {"lsaquo", 0x2039},
    {"lsquo", 0x2018},  {"lt", 0x003C},     {"macr", 0x00AF},   {"mdash", 0x2014},
    {"micro", 0x00B5},  {"middot", 0x00B7}, {"nbsp", 0x0020},   {"ndash", 0x2013},
    {"not", 0x00AC},    {"ntilde", 0x00F1}, {"oacute", 0x00F3}, {"ocirc", 0x00F4},
    {"oelig", 0x0153},  {"ograve", 0x00F2}, {"ordf", 0x00AA},   {"ordm", 0x00BA},
    {"oslash", 0x00F8}, {"otilde", 0x00F5}, {"ouml", 0x00F6},   {"para", 0x00B6},
    {"permil", 0x2030}, {"plusmn", 0x00B1}, {"pound", 0x00A3},  {"quot", 0x0022},
    {"raquo", 0x00BB},  {"rarr", 0x2192},   {"rdquo", 0x201D},  {"reg", 0x00AE},
    {"rsaquo", 0x203A}, {"rsquo", 0x2019},  {"sbquo", 0x201A},  {"scaron", 0x0161},
    {"sect", 0x00A7},   {"shy", 0x0000},    {"sup1", 0x00B9},   {"sup2", 0x00B2},
    {"sup3", 0x00B3},   {"szlig", 0x00DF},  {"thinsp", 0x0020}, {"thorn", 0x00FE},
    {"tilde", 0x02DC},  {"times", 0x00D7},  {"trade", 0x2122},  {"uacute", 0x00FA},
    {"uarr", 0x2191},   {"ucirc", 0x00FB},  {"ugrave", 0x00F9}, {"uml", 0x00A8},
    {"uuml", 0x00FC},   {"yacute", 0x00FD}, {"yen", 0x00A5},    {"yuml", 0x00FF},
};

bool is_ascii_space(unsigned char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

bool is_name_char(unsigned char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' ||
         c == ':';
}

bool is_alpha(unsigned char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

char ascii_lower(unsigned char c) {
  if (c >= 'A' && c <= 'Z') return static_cast<char>(c - 'A' + 'a');
  return static_cast<char>(c);
}

bool in_sorted(const std::string_view* begin, const std::string_view* end, std::string_view name) {
  return std::binary_search(begin, end, name);
}

constexpr std::string_view kSkip[] = {"math", "noscript", "script", "style", "svg", "template"};
constexpr std::string_view kRcdata[] = {"textarea", "title"};
constexpr std::string_view kTight[] = {"dd", "dt", "li", "tr"};
constexpr std::string_view kCell[] = {"td", "th"};
constexpr std::string_view kBlock[] = {
    "address",    "article", "aside",  "blockquote", "body",        "caption", "center",
    "dd",         "details", "div",    "dl",         "dt",          "figcaption", "figure",
    "footer",     "form",    "h1",     "h2",         "h3",          "h4",      "h5",
    "h6",         "header",  "hr",     "html",       "li",          "main",    "nav",
    "ol",         "p",       "pre",    "section",    "table",       "tbody",   "tfoot",
    "thead",      "tr",      "ul",
};

void append_utf8(std::string& out, uint32_t cp) {
  if (cp == 0) return;
  if (cp >= 0xD800 && cp <= 0xDFFF) cp = 0xFFFD;
  if (cp > 0x10FFFF) cp = 0xFFFD;
  if (cp < 0x80) {
    out.push_back(static_cast<char>(cp));
  } else if (cp < 0x800) {
    out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
  } else if (cp < 0x10000) {
    out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
    out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
  } else {
    out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
    out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
  }
}

const Entity* find_entity(std::string_view name) {
  const Entity* begin = std::begin(kEntities);
  const Entity* end = std::end(kEntities);
  const Entity* it = std::lower_bound(begin, end, name, [](const Entity& ent, std::string_view n) {
    return std::string_view(ent.name) < n;
  });
  if (it != end && std::string_view(it->name) == name) return it;
  return nullptr;
}

class Extractor {
 public:
  explicit Extractor(std::string_view html) : html_(html), n_(html.size()) {}

  std::string run() {
    if (n_ >= 3 && static_cast<unsigned char>(html_[0]) == 0xef &&
        static_cast<unsigned char>(html_[1]) == 0xbb &&
        static_cast<unsigned char>(html_[2]) == 0xbf) {
      i_ = 3;
    }
    out_.reserve(html_.size());
    while (i_ < n_) {
      if (html_[i_] == '<' && is_markup_start(i_)) parse_markup();
      else consume_text_char();
    }
    if (!out_.empty() && out_.back() != '\n') out_.push_back('\n');
    return out_;
  }

 private:
  bool is_markup_start(size_t at) const {
    if (at + 1 >= n_) return false;
    const unsigned char c = static_cast<unsigned char>(html_[at + 1]);
    return is_alpha(c) || c == '/' || c == '!' || c == '?';
  }

  void skip_ws() {
    while (i_ < n_ && is_ascii_space(static_cast<unsigned char>(html_[i_]))) ++i_;
  }

  std::string read_name() {
    std::string name;
    while (i_ < n_ && is_name_char(static_cast<unsigned char>(html_[i_]))) {
      name.push_back(ascii_lower(static_cast<unsigned char>(html_[i_])));
      ++i_;
    }
    return name;
  }

  uint32_t parse_entity() {
    const size_t start = i_;
    ++i_;
    if (i_ >= n_) return '&';

    if (html_[i_] == '#') {
      ++i_;
      uint32_t cp = 0;
      bool any = false;
      if (i_ < n_ && (html_[i_] == 'x' || html_[i_] == 'X')) {
        ++i_;
        while (i_ < n_) {
          const unsigned char c = static_cast<unsigned char>(html_[i_]);
          uint32_t d = 0;
          if (c >= '0' && c <= '9') d = c - '0';
          else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
          else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
          else break;
          any = true;
          if (cp <= 0x10FFFF / 16) cp = cp * 16 + d;
          else cp = 0x110000;
          ++i_;
        }
      } else {
        while (i_ < n_) {
          const unsigned char c = static_cast<unsigned char>(html_[i_]);
          if (c < '0' || c > '9') break;
          any = true;
          if (cp <= 0x10FFFF / 10) cp = cp * 10 + (c - '0');
          else cp = 0x110000;
          ++i_;
        }
      }
      if (!any) {
        i_ = start + 1;
        return '&';
      }
      if (i_ < n_ && html_[i_] == ';') ++i_;
      if (cp == 0 || cp > 0x10FFFF) return 0xFFFD;
      return cp;
    }

    if (!is_alpha(static_cast<unsigned char>(html_[i_]))) {
      return '&';
    }
    const size_t name_at = i_;
    while (i_ < n_ && is_name_char(static_cast<unsigned char>(html_[i_]))) ++i_;
    const std::string_view name = html_.substr(name_at, i_ - name_at);
    if (i_ < n_ && html_[i_] == ';') ++i_;
    else {
      i_ = start + 1;
      return '&';
    }
    if (const Entity* ent = find_entity(name)) return ent->cp;
    i_ = start + 1;
    return '&';
  }

  std::string read_attr_value() {
    std::string value;
    if (i_ >= n_) return value;
    const char quote = html_[i_];
    if (quote == '"' || quote == '\'') {
      ++i_;
      while (i_ < n_ && html_[i_] != quote) {
        if (html_[i_] == '&') append_utf8(value, parse_entity());
        else value.push_back(html_[i_++]);
      }
      if (i_ < n_) ++i_;
      return value;
    }
    while (i_ < n_) {
      const unsigned char c = static_cast<unsigned char>(html_[i_]);
      if (is_ascii_space(c) || c == '>') break;
      if (html_[i_] == '&') append_utf8(value, parse_entity());
      else value.push_back(html_[i_++]);
    }
    return value;
  }

  void emit_byte(unsigned char c) {
    if (c == '\r') c = '\n';
    if (pre_depth_ > 0) {
      if (c == '\n') {
        emit_newline_raw();
        return;
      }
      out_.push_back(static_cast<char>(c));
      at_line_start_ = false;
      newlines_at_end_ = 0;
      return;
    }
    if (is_ascii_space(c)) {
      pending_space_ = true;
      return;
    }
    if (pending_space_ && !at_line_start_) out_.push_back(' ');
    pending_space_ = false;
    out_.push_back(static_cast<char>(c));
    at_line_start_ = false;
    newlines_at_end_ = 0;
  }

  // Consume html_[i_]. A CR LF pair is one newline.
  void consume_text_char() {
    const unsigned char c = static_cast<unsigned char>(html_[i_]);
    if (c == '\r') {
      ++i_;
      if (i_ < n_ && html_[i_] == '\n') ++i_;
      emit_byte('\n');
      return;
    }
    if (c == '&') {
      emit_decoded(parse_entity());
      return;
    }
    emit_byte(c);
    ++i_;
  }

  void emit_decoded(uint32_t cp) {
    if (cp == 0) return;
    if (pre_depth_ == 0 && cp < 0x80 && is_ascii_space(static_cast<unsigned char>(cp))) {
      pending_space_ = true;
      return;
    }
    if (pre_depth_ == 0 && pending_space_ && !at_line_start_) out_.push_back(' ');
    pending_space_ = false;
    const size_t before = out_.size();
    append_utf8(out_, cp);
    if (out_.size() > before) {
      at_line_start_ = false;
      newlines_at_end_ = 0;
    }
  }

  void emit_newline_raw() {
    pending_space_ = false;
    out_.push_back('\n');
    at_line_start_ = true;
    if (newlines_at_end_ < 2) ++newlines_at_end_;
  }

  void ensure_newlines(int n) {
    pending_space_ = false;
    if (out_.empty()) return;
    while (newlines_at_end_ < n) {
      out_.push_back('\n');
      ++newlines_at_end_;
    }
    at_line_start_ = true;
  }

  void emit_text(std::string_view text) {
    for (size_t k = 0; k < text.size();) {
      const unsigned char c = static_cast<unsigned char>(text[k]);
      if (c < 0x80) {
        emit_byte(c);
        ++k;
      } else {
        if (pre_depth_ == 0 && pending_space_ && !at_line_start_) out_.push_back(' ');
        pending_space_ = false;
        out_.push_back(static_cast<char>(c));
        ++k;
        at_line_start_ = false;
        newlines_at_end_ = 0;
      }
    }
  }

  void skip_until(const std::string& name, bool emit) {
    while (i_ < n_) {
      if (html_[i_] == '<') {
        const size_t save = i_;
        ++i_;
        if (i_ < n_ && html_[i_] == '/') {
          ++i_;
          const std::string close = read_name();
          skip_ws();
          if (close == name && i_ < n_ && html_[i_] == '>') {
            ++i_;
            return;
          }
        }
        i_ = save + 1;
        if (emit) emit_byte(static_cast<unsigned char>(html_[save]));
        continue;
      }
      if (!emit) {
        ++i_;
        continue;
      }
      consume_text_char();
    }
  }

  void parse_markup() {
    ++i_;
    if (i_ >= n_) {
      emit_byte('<');
      return;
    }
    if (html_[i_] == '!') {
      ++i_;
      if (i_ + 1 < n_ && html_[i_] == '-' && html_[i_ + 1] == '-') {
        i_ += 2;
        while (i_ + 2 < n_ &&
               !(html_[i_] == '-' && html_[i_ + 1] == '-' && html_[i_ + 2] == '>')) {
          ++i_;
        }
        if (i_ + 2 < n_) i_ += 3;
        else i_ = n_;
        return;
      }
      if (i_ + 6 < n_ && html_.substr(i_, 7) == "[CDATA[") {
        i_ += 7;
        while (i_ + 2 < n_ &&
               !(html_[i_] == ']' && html_[i_ + 1] == ']' && html_[i_ + 2] == '>')) {
          consume_text_char();
        }
        if (i_ + 2 < n_) i_ += 3;
        else i_ = n_;
        return;
      }
      while (i_ < n_ && html_[i_] != '>') ++i_;
      if (i_ < n_) ++i_;
      return;
    }
    if (html_[i_] == '?') {
      ++i_;
      while (i_ < n_) {
        if (html_[i_] == '>' && i_ > 0 && html_[i_ - 1] == '?') {
          ++i_;
          return;
        }
        if (html_[i_] == '>') {
          ++i_;
          return;
        }
        ++i_;
      }
      return;
    }

    const bool closing = html_[i_] == '/';
    if (closing) ++i_;
    const std::string name = read_name();
    std::string alt;
    while (i_ < n_) {
      skip_ws();
      if (i_ >= n_) break;
      if (html_[i_] == '>') {
        ++i_;
        break;
      }
      if (html_[i_] == '/') {
        ++i_;
        skip_ws();
        if (i_ < n_ && html_[i_] == '>') ++i_;
        break;
      }
      const std::string attr = read_name();
      if (attr.empty()) {
        // UTF-8 or other junk in a tag is not a name. Skip one byte so the
        // scan cannot sit still on that character.
        if (i_ < n_) ++i_;
        continue;
      }
      skip_ws();
      std::string value;
      if (i_ < n_ && html_[i_] == '=') {
        ++i_;
        skip_ws();
        value = read_attr_value();
      }
      if (attr == "alt") alt = std::move(value);
    }

    if (name.empty()) return;

    if (!closing && in_sorted(std::begin(kSkip), std::end(kSkip), name)) {
      skip_until(name, false);
      return;
    }
    if (!closing && in_sorted(std::begin(kRcdata), std::end(kRcdata), name)) {
      skip_until(name, true);
      ensure_newlines(1);
      return;
    }
    if (name == "br") {
      ensure_newlines(1);
      return;
    }
    if (name == "img") {
      emit_text(alt);
      return;
    }
    if (name == "pre") {
      if (!closing) {
        ensure_newlines(2);
        ++pre_depth_;
      } else if (pre_depth_ > 0) {
        --pre_depth_;
        ensure_newlines(1);
      }
      return;
    }
    if (in_sorted(std::begin(kCell), std::end(kCell), name)) {
      if (!at_line_start_) pending_space_ = true;
      return;
    }
    if (in_sorted(std::begin(kTight), std::end(kTight), name)) {
      ensure_newlines(1);
      return;
    }
    if (in_sorted(std::begin(kBlock), std::end(kBlock), name)) {
      // Open starts a paragraph. Close only ends the line, so a lone
      // <p>hello</p> stays one trailing newline.
      ensure_newlines(closing ? 1 : 2);
      return;
    }
  }

  std::string_view html_;
  size_t n_ = 0;
  size_t i_ = 0;
  std::string out_;
  int pre_depth_ = 0;
  bool pending_space_ = false;
  bool at_line_start_ = true;
  int newlines_at_end_ = 0;
};

}  // namespace

std::string extract(std::string_view html) {
  return Extractor(html).run();
}

}  // namespace html2txt
