#ifndef LAMBDA_H
#define LAMBDA_H

#include <stddef.h>

struct lambda_buffer {
    char *data;
    size_t len;
};

struct lambda_context {
    const char *aws_request_id;
};

/*
 * Called once per invocation. Puts the response in memory from malloc(),
 * which the runtime frees after sending. Returns 0 on success, or non-zero
 * to report the invocation as failed.
 */
typedef int (*lambda_handler)(const struct lambda_buffer *event,
                              const struct lambda_context *context,
                              struct lambda_buffer *response);

/*
 * Fetches events from the Lambda runtime API and passes each one to
 * handler. Never returns; exits the process if it cannot fetch the next
 * invocation.
 */
void lambda_run(lambda_handler handler);

#endif
