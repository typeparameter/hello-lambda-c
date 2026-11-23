#include <stdio.h>
#include <stdlib.h>

#include "http.h"
#include "lambda.h"

void lambda_run(lambda_handler handler)
{
    const char *api_host = getenv("AWS_LAMBDA_RUNTIME_API");

    for (;;) {
        int ret;
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
        if (ret < 0) {
            fprintf(stderr, "lambda: could not fetch next invocation\n");
            exit(EXIT_FAILURE);
        }

        lambda__http_response_header(&http_res, "Lambda-Runtime-Aws-Request-Id",
                                     &request_id);
        context.aws_request_id = request_id;
        event = http_res.body;
        http_res.body.data = NULL;
        http_res.body.len = 0;
        lambda__http_response_destroy(&http_res);

        ret = handler(&event, &context, &response);
        if (ret != 0) {
            sprintf(path, "/2018-06-01/runtime/invocation/%s/error",
                    request_id);
            lambda__http_post(api_host, path, NULL, &http_res);
            lambda__http_response_destroy(&http_res);
        } else {
            sprintf(path, "/2018-06-01/runtime/invocation/%s/response",
                    request_id);
            lambda__http_post(api_host, path, &response, &http_res);
            lambda__http_response_destroy(&http_res);
        }

        free(request_id);
        free(event.data);
        free(response.data);
    }
}
