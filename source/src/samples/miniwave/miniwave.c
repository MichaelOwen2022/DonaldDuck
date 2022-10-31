/*
 * Copyright (c) 2022 - 2023, tangchunhui@coros.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/************************************************************************************************************************/

#include "miniwave.h"

struct libops
{
	void (*version)(char *name, int *major, int *minor, char *date);
};

static struct libops libops[] =
{
	{miniwave_version},
};

/************************************************************************************************************************/

#include <getopt.h>

#define USAGE_STRING \
"\
usage: " NAME_STRING "[options]\n\
   MiniWave音频解码&保存\n\
        --help        display help and exit\n\
        --version     display version and exit\n\
"

static void display_help(void)
{
	printf(USAGE_STRING);
	exit(0);
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))
#endif

static void display_version(void)
{
	int i;

	printf(NAME_STRING " version: %d.%d [%s]\n", VERSION_MAJOR, VERSION_MINOR, BUILD_DATE);

	for (i = 0; i < ARRAY_SIZE(libops); i++)
	{
		if (libops[i].version)
		{
			char name[64]="";
			int  major, minor;
			char date[64]="";

			libops[i].version(name, &major, &minor, date);
			printf("	%s version: %d.%d [%s]\n", name, major, minor, date);
		}
	}

	exit(0);
}

static void process_options(int argc, char **argv)
{
	for (;;)
	{
		int option_index = 0;
		static const char * short_options = "";
		static const struct option long_options[] =
		{
			{"help", 	no_argument, 0, 0},
			{"version", no_argument, 0, 0},
			{0, 0, 0, 0},
		};

		int opt = getopt_long(argc, argv, short_options,
							long_options, &option_index);
		if (opt == EOF) break;

		switch(opt)
		{
		case 0:
			switch(option_index)
			{
			case 0:
				display_help();
				break;
			case 1:
				display_version();
				break;
			}
			break;
		case '?':
			printf("Unknown option %c\n", optopt);
			break;
		}
	}
}

/************************************************************************************************************************/

int main(int argc, char **argv)
{
	WAV iwave = NULL;
	WAV owave = NULL;
	void *data = NULL;
	int   size = 0;
	WAV_ATTR attr;
	int retval = 0;

	process_options(argc, argv);

	iwave = miniwave_open(argv[1], WAVE_O_RDONLY, &attr);
	if (iwave == NULL)
	{
		retval = -EPERM;
		goto ERR_EXIT;
	}

	owave = miniwave_open(argv[2], WAVE_O_WRONLY, &attr);
	if (owave == NULL)
	{
		retval = -EPERM;
		goto ERR_EXIT;
	}

	size = (1105 * (attr.sampbits / 8) * attr.channels);

	data = malloc(size);
	if (data == NULL)
	{
		retval = -ENOMEM;
		goto ERR_EXIT;
	}

	while (1)
	{
		retval = miniwave_read(iwave, data, size);
		if (retval <= 0)
			break;

		retval = miniwave_write(owave, data, retval);
		if (retval <= 0)
			break;
	}

ERR_EXIT:
	if (data)
		free(data);

	if (iwave)
		miniwave_close(iwave);

	if (owave)
		miniwave_close(owave);

	return retval;
}
