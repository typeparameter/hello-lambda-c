#ifndef HTTP_H
#define HTTP_H

#include "lambda.h"

struct http_response {
    int status_code;
    struct lambda_buffer headers;
    struct lambda_buffer body;
};

/* On failure, res has nothing to destroy. */
int lambda__http_get(const char *host, const char *path,
                     struct http_response *res);
int lambda__http_post(const char *host, const char *path,
                      const struct lambda_buffer *body,
                      struct http_response *res);
int lambda__http_response_header(const struct http_response *res,
                                 const char *name, char **value);
void lambda__http_response_destroy(struct http_response *res);

#endif
