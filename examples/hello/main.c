#include <stdlib.h>
#include <string.h>

#include <lambda.h>

static int hello(const struct lambda_buffer *event,
                 const struct lambda_context *context,
                 struct lambda_buffer *response)
{
    const char *message = "Hello, Lambda!";

    response->len = strlen(message);
    response->data = malloc(response->len);
    if (response->data == NULL)
        return -1;

    memcpy(response->data, message, response->len);
    return 0;
}

int main(void)
{
    lambda_run(hello);
    return EXIT_SUCCESS;
}
