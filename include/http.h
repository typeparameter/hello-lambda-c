#ifndef HTTP_H
#define HTTP_H

#include "buffer.h"

struct http_response {
	int status_code;
	struct buffer headers;
	struct buffer body;
};

int http_get(const char *host, const char *path, struct http_response *res);
int http_post(const char *host, const char *path, const struct buffer *body, struct http_response *res);
int http_response_header(const struct http_response *res, const char *name, char **value);
void http_response_destroy(struct http_response *res);

#endif
