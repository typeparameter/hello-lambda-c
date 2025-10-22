#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "lambda.h"

static size_t write_res_cb(char *data, size_t size, size_t nmemb, struct buffer *res);
static size_t drop_res_cb(char *data, size_t size, size_t nmemb, struct buffer *res);

int main() {
	CURL *curl;
	int res = curl_global_init(CURL_GLOBAL_NOTHING);
	if (res != EXIT_SUCCESS) return res;

	const char *api_host = getenv("AWS_LAMBDA_RUNTIME_API");

	char url[512];
	struct curl_header *header;

	curl = curl_easy_init();
	while (true) {
		struct buffer event = { .data = NULL, .len = 0 };
		struct buffer response = { .data = NULL, .len = 0 };
		curl_easy_reset(curl);

		snprintf(url, sizeof(url), "http://%s/2018-06-01/runtime/invocation/next", api_host);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_res_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &event);
		curl_easy_perform(curl);

		curl_easy_header(curl, "Lambda-Runtime-Aws-Request-Id", 0, CURLH_HEADER, -1, &header);
		char *request_id = strdup(header->value);

		curl_easy_header(curl, "Lambda-Runtime-Trace-Id", 0, CURLH_HEADER, -1, &header);
		char *trace_id = strdup(header->value);
		setenv("_X_AMZN_TRACE_ID", trace_id, 1);

		curl_easy_reset(curl);

		struct lambda_context context = {
			.aws_request_id = request_id
		};

		res = handler(&event, &context, &response);
		if (res != EXIT_SUCCESS) {
			snprintf(url, sizeof(url), "http://%s/2018-06-01/runtime/invocation/%s/error", api_host, request_id);
			curl_easy_setopt(curl, CURLOPT_POST, 1);
		} else {
			snprintf(url, sizeof(url), "http://%s/2018-06-01/runtime/invocation/%s/response", api_host, request_id);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, response.data);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, response.len);
		}

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, drop_res_cb);
		curl_easy_perform(curl);

		free(request_id);
		free(trace_id);
		free(event.data);
		free(response.data);
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return EXIT_SUCCESS;
}

static size_t write_res_cb(char *data, size_t size, size_t nmemb, struct buffer *res) {
	size_t realsize = size * nmemb;

	char *tmp = realloc(res->data, res->len + realsize);
	if (!tmp) return 0;
	res->data = tmp;

	memcpy(res->data + res->len, data, realsize);
	res->len += realsize;

	return realsize;
}

static size_t drop_res_cb(char *data, size_t size, size_t nmemb, struct buffer *res) {
	return size * nmemb;
}
