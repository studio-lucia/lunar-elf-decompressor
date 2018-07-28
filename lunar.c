/*----------------------------------------------------------------------------*/
/*-- lunar.c                                                                --*/
/*-- ELF decompressor for 'Lunar Silver Star Story - PSX'                   --*/
/*-- Copyright (C) 2010 CUE                                                 --*/
/*--                                                                        --*/
/*-- This program is free software: you can redistribute it and/or modify   --*/
/*-- it under the terms of the GNU General Public License as published by   --*/
/*-- the Free Software Foundation, either version 3 of the License, or      --*/
/*-- (at your option) any later version.                                    --*/
/*--                                                                        --*/
/*-- This program is distributed in the hope that it will be useful,        --*/
/*-- but WITHOUT ANY WARRANTY; without even the implied warranty of         --*/
/*-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           --*/
/*-- GNU General Public License for more details.                           --*/
/*--                                                                        --*/
/*-- You should have received a copy of the GNU General Public License      --*/
/*-- along with this program. If not, see <http://www.gnu.org/licenses/>.   --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/
#define EXIT(text) { printf(text); exit(EXIT_FAILURE); }

/*----------------------------------------------------------------------------*/
void  Title(void);
void  Usage(void);
char *Load(char *filename);
void  Save(char *filename, char *buffer, int length);
char *Memory(int length, int size);
void  Decode(char *elfname, char *newname);

/*----------------------------------------------------------------------------*/
int main(int argc, char **argv) {
  Title();
  if (argc != 3) Usage();

  Decode(argv[1], argv[2]);

  printf("Done\n");

  exit(EXIT_SUCCESS);
}

/*----------------------------------------------------------------------------*/
void Title(void) {
  printf(
    "\n"
    "LUNAR - Copyright (C) 2010 CUE\n"
    "ELF decompressor for 'Lunar Silver Star Story - PSX'\n"
    "\n"
  );
}

/*----------------------------------------------------------------------------*/
void Usage(void) {
  EXIT(
    "Usage: LUNAR elf_name new_name\n"
    "\n"
    "- 'elf_name' is the USA ELF filename\n"
    "- 'new_name' is the new decoded ELF filename\n"
  );
}

/*----------------------------------------------------------------------------*/
char *Load(char *filename) {
  FILE *fp;
  int   fs;
  char *fb;

  if ((fp = fopen(filename, "rb")) == NULL) EXIT("File open error\n");
  fs = filelength(fileno(fp));
  if (fs < 0x00001000) EXIT("File too small\n");
  if (fs > 0x00100000) EXIT("File too big\n");
  fb = Memory(fs, sizeof(char));
  if (fread(fb, 1, fs, fp) != fs) EXIT("File read error\n");
  if (fclose(fp) == EOF) EXIT("File close error\n");

  return(fb);
}

/*----------------------------------------------------------------------------*/
void Save(char *filename, char *buffer, int length) {
  FILE *fp;

  if ((fp = fopen(filename, "wb")) == NULL) EXIT("File create error\n");
  if (fwrite(buffer, 1, length, fp) != length) EXIT("File write error\n");
  if (fclose(fp) == EOF) EXIT("File close error\n");
}

/*----------------------------------------------------------------------------*/
char *Memory(int length, int size) {
  char *fb;

  fb = (char *)calloc(length * size, size);
  if (fb == NULL) EXIT("Memory error\n");

  return(fb);
}

/*----------------------------------------------------------------------------*/
void Decode(char *elfname, char *newname) {
  unsigned char *in, *out, *enc, *dec;
  unsigned int flags, len, pos;

  in  = (unsigned char *)Load(elfname);
  if (*(unsigned int *)(in + 0x1000) != 0x2D5350FF)
    EXIT("File is not a valid compressed ELF");

  out = (unsigned char *)Memory(0x1000000, sizeof(char));

  enc = in + 0x1000;
  dec = out;

  for (flags = 0; ; ) {
    if ((flags >>= 1) < 0x100) flags = 0xFF00 | *enc++;
    if (flags & 0x1) {
      *dec++ = *enc++;
    } else {
      if ((flags >>= 1) < 0x100) flags = 0xFF00 | *enc++;
      if (flags & 0x1) {
        pos = *enc++;
        len = pos >> 6;
        pos |= -0x40;
      } else {
        pos = *enc++ << 8;
        pos |= *enc++;
        len = pos & 0xF000 ? pos >> 12 : *enc++;
        if (!len) break;
        pos |= -0x1000;
      }
      len += 2;
      while (len--) *dec++ = *(dec + pos);
    }
  }

  Save(newname, out, dec - out);

  free(out);
  free(in);
}

/*----------------------------------------------------------------------------*/
/*--  EOF                                           Copyright (C) 2010 CUE  --*/
/*----------------------------------------------------------------------------*/
