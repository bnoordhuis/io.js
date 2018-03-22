#include "source/common/unicode/uvernum.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

inline void die(const char* message) {
  fprintf(stderr, "%s\n", message);
  exit(1);
}

inline void bin2c(const char* infile, FILE* out) {
  static unsigned char buf[1 << 20];  // static to avoid stack overflow.
  FILE* in = fopen(infile, "rb");
  if (in == nullptr) die("cannot open input file");
  fprintf(out, "unsigned icudt%d_dat[] = {", U_ICU_VERSION_MAJOR_NUM);
  while (size_t n = fread(buf, 1, sizeof(buf), in)) {
    if (n % 4 != 0) die("file size not a multiple of 4");
    for (size_t i = 0; i < n; i += 4) {
      unsigned u = buf[i];  // Data is little endian.
      u += buf[i+1] * 0x100u;
      u += buf[i+2] * 0x10000u;
      u += buf[i+3] * 0x1000000u;
      if (i % 16 == 0) fprintf(out, "\n");
      fprintf(out, "%u", u);
      if (u >  0x7FFFFFFFu) fprintf(out, "u");
      fprintf(out, ",");
    }
  }
  fprintf(out, "};\n");
  fclose(in);
}

inline void incbin(const char* infile, FILE* out, const char* os) {
  const char* prefix = !strcmp(os, "mac") ? "_" : "";
  if (!strcmp(os, "mac")) fprintf(out, ".const_data\n");
  else fprintf(out, ".section .rodata\n");
  fprintf(out, ".globl %sicudt%d_dat\n", prefix, U_ICU_VERSION_MAJOR_NUM);
  fprintf(out, ".balign 4096\n");
  fprintf(out, "%sicudt%d_dat:\n", prefix, U_ICU_VERSION_MAJOR_NUM);
  fprintf(out, ".incbin \"%s\"\n", infile);
}

int main(int argc, char** argv) {
  if (argc < 4) die("not enough arguments");
  const char* infile = argv[1];
  const char* outfile = argv[2];
  const char* action = argv[3];
  const char* os = argv[4];
  FILE* out = argc > 2 ? fopen(outfile, "w") : stdout;
  if (out == nullptr) die("cannot open output file");
  if (!strcmp(action, "incbin")) incbin(infile, out, os);
  else if (!strcmp(action, "bin2c")) bin2c(infile, out);
  else die("bad argument - must be bin2c or incbin");
  fclose(out);
}
