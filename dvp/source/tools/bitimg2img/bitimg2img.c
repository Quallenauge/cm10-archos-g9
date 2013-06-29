/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file
 * \brief Tool to convert binary image file into byte image file (0x01 -> 0xFF)
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
  FILE *fp;
  FILE *fd;
  unsigned char inword[4];
  unsigned char outword[4];

  if (argc != 3) {
    printf( "bitimg2img <infile> <outfile>\r\n");
    return -1;
  }

  fp = fopen(argv[1], "rb");
  fd = fopen(argv[2], "wb");


while (fread(inword, sizeof(inword), 1, fp) > 0) {
    outword[0] = (inword[0]) ? 0xFF : 0;
    outword[1] = (inword[1]) ? 0xFF : 0;
    outword[2] = (inword[2]) ? 0xFF : 0;
    outword[3] = (inword[3]) ? 0xFF : 0;
    fwrite(outword ,sizeof(outword), 1, fd);
  }

  fclose(fp);
  fclose(fd);

  return 0;
}
