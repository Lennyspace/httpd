#define _XOPEN_SOURCE 500
#include "http.h"

#include <err.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "../utils/string/string.h"

static int trouve_position(struct string *chaine, const char *motif,
                           size_t taille_motif)
{
    if (!chaine || !motif || chaine->size < taille_motif)
        return -1;

    for (size_t i = 0; i <= chaine->size - taille_motif; i++)
    {
        size_t trouve = 1;
        for (size_t j = 0; j < taille_motif; j++)
        {
            if (chaine->data[i + j] != motif[j])
            {
                trouve = 0;
                break;
            }
        }
        if (trouve)
            return i;
    }
    return -1;
}

static int parse_ligne_requete(struct requete_http *requete,
                               struct string *ligne)
{
    if (!requete || !ligne)
        return -1;

    int pos_espace1 = -1;
    for (size_t i = 0; i < ligne->size; i++)
    {
        if (ligne->data[i] == ' ')
        {
            pos_espace1 = i;
            break;
        }
    }

    if (pos_espace1 == -1)
        return -1;

    int pos_espace2 = -1;
    for (size_t i = pos_espace1 + 1; i < ligne->size; i++)
    {
        if (ligne->data[i] == ' ')
        {
            pos_espace2 = i;
            break;
        }
    }

    if (pos_espace2 == -1)
        return -1;

    requete->methode = string_create(ligne->data, pos_espace1);

    size_t taille_cible = pos_espace2 - pos_espace1 - 1;
    requete->cible = string_create(ligne->data + pos_espace1 + 1, taille_cible);

    size_t taille_version = ligne->size - pos_espace2 - 1;
    requete->version =
        string_create(ligne->data + pos_espace2 + 1, taille_version);

    if (requete->methode->size == 0 || requete->cible->size == 0
        || requete->version->size == 0 || requete->cible->data[0] != '/')
    {
        if (requete->methode)
        {
            string_destroy(requete->methode);
        }
        if (requete->cible)
        {
            string_destroy(requete->cible);
        }
        if (requete->version)
        {
            string_destroy(requete->version);
        }
        return -1;
    }

    return 0;
}

static int parse_header(struct requete_http *requete, struct string *ligne)
{
    if (!requete || !ligne)
        return -1;

    int pos_deux_points = -1;
    for (size_t i = 0; i < ligne->size; i++)
    {
        if (ligne->data[i] == ':')
        {
            pos_deux_points = i;
            break;
        }
    }

    if (pos_deux_points == -1)
        return -1;

    struct string *nom = string_create(ligne->data, pos_deux_points);

    size_t debut_valeur = pos_deux_points + 1;
    while (debut_valeur < ligne->size && ligne->data[debut_valeur] == ' ')
        debut_valeur++;

    struct string *valeur =
        string_create(ligne->data + debut_valeur, ligne->size - debut_valeur);

    if (string_compare_n_str(nom, "Host", 4) == 0)
    {
        requete->host = valeur;
    }
    else if (string_compare_n_str(nom, "Content-Length", 14) == 0)
    {
        char buffer[32];
        memcpy(buffer, valeur->data, valeur->size);
        buffer[valeur->size] = '\0';
        requete->content_length = atoi(buffer);
        string_destroy(valeur);
    }
    else
    {
        string_destroy(valeur);
    }

    string_destroy(nom);
    return 0;
}

struct requete_http *parse_requete(struct string *donnees_brutes)
{
    struct requete_http *requete = malloc(sizeof(struct requete_http));
    requete->methode = NULL;
    requete->cible = NULL;
    requete->version = NULL;
    requete->host = NULL;
    requete->body = NULL;
    requete->content_length = 0;

    int pos_fin_headers = trouve_position(donnees_brutes, "\r\n\r\n", 4);
    if (pos_fin_headers == -1)
    {
        free(requete);
        return NULL;
    }

    size_t debut = 0;
    int premiere_ligne = 1;

    for (int i = 0; i < pos_fin_headers; i++)
    {
        if (i > 0 && donnees_brutes->data[i - 1] == '\r'
            && donnees_brutes->data[i] == '\n')
        {
            size_t taille_ligne =
                i - debut - 1; // -1 pour enlever le \r t as capter
            struct string *ligne =
                string_create(donnees_brutes->data + debut, taille_ligne);

            if (premiere_ligne)
            {
                if (parse_ligne_requete(requete, ligne) == -1)
                {
                    string_destroy(ligne);
                    liberer_requete(requete);
                    return NULL;
                }
                premiere_ligne = 0;
            }
            else if (taille_ligne > 0)
            {
                parse_header(requete, ligne);
            }

            string_destroy(ligne);
            debut = i + 1;
        }
    }

    if (requete->content_length > 0)
    {
        size_t debut_body = pos_fin_headers + 4;
        size_t taille_body = donnees_brutes->size - debut_body;
        requete->body =
            string_create(donnees_brutes->data + debut_body, taille_body);
    }

    return requete;
}

static void generer_date_gmt(char *buffer, size_t taille)
{
    time_t maintenant = time(NULL);
    struct tm *tm_gmt = gmtime(&maintenant);
    strftime(buffer, taille, "%a, %d %b %Y %H:%M:%S GMT", tm_gmt);
}

struct reponse_http *creer_reponse(int code_statut, const char *raison,
                                   const char *contenu, size_t taille_contenu)
{
    struct reponse_http *reponse = malloc(sizeof(struct reponse_http));
    reponse->date = NULL;
    reponse->connection = NULL;
    reponse->body = NULL;

    reponse->code_statut = code_statut;
    reponse->raison = strdup(raison);

    char date_buffer[256];
    generer_date_gmt(date_buffer, sizeof(date_buffer));
    reponse->date = strdup(date_buffer);

    reponse->content_length = taille_contenu;
    reponse->connection = strdup("close");

    reponse->body = malloc(taille_contenu);
    memcpy(reponse->body, contenu, taille_contenu);
    reponse->taille_body = taille_contenu;

    return reponse;
}

struct string *reponse_vers_string(struct reponse_http *reponse)
{
    char *buffer = malloc(512 + reponse->taille_body);

    int nbecrit = snprintf(buffer, 512 + reponse->taille_body,
                           "HTTP/1.1 %d %s\r\n"
                           "Date: %s\r\n"
                           "Content-Length: %zu\r\n"
                           "Connection: %s\r\n"
                           "\r\n",
                           reponse->code_statut, reponse->raison, reponse->date,
                           reponse->content_length, reponse->connection);

    struct string *resultat = string_create(buffer, nbecrit);

    if (reponse->body && reponse->taille_body > 0)
    {
        string_concat_str(resultat, reponse->body, reponse->taille_body);
    }

    free(buffer);
    return resultat;
}

void liberer_requete(struct requete_http *requete)
{
    if (!requete)
        return;

    if (requete->methode)
        string_destroy(requete->methode);
    if (requete->cible)
        string_destroy(requete->cible);
    if (requete->version)
        string_destroy(requete->version);
    if (requete->host)
        string_destroy(requete->host);
    if (requete->body)
        string_destroy(requete->body);

    free(requete);
}

void liberer_reponse(struct reponse_http *reponse)
{
    if (!reponse)
        return;

    free(reponse->raison);
    free(reponse->date);
    free(reponse->connection);
    free(reponse->body);
    free(reponse);
}

int methode_supportee(struct string *methode)
{
    return (string_compare_n_str(methode, "GET", 3) == 0
            || string_compare_n_str(methode, "HEAD", 4) == 0);
}

int version_supportee(struct string *version)
{
    return string_compare_n_str(version, "HTTP/1.1", 8) == 0;
}
