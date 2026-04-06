#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quadra.h"

struct Quadra{
    char cep[QUADRA_CEP_MAX];
    char cfill[QUADRA_COR_MAX];
    char cstrk[QUADRA_COR_MAX];
    double x, y, w, h, sw;
};

Quadra* criarQuadra(const char* cep, double x, double y, double w, double h, double sw, const char* cfill, const char* cstrk){
    if(!cep || !cfill || !cstrk) return NULL;
    if(w < 0.0 || h < 0.0) return NULL;

    Quadra *q = (Quadra*) malloc(sizeof(Quadra));
    if(q == NULL){
        return NULL;
    }

    strncpy(q->cep, cep, QUADRA_CEP_MAX - 1); q->cep[QUADRA_CEP_MAX - 1] = '\0';
    strncpy(q->cfill, cfill, QUADRA_COR_MAX - 1); q->cfill[QUADRA_COR_MAX - 1] = '\0';
    strncpy(q->cstrk, cstrk, QUADRA_COR_MAX - 1); q->cstrk[QUADRA_COR_MAX - 1] = '\0';

    q->x = x;
    q->y = y;
    q->w = w;
    q->h = h;
    q->sw = sw;

    return q;
}

void destruirQuadra(Quadra *q){
    // Como foram delimitados vetores estaticamente, só precisamos dar um unico free.
    free(q);
    // NULL é tolerado!
}

const char* getQuadraCep(const Quadra *q) { return q ? q->cep : NULL; }
const char* getQuadraCFill(const Quadra *q) { return q ? q->cfill : NULL; }
const char* getQuadraCStrk(const Quadra *q) { return q ? q->cstrk : NULL; }
 
double getQuadraX (const Quadra *q) { return q ? q->x  : 0.0; }
double getQuadraY (const Quadra *q) { return q ? q->y  : 0.0; }
double getQuadraW (const Quadra *q) { return q ? q->w  : 0.0; }
double getQuadraH (const Quadra *q) { return q ? q->h  : 0.0; }
double getQuadraSw(const Quadra *q) { return q ? q->sw : 0.0; }

int quadraContemPonto(const Quadra *q, double px, double py){
    if(!q) return QUADRA_ERRO;
    if(px < q->x || px > q->x + q->w) return QUADRA_ERRO;
    if(py < q->y || py > q->y + q->h) return QUADRA_ERRO;
    return QUADRA_OK;
}

int faceValida(char face) {
    return (face == 'O' || face == 'L' || face == 'N' || face == 'S') ? QUADRA_OK : QUADRA_ERRO;
}

int serializarQuadra(const Quadra *q, char *buffer, int tam) {
    if(!q || !buffer || tam <= 0) return QUADRA_ERRO;

    int escrito = snprintf(buffer, (size_t)tam, "%s|%.6f|%.6f|%.6f|%.6f|%.6f|%s|%s", q->cep, q->x, q->y, q->w, q->h, q->sw, q->cfill, q->cstrk);

    if(escrito < 0 || escrito >= tam) return QUADRA_ERRO;

    return QUADRA_OK;
}

Quadra *desserializarQuadra(const char *buffer){
    if(!buffer) return NULL;

    char copia[QUADRA_SERIAL_MAX];
    strncpy(copia, buffer, QUADRA_SERIAL_MAX - 1);
    copia[QUADRA_SERIAL_MAX - 1] = '\0';

    char cep[QUADRA_CEP_MAX];
    char cfill[QUADRA_COR_MAX];
    char cstrk[QUADRA_COR_MAX];
    double x, y, w, h, sw;

    // Extraindo cep.
    char *token = strtok(copia, "|");
    if(!token) return NULL;
    strncpy(cep, token, QUADRA_CEP_MAX - 1);
    cep[QUADRA_CEP_MAX - 1] = '\0';

    // Extraindo cinco campos númericos.
    double *numericos[] = { &x, &y, &w, &h, &sw};
    for(int i = 0; i < 5; i++){
        token = strtok(NULL, "|");
        if(!token) return NULL;
        *numericos[i] = atof(token);
    }

    // Extraindo cfill.
    token = strtok(NULL, "|");
    if(!token) return NULL;
    strncpy(cfill, token, QUADRA_COR_MAX - 1);
    cfill[QUADRA_COR_MAX - 1] = '\0';

    // Extraindo cstrk.
    token = strtok(NULL, "|");
    if(!token) return NULL;
    strncpy(cstrk, token, QUADRA_COR_MAX - 1);
    cstrk[QUADRA_COR_MAX - 1] = '\0';

    return criarQuadra(cep, x, y, w, h, sw, cfill, cstrk);
}