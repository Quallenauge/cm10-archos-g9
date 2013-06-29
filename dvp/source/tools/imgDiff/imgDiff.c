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
 * \brief Tool to perform a diff compare to golden data
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <features.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fnmatch.h>


static char prefix[2];

static int
one (const struct dirent *unused)
{
  if(strncmp(prefix, unused->d_name, 2)==0)
    return 1;
  else
    return 0;
}

int
img_diff(FILE *fref,
         FILE *fout,
         int width,
         int height,
         int frames,
         int hSt,
         int vSt,
         int hBuff,
         int vBuff,
         int bpp,
         int planes,
         int divider)
{
    int i,j,k,f, errors;
    unsigned char *refBuffer;
    unsigned char *outBuffer;
    int shift, step, div;
    size_t r = 0;
    errors = 0;

    refBuffer = (unsigned char *)malloc(sizeof(char)*width*bpp);
    outBuffer = (unsigned char *)malloc(sizeof(char)*width*bpp);

    step = (width-hBuff)*bpp;

    for(f=0;f<frames;f++)
    {
        for(k=0;k<planes;k++)
        {
            if(k==0)    // First plane is never divided, only subsequent planes
                div = 1;
            else
                div = divider;

            for(j=0;j<vSt;j++)
            {
                r+=fread(refBuffer, 1, width*bpp/div, fref);
                r+=fread(outBuffer, 1, width*bpp/div, fout);
            }
            for(;j<(height/div-vBuff);j++)
            {
                r+=fread(refBuffer, 1, width*bpp/div, fref);
                r+=fread(outBuffer, 1, width*bpp/div, fout);
                for(i=0;i<(step/div);i++)
                {
                    if ((f>0) && (refBuffer[i] != outBuffer[i]))
//                  if ((f>0) && (refBuffer[i] != outBuffer[i]) && errors < 1000)
                    {
//                      printf("\tFAIL: x=%d, y=%d, pl=%d, ref=%d, out=%d\n", i, j, k, refBuffer[i], outBuffer[i]);
                        errors++;
                    }
                }
            }
            for(;j<(height/div);j++)
            {
                r+=fread(refBuffer, 1, width*bpp/div, fref);
                r+=fread(outBuffer, 1, width*bpp/div, fout);
            }
        }
    }

    free(refBuffer);
    free(outBuffer);
    return errors;
}

void
parse_format(char *format, int *bpp, int *planes)
{
    *bpp = 1;
    *planes = 1;

    if((strcmp(format, "P444") == 0) ||
       (strcmp(format, "IYUV") == 0) )
        *planes = 3;
    else if(strcmp(format, "UYVY") == 0)
        *bpp = 2;
    else if(strcmp(format, "I32") == 0)
        *bpp = 4;
    else if(strcmp(format, "16b") == 0)
        *bpp = 2;
    else if(strcmp(format, "32b") == 0)
        *bpp = 4;

    return;
}

int
main(int argc, char **argv)
{
  FILE *fout;
  FILE *fref;
  FILE *fconfig;

  char refDir[255];
  char outDir[255];
  char refFile[255];
  char outFile[255];
  char fileName[255];

  int frames, debug;
  struct dirent **eps;
  int n;
  int bpp, planes, hSt, vSt, hBuff, vBuff;
  int totalErrors = 0;
  int totalFiles = 0;

  if (argc != 6) {
    printf( "dvp_compare <reference dir> <out dir> <config file> <num frames> <debug 0/1>\r\n");
    return -1;
  }

  // Parse command line arguments
  strcpy(refDir, argv[1]);
  strcpy(outDir, argv[2]);
  fconfig = fopen(argv[3], "r");
  frames = atoi(argv[4]);
  debug = atoi(argv[5]);

  // Parse each line of the config file
  while(fscanf(fconfig, "%s %d %d %d %d", prefix, &hSt, &vSt, &hBuff, &vBuff) != EOF)
  {
    if(debug) printf("%s %d %d %d %d\n", prefix, hSt, vSt, hBuff, vBuff);
    n = scandir (refDir, &eps, one, alphasort);
    if (n >= 0)
    {
      int cnt;
      for (cnt = 0; cnt < n; ++cnt) {
        char *number, *name, *resolution, *fr;
        char *w, *h, *format;
        char *sp[20];
        int width, height, error, divider, i;
        strcpy(refFile, refDir);
        strcat(refFile, eps[cnt]->d_name);
        strcpy(outFile, outDir);
        strcat(outFile, eps[cnt]->d_name);
        strcpy(fileName, eps[cnt]->d_name);
        if(debug) puts (eps[cnt]->d_name);
        if(debug) puts (refFile);
        if(debug) puts (outFile);
        fref = fopen(refFile, "rb");
        if(fref == NULL) {
          printf ("Couldn't open the reference file %s\n", refFile);
          break;
        }
        fout = fopen(outFile, "rb");
        if(fout == NULL) {
          printf ("Couldn't open the output file %s.\n", outFile);
          fclose(fref);
          break;
        }
        if(debug) printf("Files are open\n");

        // Parse file name
        number = strtok(eps[cnt]->d_name, "_");

#if 1
        for( i=0; (sp[i] = strtok(NULL, "_")) != NULL; i++)
            if(debug) printf("sp[%d]: %s\n", i, sp[i]);
        format = strtok(sp[i-1], ".");
        for( i=0; strchr(sp[i], (int)'x') == NULL; i++);
        w = strtok(sp[i], "x");
        h = strtok(NULL, "x");
        if(debug) printf("number: %s, format: %s, w: %s, h: %s\n",
                          number, format, w, h);
#else
        name = strtok(NULL, "_");
        resolution = strtok(NULL, "_");
        fr = strtok(NULL, "_");
        format = strtok(NULL, ".");
        w = strtok(resolution, "x");
        h = strtok(NULL, "x");
        if(debug) printf("number: %s, name: %s, res: %s, fr: %s, format: %s, w: %s, h: %s\n",
                          number, name, resolution, fr, format, w, h);
#endif

        divider = 1;
        if(strcmp(format, "IYUV") == 0)
            divider = 2;

        width = atoi(w);
        height = atoi(h);
        if(debug) printf("%d %d\n", width, height);

        // Extract bpp and planes from file name format
        parse_format(format, &bpp, &planes);
        if(debug) printf("format: %s, bpp: %d, planes:%d\n", format, bpp, planes);

        // Compare files
        error = img_diff(fref, fout, width, height, frames, hSt, vSt, hBuff, vBuff, bpp, planes, divider);

        if (error)
        {
            puts(fileName);
            printf("\tFAIL: bytes mismatch = %d\n", error);
            totalErrors++;
        }
        totalFiles++;

        fclose(fref);
        fclose(fout);
      }
    }
    else
      perror ("Couldn't open the directory");
  }

  printf("\tTOTAL:  %d\n\tPASSED: %d\n\tFAILED: %d\n", totalFiles, totalFiles-totalErrors, totalErrors);

  fclose(fconfig);

  return 0;
}
