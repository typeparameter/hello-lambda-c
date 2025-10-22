#include <stddef.h>

struct buffer {
	char* data;
	size_t len;
};

struct lambda_context {
	char *aws_request_id;
};

extern int handler(const struct buffer *event, const struct lambda_context *context, struct buffer *response);
