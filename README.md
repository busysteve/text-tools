# text-tools

C++17 Unix tools: `html2txt`, `breakout`, `utf8rm`, `binrm`, and `splitter`.

```bash
cmake -B build -S .
cmake --build build
```

Binaries land in `build/`. Run the tests with `ctest --test-dir build`.

## html2txt

Writes the visible text of an HTML document.

```bash
html2txt                 # stdin -> stdout
html2txt page.html       # file -> stdout
html2txt page.html out.txt
```

A dash for `IN` or `OUT` is stdin or stdout.

Tags, comments, and the contents of `script`, `style`, `noscript`, `template`, `svg`, and `math` are dropped. Named and numeric entities are decoded. UTF-8 text is copied through. Block tags become line breaks. `<pre>` keeps its whitespace. `<img alt>` is kept as text.

## breakout

Writes one sentence per line. Newlines always split. With no options every mark is used: period, comma, colon, semicolon, and two spaces. If any option is given, only those marks split. Punctuation is kept.

```bash
breakout                          # stdin -> stdout, all marks
breakout in.txt                   # file -> stdout
breakout in.txt out.txt           # file -> file
breakout --comma --colon in.txt   # only comma and colon
```

A dash for `IN` or `OUT` is stdin or stdout.

```text
  --period         break after a period
  --comma          break after a comma
  --colon          break after a colon
  --semicolon      break after a semicolon
  --double-space   break on two spaces
```

`--period` splits on every period, including `Mr.` and `3.14`. Surrounding whitespace and blank lines are dropped. A run of the same mark stays with the sentence (`Wait...`, `a,,`).

## utf8rm

Writes ASCII. It reads UTF-8 and drops each non-ASCII character. Ill-formed bytes are dropped.

```bash
utf8rm                      # stdin -> stdout
utf8rm dirty.txt            # file -> stdout
utf8rm dirty.txt out.txt    # file -> file
```

A dash for `IN` or `OUT` is stdin or stdout.

## binrm

Writes ASCII text. It drops binary bytes. Carriage return, line feed, whitespace, and printable ASCII are kept.

```bash
binrm                       # stdin -> stdout
binrm dirty.bin             # file -> stdout
binrm dirty.bin out.txt     # file -> file
```

A dash for `IN` or `OUT` is stdin or stdout.

## splitter

Writes a sequence of files, each holding at most `LINES` lines. The first file is `0000000000.dat`, then `0000000001.dat`, and so on. The last file may be shorter. A line ends at a newline. A final partial line is kept. Bytes are copied unchanged.

```bash
splitter 1000                    # stdin -> ./0000000000.dat, ...
splitter 1000 in.txt             # file -> current directory
splitter 1000 in.txt outdir      # file -> outdir
splitter 1000 - outdir           # stdin -> outdir
```

A dash for `IN` is stdin. `DIR` defaults to the current directory and is created when missing. `LINES` is a positive integer. An empty input writes no files. A file already named for a chunk is replaced. Other files in `DIR` are left in place.

License is CC0 1.0.
