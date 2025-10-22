#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lambda.h"

int handler(const struct buffer *event, const struct lambda_context *context, struct buffer *response) {
	const char *message = "Hello, Lambda!";
	response->len = strlen(message);
	response->data = malloc(response->len);
	if (!response->data) return EXIT_FAILURE;

	memcpy(response->data, message, response->len);
	return EXIT_SUCCESS;
}
