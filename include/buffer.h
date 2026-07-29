#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

struct buffer {
	char *data;
	size_t len;
};

#endif
