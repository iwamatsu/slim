/****************************************************************************
    jpeg.c - read and write jpeg images using libjpeg routines
    Copyright (C) 2002 Hari Nair <hari@alumni.caltech.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>
#include "const.h"

int
read_jpeg(const char *filename, int *width, int *height, unsigned char **rgb)
{
    int ret = 0;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char *ptr = NULL;
    unsigned int i, ipos;

    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) {
        fprintf(stderr, "Can not fopen file: %s\n",filename);
        return ret;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    /* Prevent against integer overflow */
    if(cinfo.output_width >= MAX_DIMENSION || cinfo.output_height >= MAX_DIMENSION) {
        fprintf(stderr, "Unreasonable dimension found in file: %s\n",filename);
        goto close_file;
    }

    *width = cinfo.output_width;
    *height = cinfo.output_height;

    rgb[0] = malloc(3 * cinfo.output_width * cinfo.output_height);
    if (rgb[0] == NULL) {
        fprintf(stderr, "Can't allocate memory for JPEG file.\n");
        goto close_file;
    }

    if (cinfo.output_components == 3) {
        ptr = rgb[0];
        while (cinfo.output_scanline < cinfo.output_height) {
            jpeg_read_scanlines(&cinfo, &ptr, 1);
            ptr += 3 * cinfo.output_width;
        }
    } else if (cinfo.output_components == 1) {
        ptr = malloc(cinfo.output_width);
        if (ptr == NULL) {
            fprintf(stderr, "Can't allocate memory for JPEG file.\n");
            goto rgb_free;
        }

        ipos = 0;
        while (cinfo.output_scanline < cinfo.output_height) {
            jpeg_read_scanlines(&cinfo, &ptr, 1);

            for (i = 0; i < cinfo.output_width; i++) {
                memset(rgb[0] + ipos, ptr[i], 3);
                ipos += 3;
            }
        }

        free(ptr);
    }

    jpeg_finish_decompress(&cinfo);

    ret = 1;
    goto close_file;

rgb_free:
    free(rgb[0]);

close_file:
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return(ret);
}
