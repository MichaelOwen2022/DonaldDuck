/*
 * Copyright (c) 2022 - 2023, tangchunhui@coros.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

typedef void* WAV;

typedef struct
{
    unsigned int samprate;
    unsigned int sampbits;
    unsigned int channels;
    unsigned int dataoffs;
    unsigned int datasize;
} WAV_ATTR;

#define WAVE_O_RDONLY   (1 << 0)
#define WAVE_O_WRONLY   (1 << 1)

void miniwave_version(char *name, int *major, int *minor, char *date);

WAV miniwave_open(const char *name, int flags, WAV_ATTR *attr);

WAV miniwave_open_with_file(int file, int flags, WAV_ATTR *attr);

int miniwave_attr(WAV wav, WAV_ATTR *attr);

int miniwave_read(WAV wav, void *buf, int len);

int miniwave_write(WAV wav, void *buf, int len);

int miniwave_close(WAV wav);

#endif
