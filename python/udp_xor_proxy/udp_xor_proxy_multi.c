/*
 * udp_xor_proxy_multi_fixed.c
 *
 * Multi-client bidirectional UDP XOR proxy (per-client upstream socket)
 *
 * Each local client gets its own upstream socket to the fixed remote.
 * Bidirectional XOR obfuscation applied.
 *
 * Build:
 *   gcc -O2 -Wall -pthread -o udp_xor_proxy_multi_fixed udp_xor_proxy_multi_fixed.c
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
#include <time.h>
#include <fcntl.h>

#define BUF_SIZE 65535
#define CLIENT_TIMEOUT 60  // seconds

struct client {
    struct sockaddr_in addr;   // local client address
    int remote_fd;             // upstream socket
    time_t last_seen;
    struct client *next;
};

struct proxy {
    int local_fd;
    struct sockaddr_in remote_addr;
    char key[256];
    size_t key_len;
    pthread_mutex_t clients_lock;
    struct client *clients;
};

// XOR buffer with key
void xor_buffer(char *buf, size_t len, const char *key, size_t key_len) {
    for (size_t i = 0; i < len; i++) {
        buf[i] ^= key[i % key_len];
    }
}

// Compare sockaddr_in
int addr_equal(struct sockaddr_in *a, struct sockaddr_in *b) {
    return a->sin_family == b->sin_family &&
           a->sin_port == b->sin_port &&
           a->sin_addr.s_addr == b->sin_addr.s_addr;
}

// Create or get client struct for a given local address
struct client *get_client(struct proxy *p, struct sockaddr_in *client_addr) {
    pthread_mutex_lock(&p->clients_lock);
    struct client *c = p->clients;
    while (c) {
        if (addr_equal(&c->addr, client_addr)) {
            c->last_seen = time(NULL);
            pthread_mutex_unlock(&p->clients_lock);
            return c;
        }
        c = c->next;
    }

    // New client
    c = calloc(1, sizeof(struct client));
    if (!c) {
        pthread_mutex_unlock(&p->clients_lock);
        return NULL;
    }
    memcpy(&c->addr, client_addr, sizeof(c->addr));
    c->remote_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (c->remote_fd < 0) {
        perror("socket");
        free(c);
        pthread_mutex_unlock(&p->clients_lock);
        return NULL;
    }
    c->last_seen = time(NULL);
    c->next = p->clients;
    p->clients = c;
    pthread_mutex_unlock(&p->clients_lock);

    return c;
}

// Parse host:port or PORT
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

    // Argument parsing
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

    int local_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (local_fd < 0) { perror("socket"); return 1; }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(listen_port);
    inet_pton(AF_INET, listen_host, &local_addr.sin_addr);
    if (bind(local_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind"); return 1;
    }

    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    inet_pton(AF_INET, remote_host, &remote_addr.sin_addr);

    struct proxy p;
    memset(&p, 0, sizeof(p));
    p.local_fd = local_fd;
    p.remote_addr = remote_addr;
    strncpy(p.key, key_arg, sizeof(p.key)-1);
    p.key_len = strlen(key_arg);
    pthread_mutex_init(&p.clients_lock, NULL);

    printf("Listening on %s:%d <-> %s:%d (multi-client per-client sockets)\n",
           listen_host, listen_port, remote_host, remote_port);

    char buf[BUF_SIZE];
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        ssize_t n = recvfrom(local_fd, buf, BUF_SIZE, 0,
                             (struct sockaddr *)&client_addr, &addr_len);
        if (n <= 0) continue;

        xor_buffer(buf, n, p.key, p.key_len);

        struct client *c = get_client(&p, &client_addr);
        if (!c) continue;

        // Send to remote
        sendto(c->remote_fd, buf, n, 0, (struct sockaddr *)&p.remote_addr, sizeof(p.remote_addr));

        // Non-blocking check for remote response
        ssize_t r = recvfrom(c->remote_fd, buf, BUF_SIZE, MSG_DONTWAIT, NULL, NULL);
        if (r > 0) {
            xor_buffer(buf, r, p.key, p.key_len);
            sendto(local_fd, buf, r, 0, (struct sockaddr *)&c->addr, sizeof(c->addr));
        }
    }

    return 0;
}
