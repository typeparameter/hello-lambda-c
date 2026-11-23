#define _POSIX_C_SOURCE 199309L

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http.h"

struct http_request {
    const char *method;
    const char *host;
    const char *path;
    const struct buffer *body;
};

static int send_request(const struct http_request *req,
                        struct http_response *res);
static int connect_tcp(const struct http_request *req);
static int write_request(FILE *fp, const struct http_request *req);
static int read_response(FILE *fp, struct http_response *res);
static int readline(FILE *fp, char *buffer, size_t size);

int http_get(const char *host, const char *path, struct http_response *res)
{
    struct http_request req;

    req.method = "GET";
    req.host = host;
    req.path = path;
    req.body = NULL;

    return send_request(&req, res);
}

int http_post(const char *host, const char *path, const struct buffer *body,
              struct http_response *res)
{
    struct http_request req;

    req.method = "POST";
    req.host = host;
    req.path = path;
    req.body = body;

    return send_request(&req, res);
}

int http_response_header(const struct http_response *res, const char *name,
                         char **value)
{
    const char *ptr, *body_end;
    const char *line_start, *line_end;
    const char *val_start, *val_end;
    size_t name_len, val_len;

    if (res->headers.len == 0)
        return -1;

    name_len = strlen(name);
    *value = NULL;

    line_start = res->headers.data;
    body_end = line_start + res->headers.len;

    while (line_start < body_end) {
        line_end = NULL;
        for (ptr = line_start; ptr + 1 < body_end; ptr++) {
            if (ptr[0] == '\r' && ptr[1] == '\n') {
                line_end = ptr;
                break;
            }
        }

        if (line_end == NULL)
            return -1;

        ptr = memchr(line_start, ':', line_end - line_start);
        if (ptr != NULL && ptr - line_start == name_len &&
            strncmp(line_start, name, name_len) == 0) {
            val_start = ptr + 1;
            val_end = line_end;

            while (val_start < line_end &&
                   (*val_start == ' ' || *val_start == '\t'))
                val_start++;

            while (val_end > val_start &&
                   (val_end[-1] == ' ' || val_end[-1] == '\t'))
                val_end--;

            val_len = val_end - val_start;
            *value = malloc(val_len + 1);
            if (*value == NULL)
                return -1;

            memcpy(*value, val_start, val_len);
            (*value)[val_len] = '\0';

            return 0;
        }

        line_start = line_end + 2;
    }

    return -1;
}

void http_response_destroy(struct http_response *res)
{
    free(res->headers.data);
    res->headers.data = NULL;
    res->headers.len = 0;

    free(res->body.data);
    res->body.data = NULL;
    res->body.len = 0;
}

static int send_request(const struct http_request *req,
                        struct http_response *res)
{
    int fd, ret;
    FILE *fp;

    fd = connect_tcp(req);
    if (fd < 0)
        return -1;

    fp = fdopen(fd, "r+");
    if (fp == NULL) {
        close(fd);
        return -1;
    }

    ret = write_request(fp, req);
    if (ret < 0) {
        fclose(fp);
        return -1;
    }

    ret = read_response(fp, res);
    if (ret < 0) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

static int connect_tcp(const struct http_request *req)
{
    int fd, ret;
    long value;
    size_t hostname_size;
    char *hostname, *sep, *end;
    unsigned short port;
    struct hostent *host;
    struct sockaddr_in addr;

    sep = strrchr(req->host, ':');
    if (sep == req->host) {
        return -1;
    } else if (sep == NULL) {
        port = 80;
        hostname_size = strlen(req->host);
    } else {
        value = strtol(sep + 1, &end, 10);
        if (end == sep + 1 || *end != '\0')
            return -1;
        if (value < 1 || value > 65535)
            return -1;
        port = (unsigned short)value;
        hostname_size = (size_t)(sep - req->host);
    }

    hostname = malloc(hostname_size + 1);
    if (hostname == NULL)
        return -1;
    memcpy(hostname, req->host, hostname_size);
    hostname[hostname_size] = '\0';

    host = gethostbyname(hostname);
    free(hostname);
    if (host == NULL)
        return -1;
    if (host->h_addrtype != AF_INET)
        return -1;
    if (host->h_addr_list == NULL || host->h_addr_list[0] == NULL)
        return -1;
    if (host->h_length > sizeof(addr.sin_addr))
        return -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, host->h_addr_list[0], host->h_length);

    ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static int write_request(FILE *fp, const struct http_request *req)
{
    int ret;
    size_t n;

    ret = fprintf(fp, "%s %s HTTP/1.0\r\n", req->method, req->path);
    if (ret < 0)
        return -1;

    ret = fprintf(fp, "Host: %s\r\n", req->host);
    if (ret < 0)
        return -1;

    if (req->body != NULL) {
        ret = fprintf(fp, "Content-Length: %lu\r\n",
                      (unsigned long)req->body->len);
        if (ret < 0)
            return -1;
    }

    ret = fprintf(fp, "\r\n");
    if (ret < 0)
        return -1;

    if (req->body != NULL && req->body->len > 0) {
        n = fwrite(req->body->data, 1, req->body->len, fp);
        if (n != req->body->len)
            return -1;
    }

    ret = fflush(fp);
    if (ret != 0)
        return -1;

    return 0;
}

static int read_response(FILE *fp, struct http_response *res)
{
    int ret;
    size_t capacity, required, header_len, body_len, bytes_read;
    char buf[1024];
    char *str, *new;

    ret = readline(fp, buf, sizeof(buf));
    if (ret < 0)
        return -1;

    str = strchr(buf, ' ');
    if (str == NULL)
        return -1;
    res->status_code = atoi(str + 1);

    res->headers.data = NULL;
    res->headers.len = 0;
    capacity = 0;
    body_len = 0;

    for (;;) {
        ret = readline(fp, buf, sizeof(buf));
        if (ret < 0) {
            free(res->headers.data);
            return -1;
        }

        if (strcmp(buf, "\r\n") == 0)
            break;

        header_len = strlen(buf);
        required = header_len + res->headers.len;

        if (required > capacity) {
            capacity = required * 2;
            new = realloc(res->headers.data, capacity);
            if (new == NULL) {
                free(res->headers.data);
                return -1;
            }

            res->headers.data = new;
        }

        memcpy(res->headers.data + res->headers.len, buf, header_len);
        res->headers.len += header_len;

        str = strchr(buf, ':');
        if (str == NULL)
            continue; /* Don't attempt to parse bad headers */
        *str = '\0';

        if (strcmp(buf, "Content-Length") == 0)
            body_len = atoi(str + 1);
    }

    if (body_len > 0) {
        res->body.len = body_len;
        res->body.data = malloc(body_len);
        if (res->body.data == NULL) {
            free(res->headers.data);
            return -1;
        }

        bytes_read = fread(res->body.data, 1, body_len, fp);
        if (bytes_read != body_len) {
            free(res->headers.data);
            free(res->body.data);
            return -1;
        }
    } else {
        res->body.data = NULL;
        res->body.len = 0;
    }

    return 0;
}

static int readline(FILE *fp, char *buffer, size_t size)
{
    size_t len;

    if (fgets(buffer, size, fp) == NULL)
        return -1;

    len = strlen(buffer);
    if (len < 2 || buffer[len - 2] != '\r' || buffer[len - 1] != '\n')
        return -1;

    return 0;
}
