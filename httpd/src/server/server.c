#define _POSIX_C_SOURCE 200112L
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../http/http.h"
#include "../utils/string/string.h"

#define BUFFER_SIZE 4096

static int stop = 0;

static void signal_handler(int signal)
{
    signal++;
    stop = 1;
}
static void respond(int client_fd, const char *buffer, ssize_t bytes)
{
    ssize_t total = 0;
    ssize_t sent = 0;

    while (total != bytes)
    {
        sent = send(client_fd, buffer + total, bytes - total, 0);
        if (sent == -1)
        {
            printf("Send failed\n");
            return;
        }
        total += sent;
    }
}

static int create_and_bind(const char *node, const char *service)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = NULL;

    if (getaddrinfo(node, service, &hints, &res) == -1)
    {
        printf("create_and_bind: failed getaddrinfo\n");
        return -1;
    }

    int sockfd = -1;

    for (struct addrinfo *p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;

        int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) != -1)
            break;

        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);

    return sockfd;
}
static int find_seq(const char *buf, size_t len, const char *seq, size_t seqlen)
{
    if (len < seqlen)
    {
        return 0;
    }
    for (size_t i = 0; i <= len - seqlen; ++i)
    {
        size_t j = 0;
        while (j < seqlen && buf[i + j] == seq[j])
            ++j;
        if (j == seqlen)
            return 1;
    }
    return 0;
}
static void communicate(int client_fd, char *root_dir, char *default_file)
{
    ssize_t bytes = 0;
    size_t totalread = 0;
    char buffer[BUFFER_SIZE];
    while (1)
    {
        if ((bytes = recv(client_fd, buffer + totalread,
                          BUFFER_SIZE - totalread - 1, 0))
            <= 0)
        {
            if (bytes == 0)
            {
                printf("Client closed connection\n");
            }
            else
            {
                printf("error Receiving data\n");
            }
            return;
        }
        totalread += bytes;
        buffer[totalread] = 0;

        if (find_seq(buffer, totalread, "\r\n\r\n", 4))
        {
            printf("Received all request\n");
            break;
        }
    }
    struct string *donnees = string_create(buffer, totalread);
    struct requete_http *requete = parse_requete(donnees);

    struct reponse_http *response = NULL;
    struct string *res_str = NULL;
    if (requete == NULL)
    {
        response = creer_reponse(400, "Bad Request", "", 0);
    }
    else if (methode_supportee(requete->methode) == 0)
    {
        response = creer_reponse(405, "Method Not Allowed", "", 0);
    }
    else if (version_supportee(requete->version) == 0)
    {
        response = creer_reponse(505, "HTTP Version Not Supported", "", 0);
    }
    else
    {
        char cible_str[256];
        memcpy(cible_str, requete->cible->data, requete->cible->size);
        cible_str[requete->cible->size] = '\0'; // pour avoir le 0 a la fin

        char chemin[1024];
        if (requete->cible->size == 1 && requete->cible->data[0] == '/')
        {
            snprintf(chemin, 1024, "%s/%s", root_dir, default_file);
        }
        else
        {
            snprintf(chemin, 1024, "%s%s", root_dir, cible_str);
        }

        int fd = open(chemin, O_RDONLY);
        if (fd == -1)
        {
            if (errno == EACCES)
            {
                response = creer_reponse(403, "Forbidden", "", 0);
            }
            else
            {
                response = creer_reponse(404, "Not Found", "", 0);
            }
        }
        else
        {
            struct stat st;
            fstat(fd, &st);
            if ((string_compare_n_str(requete->methode, "HEAD", 4) == 0))
            {
                close(fd);
                response = creer_reponse(200, "OK", "", 0);
                response->content_length = st.st_size;
            }
            else
            {
                char *contenue = malloc(st.st_size);
                read(fd, contenue, st.st_size);
                close(fd);

                response = creer_reponse(200, "OK", contenue, st.st_size);

                free(contenue);
            }
        }
    }

    res_str = reponse_vers_string(response);

    respond(client_fd, res_str->data, res_str->size);
    string_destroy(res_str);
    liberer_requete(requete);
    liberer_reponse(response);
    string_destroy(donnees);
}

static void start_server(int server_socket, char *root_dir, char *default_file)
{
    if (listen(server_socket, SOMAXCONN) == -1)
        return;

    struct sigaction siga;
    siga.sa_flags = 0;
    siga.sa_handler = signal_handler;
    if (sigemptyset(&siga.sa_mask) < 0)
    {
        return;
    }
    if (sigaction(SIGINT, &siga, NULL) == -1)
    {
        return;
    }
    while (!stop)
    {
        int cfd = accept(server_socket, NULL, NULL);
        if (cfd != -1)
        {
            printf("Client connected\n");
            communicate(cfd, root_dir, default_file);
            close(cfd);
            printf("Client disconnected\n");
        }
    }
}

void start_serv(char *ip, char *port, char *root_dir, char *default_file)
{
    int socket_fd = create_and_bind(ip, port);
    if (socket_fd == -1)
    {
        return;
    }

    start_server(socket_fd, root_dir, default_file);
    close(socket_fd);
}
