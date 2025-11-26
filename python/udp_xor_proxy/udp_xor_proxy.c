/*
 * udp_xor_proxy.c
 *
 * Simple bidirectional UDP proxy with XOR obfuscation
 *
 * Usage examples:
 *   ./udp_xor_proxy -l 12345 -r 4000 -k secret
 *   ./udp_xor_proxy -l 0.0.0.0:12345 -r 192.168.1.10:4000 -k key
 *
 * -l PORT or HOST:PORT  : local listening address (default host 0.0.0.0)
 * -r PORT or HOST:PORT  : remote destination address (default host 127.0.0.1)
 * -k KEY                : XOR key string
 *
 * Build: gcc -O2 -Wall -pthread -o udp_xor_proxy udp_xor_proxy.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUF_SIZE 65535

struct proxy_state {
    int local_fd;
    int remote_fd;
    struct sockaddr_in remote_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    char key[256];
    size_t key_len;
    pthread_mutex_t lock;
};

void xor_buffer(char *buf, size_t len, const char *key, size_t key_len) {
    for (size_t i = 0; i < len; i++) {
        buf[i] ^= key[i % key_len];
    }
}

void *local_to_remote_thread(void *arg) {
    struct proxy_state *state = (struct proxy_state *)arg;
    char buf[BUF_SIZE];

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        ssize_t n = recvfrom(state->local_fd, buf, BUF_SIZE, 0,
                             (struct sockaddr *)&client_addr, &addr_len);
        if (n <= 0) continue;

        xor_buffer(buf, n, state->key, state->key_len);

        // store last client address
        pthread_mutex_lock(&state->lock);
        memcpy(&state->client_addr, &client_addr, sizeof(client_addr));
        state->client_addr_len = addr_len;
        pthread_mutex_unlock(&state->lock);

        sendto(state->remote_fd, buf, n, 0,
               (struct sockaddr *)&state->remote_addr, sizeof(state->remote_addr));
    }
    return NULL;
}

void *remote_to_local_thread(void *arg) {
    struct proxy_state *state = (struct proxy_state *)arg;
    char buf[BUF_SIZE];

    while (1) {
        ssize_t n = recvfrom(state->remote_fd, buf, BUF_SIZE, 0, NULL, NULL);
        if (n <= 0) continue;

        xor_buffer(buf, n, state->key, state->key_len);

        pthread_mutex_lock(&state->lock);
        struct sockaddr_in client = state->client_addr;
        socklen_t client_len = state->client_addr_len;
        pthread_mutex_unlock(&state->lock);

        if (client_len > 0)
            sendto(state->local_fd, buf, n, 0,
                   (struct sockaddr *)&client, client_len);
    }
    return NULL;
}

void parse_hostport(const char *s, char *host, int *port, int is_remote) {
    const char *colon = strchr(s, ':');
    if (colon) {
        size_t len = colon - s;
        if (len > 0) strncpy(host, s, len);
        else strcpy(host, is_remote ? "127.0.0.1" : "0.0.0.0");
        host[len] = '\0';
        *port = atoi(colon + 1);
    } else if (s[0] >= '0' && s[0] <= '9') {
        strcpy(host, is_remote ? "127.0.0.1" : "0.0.0.0");
        *port = atoi(s);
    } else {
        fprintf(stderr, "Invalid host/port: %s\n", s);
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    char *listen_arg = NULL, *remote_arg = NULL, *key_arg = "";
    int listen_port = 12345;
    char listen_host[64] = "0.0.0.0";
    int remote_port = 0;
    char remote_host[64] = "127.0.0.1";

    // Simple argument parsing
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--listen")) listen_arg = argv[++i];
        else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--remote")) remote_arg = argv[++i];
        else if (!strcmp(argv[i], "-k") || !strcmp(argv[i], "--key")) key_arg = argv[++i];
        else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    if (!remote_arg) {
        fprintf(stderr, "Remote address required (-r PORT or HOST:PORT)\n");
        return 1;
    }

    if (listen_arg) parse_hostport(listen_arg, listen_host, &listen_port, 0);
    parse_hostport(remote_arg, remote_host, &remote_port, 1);

    // Create sockets
    int local_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int remote_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (local_fd < 0 || remote_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(listen_port);
    inet_pton(AF_INET, listen_host, &local_addr.sin_addr);
    if (bind(local_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        return 1;
    }

    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    inet_pton(AF_INET, remote_host, &remote_addr.sin_addr);

    printf("Listening on %s:%d <-> %s:%d\n", listen_host, listen_port, remote_host, remote_port);

    struct proxy_state state;
    memset(&state, 0, sizeof(state));
    state.local_fd = local_fd;
    state.remote_fd = remote_fd;
    state.remote_addr = remote_addr;
    strncpy(state.key, key_arg, sizeof(state.key)-1);
    state.key_len = strlen(state.key);
    pthread_mutex_init(&state.lock, NULL);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, local_to_remote_thread, &state);
    pthread_create(&t2, NULL, remote_to_local_thread, &state);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}
