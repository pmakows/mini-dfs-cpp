#include "cache.h"
#include "cache_stats.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 9100
#define REQUEST_BUFFER_SIZE 16384
#define RESPONSE_BUFFER_SIZE 8192

static cache_t cache;

static int get_port(void) {
    const char *env_port = getenv("CACHE_PORT");

    if (env_port == NULL) {
        return DEFAULT_PORT;
    }

    return atoi(env_port);
}

static void send_response(
    int client_fd,
    int status,
    const char *status_text,
    const char *content_type,
    const char *body,
    size_t body_len
) {
    char header[1024];

    int header_len = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status,
        status_text,
        content_type,
        body_len
    );

    send(client_fd, header, header_len, 0);

    if (body != NULL && body_len > 0) {
        send(client_fd, body, body_len, 0);
    }
}

static char *find_body(char *request) {
    char *body = strstr(request, "\r\n\r\n");

    if (body == NULL) {
        return NULL;
    }

    return body + 4;
}

static size_t get_content_length(const char *request) {
    const char *header = strstr(request, "Content-Length:");

    if (header == NULL) {
        return 0;
    }

    header += strlen("Content-Length:");

    while (*header == ' ') {
        header++;
    }

    return (size_t)strtoull(header, NULL, 10);
}

static void handle_client(int client_fd) {
    char request[REQUEST_BUFFER_SIZE];

    memset(request, 0, sizeof(request));

    ssize_t received = recv(client_fd, request, sizeof(request) - 1, 0);

    if (received <= 0) {
        close(client_fd);
        return;
    }

    char method[16];
    char path[512];

    if (sscanf(request, "%15s %511s", method, path) != 2) {
        const char *body = "BAD REQUEST\n";
        send_response(client_fd, 400, "Bad Request", "text/plain", body, strlen(body));
        close(client_fd);
        return;
    }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/health") == 0) {
        const char *body = "OK\n";
        send_response(client_fd, 200, "OK", "text/plain", body, strlen(body));
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/stats") == 0) {
        char json[RESPONSE_BUFFER_SIZE];

        cache_stats_to_json(&cache.stats, json, sizeof(json));
        strcat(json, "\n");

        send_response(client_fd, 200, "OK", "application/json", json, strlen(json));
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/reset-stats") == 0) {
        cache_reset_stats(&cache);

        const char *body = "{\"status\":\"reset\"}\n";
        send_response(client_fd, 200, "OK", "application/json", body, strlen(body));
    } else if (strncmp(path, "/cache/", 7) == 0) {
        const char *key = path + 7;

        if (strcmp(method, "PUT") == 0) {
            size_t content_length = get_content_length(request);
            char *body = find_body(request);

            if (body == NULL) {
                const char *response = "BAD REQUEST\n";
                send_response(client_fd, 400, "Bad Request", "text/plain", response, strlen(response));
            } else {
                size_t available_body_len = (size_t)(received - (body - request));
                size_t body_len = content_length;

                if (body_len == 0 || body_len > available_body_len) {
                    const char *response = "BAD REQUEST\n";
                    send_response(client_fd, 400, "Bad Request", "text/plain", response, strlen(response));
                    close(client_fd);
                    return;
                }

                cache_put(&cache, key, body, body_len);
                cache.stats.puts++;

                const char *response = "OK\n";
                send_response(client_fd, 200, "OK", "text/plain", response, strlen(response));
            }
        } else if (strcmp(method, "GET") == 0) {
            char value[CACHE_VALUE_SIZE];
            size_t value_len = 0;

            int found = cache_get(
                &cache,
                key,
                value,
                sizeof(value),
                &value_len
            );

            cache.stats.gets++;

            if (!found) {
                cache.stats.misses++;

                const char *response = "MISS\n";
                send_response(client_fd, 404, "Not Found", "text/plain", response, strlen(response));
            } else {
                cache.stats.hits++;

                send_response(client_fd, 200, "OK", "application/octet-stream", value, value_len);
            }
        } else if (strcmp(method, "DELETE") == 0) {
            cache_invalidate(&cache, key);
            cache.stats.invalidations++;

            const char *response = "DELETED\n";
            send_response(client_fd, 200, "OK", "text/plain", response, strlen(response));
        } else {
            const char *response = "METHOD NOT ALLOWED\n";
            send_response(client_fd, 405, "Method Not Allowed", "text/plain", response, strlen(response));
        }
    } else {
        const char *body = "NOT FOUND\n";
        send_response(client_fd, 404, "Not Found", "text/plain", body, strlen(body));
    }

    close(client_fd);
}

int main(void) {
    int port = get_port();

    cache_init(&cache);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    struct sockaddr_in address;

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 16) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Cache node listening on port %d\n", port);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        handle_client(client_fd);
    }

    close(server_fd);

    return 0;
}