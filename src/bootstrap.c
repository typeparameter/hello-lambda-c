#define _POSIX_C_SOURCE 199309L

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "lambda.h"

int main(void) {
	int res;
	const char *api_host;
	char path[512];

	api_host = getenv("AWS_LAMBDA_RUNTIME_API");

	for (;;) {
		struct buffer event;
		struct buffer response;
		struct lambda_context context;
		struct http_response http_res;
		char *request_id;

		event.data = NULL;
		event.len = 0;
		response.data = NULL;
		response.len = 0;

		http_get(api_host, "/2018-06-01/runtime/invocation/next", &http_res);
		http_response_header(&http_res, "Lambda-Runtime-Aws-Request-Id", &request_id);
		context.aws_request_id = request_id;
		event = http_res.body;
		http_res.body.data = NULL;
		http_res.body.len = 0;
		http_response_destroy(&http_res);

		res = handler(&event, &context, &response);
		if (res != EXIT_SUCCESS) {
			sprintf(path, "/2018-06-01/runtime/invocation/%s/error", request_id);
			http_post(api_host, path, NULL, &http_res);
			http_response_destroy(&http_res);
		} else {
			sprintf(path, "/2018-06-01/runtime/invocation/%s/response", request_id);
			http_post(api_host, path, &response, &http_res);
			http_response_destroy(&http_res);
		}

		free(request_id);
		free(event.data);
		free(response.data);
	}

	return 0;
}
