#ifndef LAMBDA_H
#define LAMBDA_H

#include "buffer.h"

struct lambda_context {
    char *aws_request_id;
};

int handler(const struct buffer *event, const struct lambda_context *context,
            struct buffer *response);

#endif
