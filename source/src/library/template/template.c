/*
 * Copyright (c) 2022 - 2023, tangchunhui@coros.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

/************************************************************************************************************************/

void template_version(char *name, int *major, int *minor, char *date)
{
	if (name)
		strcpy(name, NAME_STRING);
	if (major)
		*major = VERSION_MAJOR;
	if (minor)
		*minor = VERSION_MINOR;
	if (date)
		strcpy(date, BUILD_DATE);
}
