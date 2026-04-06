#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leitorGeo.h"
#include "quadra.h"
#include "hash_extensivel.h"

#define LINHA_MAX 256
#define SW_STR_MAX 16 // "2.0px" -> margem para caber!

// Definindo os padrões
#define SW_PADRAO 1.0
#define CFILL_PADRAO "white"
#define CSTRK_PADRAO "black"

static int processar_linha_cq(const char *linha, double *sw, char *cfill, int tam_cfill, char *cstrk, int tam_cstrk){
    char cmd[8], sw_str[SW_STR_MAX];

    if(sscanf(linha, "%7s %15s %31s %31s", cmd, sw_str, cfill, cstrk) != 4)
        return 0;

    // Atof para primeira letra, então "1.0px" -> 1.0 corretamente.
    *sw = atof(sw_str);

    cfill[tam_cfill - 1] = '\0';
    cstrk[tam_cstrk - 1] = '\0';

    return 1;
}

static int processar_linha_q(const char *linha, double sw, const char *cfill, const char *cstrk, HashExtensivel *hash){
    char cmd[8];
    char cep[QUADRA_CEP_MAX];
    double x, y, w, h;

    if(sscanf(linha, "%7s %31s %lf %lf %lf %lf", cmd, cep, &x, &y, &w, &h) != 6)
        return 0;
    
    Quadra *q = criarQuadra(cep, x, y, w, h, sw, cfill, cstrk);
    if(!q) return 0;

    char buffer[QUADRA_SERIAL_MAX];
    int ok = serializarQuadra(q, buffer, sizeof(buffer));
    destruirQuadra(q);

    if(ok != QUADRA_OK) return 0;

    return inserirHash(hash, cep, buffer) == HASH_OK ? 1 : 0;
}

int lerArqGeo(const char *caminhoAqrGeo, HashExtensivel *HashQuadras){
    if(!caminhoAqrGeo || !HashQuadras) return LEITOR_GEO_ERRO;

    FILE *f = fopen(caminhoAqrGeo, "r");
    if(!f) return LEITOR_GEO_ERRO;

    double sw = SW_PADRAO;
    char cfill[QUADRA_COR_MAX];
    char cstrk[QUADRA_COR_MAX];
    strncpy(cfill, CFILL_PADRAO, QUADRA_COR_MAX - 1); cfill[QUADRA_COR_MAX - 1] = '\0';
    strncpy(cstrk, CSTRK_PADRAO, QUADRA_COR_MAX - 1); cstrk[QUADRA_COR_MAX - 1] = '\0';

    char linha[LINHA_MAX];

    while(fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = '\0';

        if(linha[0] == '\0' || linha[0] == '#') continue;

        if(strncmp(linha, "cq", 2) == 0){
            processar_linha_cq(linha, &sw, cfill, QUADRA_COR_MAX, cstrk, QUADRA_COR_MAX);
        } else if(strncmp(linha, "q ", 2) == 0){
            processar_linha_q(linha, sw, cfill, cstrk, HashQuadras);
        }
    }

    fclose(f);
    return LEITOR_GEO_OK;
}