#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "http.h"
#include "lambda.h"

static void warn(const char *format, ...);
static void die(const char *format, ...);

void lambda_run(lambda_handler handler)
{
    const char *api_host = getenv("AWS_LAMBDA_RUNTIME_API");

    if (api_host == NULL)
        die("AWS_LAMBDA_RUNTIME_API is not set");

    for (;;) {
        int ret, error;
        struct lambda_buffer event;
        struct lambda_buffer response;
        struct lambda_context context;
        struct http_response http_res;
        char *request_id;
        char path[512];

        event.data = NULL;
        event.len = 0;
        response.data = NULL;
        response.len = 0;

        ret = lambda__http_get(api_host, "/2018-06-01/runtime/invocation/next",
                               &http_res);
        if (ret < 0)
            die("could not fetch next invocation");

        if (http_res.status_code != 200)
            die("next invocation returned status %d", http_res.status_code);

        lambda__http_response_header(&http_res, "Lambda-Runtime-Aws-Request-Id",
                                     &request_id);
        if (request_id == NULL)
            die("next invocation has no request ID");

        context.aws_request_id = request_id;
        event = http_res.body;
        http_res.body.data = NULL;
        http_res.body.len = 0;
        lambda__http_response_destroy(&http_res);

        error = handler(&event, &context, &response) != 0;
        sprintf(path, "/2018-06-01/runtime/invocation/%s/%s", request_id,
                error ? "error" : "response");
        ret = lambda__http_post(api_host, path, error ? NULL : &response,
                                &http_res);
        if (ret < 0)
            die("could not send invocation %s", error ? "error" : "response");
        else if (http_res.status_code == 500)
            die("container error reported by runtime API");
        else if (http_res.status_code == 413)
            warn("response payload too large");
        else if (http_res.status_code != 202)
            warn("runtime API rejected request (status %d)",
                 http_res.status_code);
        lambda__http_response_destroy(&http_res);

        free(request_id);
        free(event.data);
        free(response.data);
    }
}

static void vwarn(const char *format, va_list args)
{
    fprintf(stderr, "lambda: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

static void warn(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vwarn(format, args);
    va_end(args);
}

static void die(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vwarn(format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
