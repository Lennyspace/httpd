#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

struct string;

struct requete_http
{
    struct string *methode;
    struct string *cible;
    struct string *version;
    struct string *host;
    int content_length;
    struct string *body;
};

struct reponse_http
{
    int code_statut;
    char *raison;
    char *date;
    size_t content_length;
    char *connection;
    void *body;
    size_t taille_body;
};

struct requete_http *parse_requete(struct string *donnees_brutes);
struct reponse_http *creer_reponse(int code_statut, const char *raison,
                                   const char *contenu, size_t taille_contenu);
struct string *reponse_vers_string(struct reponse_http *reponse);

void liberer_requete(struct requete_http *requete);
void liberer_reponse(struct reponse_http *reponse);

int methode_supportee(struct string *methode);
int version_supportee(struct string *version);


#endif /* HTTP_H */
