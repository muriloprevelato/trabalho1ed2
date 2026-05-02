#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "hash_extensivel.h"

#define MAX_CHAVE 20 // A chave pode ser tanto um cep (9 caracteres) ou um cpf (14 caracteres). Deixei uma margem.
#define MAX_DADOS 256
#define MAX_CAP_BUCKET 100 // teto (segurança).
#define MAX_PROFUNDIDADE 20

// Tamanho do cabeçalho .hfc (profundidade_global + cap_bucket)
#define HEADER_SIZE ((long)(2 * sizeof(int))) // meta-dados

typedef struct{
    char chave[MAX_CHAVE];
    char dados[MAX_DADOS];
    int ativo; // 1 = ocupado,  0 = removido/livre.
} Registro;

typedef struct{
    int profundidade_local;
    int qtd_registro;
    Registro registros[MAX_CAP_BUCKET];
} Bucket;

struct HashExtensivel{
    FILE* arquivo_dados; // .hf
    FILE* arquivo_cabecalho; // .hfc - diretório/cabeçalho.
    int profundidade_global;
    int cap_bucket;
    int num_splits; // registro no dump.
};

// Início - funções auxiliares.

static unsigned int hash_chave(const char *chave, int profundidade){
    unsigned long h = 5381;
    const unsigned char *s = (const unsigned char *)chave;
    int c;
    while((c = *s++)){
        h = ((h << 5) + h) + (unsigned long)c;
    }
    return (unsigned int)(h & ((1u << profundidade) - 1));
}
// "Função de hashing" - algoritmo djb2 !!!!

// djb2 dá um indice no cabecalho que contém o endereço(offset) do balde de interesse.
static int ler_offset_dir(HashExtensivel *hash, int indice) {
    int offset;

    fseek(hash->arquivo_cabecalho, HEADER_SIZE + (long)(indice * (int)sizeof(int)), SEEK_SET);
    fread(&offset, sizeof(int), 1, hash->arquivo_cabecalho);

    return offset;
}

static void gravar_offset_dir(HashExtensivel *hash, int indice, int offset) {
    fseek(hash->arquivo_cabecalho, HEADER_SIZE + (long)(indice * (int)sizeof(int)), SEEK_SET);
    fwrite(&offset, sizeof(int), 1, hash->arquivo_cabecalho);
}

static void ler_bucket(HashExtensivel *hash, int offset, Bucket* b){
    fseek(hash->arquivo_dados, (long)offset, SEEK_SET);
    fread(b, sizeof(Bucket), 1, hash->arquivo_dados);
}

static void gravar_bucket(HashExtensivel *hash, int offset, const Bucket *b){
    fseek(hash->arquivo_dados, (long)offset, SEEK_SET);
    fwrite(b, sizeof(Bucket), 1, hash->arquivo_dados);
    fflush(hash->arquivo_dados);
}

static int buscar_no_bucket(const Bucket *b, const char *chave, int cap) {
    for(int i = 0; i < cap; i++){
        if(b->registros[i].ativo && strcmp(b->registros[i].chave, chave) == 0){
            return i;
        }
    }
    return -1;
}

void duplicar_diretorio(HashExtensivel *hash){
    unsigned int tamanho_antigo = 1u << (unsigned)hash->profundidade_global;
    int *diretorio = (int*) malloc((size_t)tamanho_antigo * sizeof(int));
    if(!diretorio) return;

    fseek(hash->arquivo_cabecalho, HEADER_SIZE, SEEK_SET);
    fread(diretorio, sizeof(int), (size_t)tamanho_antigo, hash->arquivo_cabecalho);

    hash->profundidade_global++;

    fseek(hash->arquivo_cabecalho, 0, SEEK_SET);
    fwrite(&hash->profundidade_global, sizeof(int), 1, hash->arquivo_cabecalho);
    fwrite(&hash->cap_bucket, sizeof(int), 1, hash->arquivo_cabecalho);

    fwrite(diretorio, sizeof(int), (size_t)tamanho_antigo, hash->arquivo_cabecalho);
    fwrite(diretorio, sizeof(int), (size_t)tamanho_antigo, hash->arquivo_cabecalho);

    fflush(hash->arquivo_cabecalho);
    free(diretorio);
}

// Fim - funções auxiliares.

HashExtensivel* abrirHash(const char* nome_arq, int cap_bucket){
    // Tratamento de caminhos infelizes.
    if(nome_arq == NULL) return NULL;
    if(cap_bucket <= 0 || cap_bucket > MAX_CAP_BUCKET) return NULL;

    HashExtensivel* hash = (HashExtensivel*) malloc(sizeof(HashExtensivel));
    if(hash == NULL)
        return NULL; // Verificação;

    hash->cap_bucket = cap_bucket;
    hash->num_splits = 0;
    
    char nome_hf[256];
    char nome_hfc[256];
    snprintf(nome_hf, sizeof(nome_hf), "%s.hf", nome_arq);
    snprintf(nome_hfc, sizeof(nome_hfc), "%s.hfc", nome_arq);

    FILE* f_dat = fopen(nome_hf, "rb+");
    FILE* f_dir = fopen(nome_hfc, "rb+");
    if(!f_dir || !f_dat){
        // Algum arquivo não existe!
        if(f_dir) fclose(f_dir);
        if(f_dat) fclose(f_dat);

        f_dat = fopen(nome_hf, "wb+");
        f_dir = fopen(nome_hfc, "wb+");
        if(!f_dat || !f_dir) {
            if(f_dat) fclose(f_dat);
            if(f_dir) fclose(f_dir);
            free(hash);
            return NULL;
        }
    
        hash->profundidade_global = 1;

        // Escrevendo o cabeçalho
        fwrite(&hash->profundidade_global, sizeof(int), 1, f_dir);
        fwrite(&hash->cap_bucket, sizeof(int), 1, f_dir);

        int offset_inicial = 0;
        fwrite(&offset_inicial, sizeof(int), 1, f_dir);
        fwrite(&offset_inicial, sizeof(int), 1, f_dir);
        fflush(f_dir);

        Bucket balde_vazio;
        memset(&balde_vazio, 0, sizeof(Bucket));
        balde_vazio.profundidade_local = 1;
        balde_vazio.qtd_registro = 0;
        for(int i = 0; i < MAX_CAP_BUCKET; i++) balde_vazio.registros[i].ativo = 0;
        fwrite(&balde_vazio, sizeof(Bucket), 1, f_dat);
        fflush(f_dat);

    } else{
        // Dois arquivos existem
        fread(&hash->profundidade_global, sizeof(int), 1, f_dir);
        int cap_gravado;
        fread(&cap_gravado, sizeof(int), 1, f_dir);
        hash->cap_bucket = cap_gravado;
    }

    hash->arquivo_dados = f_dat;
    hash->arquivo_cabecalho = f_dir;
    return hash;
}

int inserirHash(HashExtensivel* hash, const char *chave, const char *dados){
    if(!hash || !chave || !dados)
        return HASH_ERRO;
    
    int indice = (int)hash_chave(chave, hash->profundidade_global);
    int offset = ler_offset_dir(hash, indice);

    // Lendo um balde.
    Bucket balde;
    ler_bucket(hash, offset, &balde);

    // Tratamento - Chave duplicada
    if(buscar_no_bucket(&balde, chave, hash->cap_bucket) >= 0)
        return HASH_ERRO;

    // Balde com espaço, procura a primeira oportunidade para inserir.
    if(balde.qtd_registro < hash->cap_bucket){
        for(int i = 0; i < hash->cap_bucket; i++){
            if(balde.registros[i].ativo == 0){
                strncpy(balde.registros[i].chave, chave, MAX_CHAVE - 1);
                balde.registros[i].chave[MAX_CHAVE - 1] = '\0';
                strncpy(balde.registros[i].dados, dados, MAX_DADOS - 1);
                balde.registros[i].dados[MAX_DADOS - 1] = '\0';
                balde.registros[i].ativo = 1;
                balde.qtd_registro++;
                break;
            }
        }
        gravar_bucket(hash, offset, &balde);
        return HASH_OK;
    }

    // BALDE CHEIO

    Registro temporarios[MAX_CAP_BUCKET + 1];
    memset(temporarios, 0, sizeof(temporarios));
    int qtd_temp = 0;

    for(int i = 0; i < hash->cap_bucket; i++){
        if(balde.registros[i].ativo){
            temporarios[qtd_temp++] = balde.registros[i];
        }
    }

    strncpy(temporarios[qtd_temp].chave, chave, MAX_CHAVE - 1);
    temporarios[qtd_temp].chave[MAX_CHAVE - 1] = '\0';
    strncpy(temporarios[qtd_temp].dados, dados, MAX_DADOS - 1);
    temporarios[qtd_temp].dados[MAX_DADOS - 1] = '\0';
    temporarios[qtd_temp].ativo = 1;
    qtd_temp++;

    int resolvido = 0;

    while(!resolvido){
        if(balde.profundidade_local == hash->profundidade_global){
            if(hash->profundidade_global >= MAX_PROFUNDIDADE){
                // FIX: Usando hash->cap_bucket em vez do MAX_CAP_BUCKET
                balde.qtd_registro = hash->cap_bucket;
                for(int i = 0; i < hash->cap_bucket; i++){
                    balde.registros[i] = temporarios[i];
                }
                gravar_bucket(hash, offset, &balde);
                return HASH_ERRO;
            }
            duplicar_diretorio(hash);
        }

        hash->num_splits++;
        int nova_prof_local = balde.profundidade_local + 1;
        int bit_decisao = 1 << (nova_prof_local - 1);

        Registro reg0[MAX_CAP_BUCKET + 1];
        Registro reg1[MAX_CAP_BUCKET + 1];
        int qtd0 = 0;
        int qtd1 = 0;

        for(int i = 0; i < qtd_temp; i++){
            int hash_val = (int)hash_chave(temporarios[i].chave, nova_prof_local);
            if((hash_val & bit_decisao) == 0){
                reg0[qtd0++] = temporarios[i];
            } else{
                reg1[qtd1++] = temporarios[i];
            }
        }

        Bucket novo_balde;
        memset(&novo_balde, 0, sizeof(Bucket));
        novo_balde.profundidade_local = nova_prof_local;
        balde.profundidade_local = nova_prof_local;

        fseek(hash->arquivo_dados, 0, SEEK_END);
        int novo_offset = (int)ftell(hash->arquivo_dados);
        
        // A reserva salva-vidas para o offset não ser roubado!
        gravar_bucket(hash, novo_offset, &novo_balde);

        int total_entradas = 1 << hash->profundidade_global;
        for(int i = 0; i < total_entradas; i++){
            if(ler_offset_dir(hash, i) == offset){
                if((i & bit_decisao) != 0){
                    gravar_offset_dir(hash, i, novo_offset);
                }
            }
        }
        fflush(hash->arquivo_cabecalho);

        // FIX: Usando hash->cap_bucket na validação da cascata!
        if(qtd0 <= hash->cap_bucket && qtd1 <= hash->cap_bucket){
            balde.qtd_registro = qtd0;
            for(int i = 0; i < qtd0; i++) balde.registros[i] = reg0[i];
            for(int i = qtd0; i < MAX_CAP_BUCKET; i++) balde.registros[i].ativo = 0;

            novo_balde.qtd_registro = qtd1;
            for(int i = 0; i < qtd1; i++) novo_balde.registros[i] = reg1[i];

            gravar_bucket(hash, offset, &balde);
            gravar_bucket(hash, novo_offset, &novo_balde);
            resolvido = 1;
        } else {
            // FIX: Usando hash->cap_bucket
            if(qtd0 > hash->cap_bucket){
                novo_balde.qtd_registro = qtd1;
                for(int i = 0; i < qtd1; i++) novo_balde.registros[i] = reg1[i];
                gravar_bucket(hash, novo_offset, &novo_balde);

                qtd_temp = qtd0;
                for(int i = 0; i < qtd0; i++) temporarios[i] = reg0[i];
            } else {
                balde.qtd_registro = qtd0;
                for(int i = 0; i < qtd0; i++) balde.registros[i] = reg0[i];
                for(int i = qtd0; i < MAX_CAP_BUCKET; i++) balde.registros[i].ativo = 0;
                gravar_bucket(hash, offset, &balde);

                offset = novo_offset;
                
                qtd_temp = qtd1;
                for(int i = 0; i < qtd1; i++) temporarios[i] = reg1[i];
            }
        }
    }

    return HASH_OK;
}

int buscarHash(HashExtensivel* hash, const char *chave, char* buffer_saida, int tam_buffer){
    if(!hash || !chave || !buffer_saida || tam_buffer <= 0)
        return HASH_ERRO;

    // Encontrando o balde, através dos bits no arquivo diretório.
    int indice_dir = hash_chave(chave, hash->profundidade_global);
    int offset_balde = ler_offset_dir(hash, indice_dir);

    Bucket balde;
    ler_bucket(hash, offset_balde, &balde);

    int slot = buscar_no_bucket(&balde, chave, hash->cap_bucket);
    if(slot < 0) return HASH_ERRO;

    // Tratamento - buffer insuficiente
    if((int)strlen(balde.registros[slot].dados) + 1 > tam_buffer) return HASH_ERRO;

    strncpy(buffer_saida, balde.registros[slot].dados, tam_buffer - 1);
    buffer_saida[tam_buffer - 1] = '\0';
    return HASH_OK;
}

int removerHash(HashExtensivel* hash, const char *chave){
    if(!hash || !chave)
        return HASH_ERRO;

    // Mesmo procedimento para ter a posição do balde desse id.
    int indice_dir = hash_chave(chave, hash->profundidade_global);
    int offset_balde = ler_offset_dir(hash, indice_dir);
    

    Bucket balde;
    ler_bucket(hash, offset_balde, &balde);

    int slot = buscar_no_bucket(&balde, chave, hash->cap_bucket);
    if(slot < 0) return HASH_ERRO;

    balde.registros[slot].ativo = 0;
    balde.qtd_registro--;
    gravar_bucket(hash, offset_balde, &balde);
    return HASH_OK;
}

void fecharHash(HashExtensivel* hash){
    if(!hash) return;
    if(hash != NULL){
        if(hash->arquivo_cabecalho != NULL){
            fclose(hash->arquivo_cabecalho);
        }
        if(hash->arquivo_dados != NULL){
            fclose(hash->arquivo_dados);
        }
        free(hash);
    }
}

void dumpHash(HashExtensivel* hash, const char* nome_arq){
    if(!hash || !nome_arq) return;
 
    char nome_hfd[256];
    snprintf(nome_hfd, sizeof(nome_hfd), "%s.hfd", nome_arq);
    FILE* f = fopen(nome_hfd, "wb");
    if(!f) return;
 
    int total = 1 << hash->profundidade_global;
 
    // Carrega diretório inteiro em memória 
    int *dir = (int *)malloc((size_t)total * sizeof(int));
    if (!dir) { fclose(f); return; }
    fseek(hash->arquivo_cabecalho, HEADER_SIZE, SEEK_SET);
    fread(dir, sizeof(int), (size_t)total, hash->arquivo_cabecalho);
 
    // Array de visitados indexado por posição do bucket 
    fseek(hash->arquivo_dados, 0, SEEK_END);
    int tam_arquivo = (int)ftell(hash->arquivo_dados);
    int max_buckets = tam_arquivo / (int)sizeof(Bucket) + 1;
    char *visitado = (char *)calloc((size_t)max_buckets, 1);
    if (!visitado) { free(dir); fclose(f); return; }
 
    // BOM UTF-16
    unsigned char bom[2] = {0xFF, 0xFE};
    fwrite(bom, 1, 2, f);
 
    #define WC(c) do { \
        unsigned char _b[2] = {(unsigned char)(c), 0}; \
        fwrite(_b, 1, 2, f); \
    } while(0)

    #define W(str) do { \
        const char *_s = (str); \
        while(*_s){ WC(*_s); _s++; } \
    } while(0)

    #define WLINE(buf) do { \
        W(buf); WC('\r'); WC('\n'); \
    } while(0)
 
    char linha[512];
    int size_record = (int)sizeof(Registro);
    int size_block  = (int)sizeof(Bucket);
 
    // Buckets únicos
    int num_buckets = 0;
    for(int i = 0; i < total; i++){
        int idx = dir[i] / (int)sizeof(Bucket);
        if(!visitado[idx]){ visitado[idx] = 1; num_buckets++; }
    }
    memset(visitado, 0, (size_t)max_buckets);
 
    /* Cabeçalho */
    WLINE("DUMP");
    WLINE("*Dump cabecalho");
    snprintf(linha, sizeof(linha), "numBuckets %d",    num_buckets);  WLINE(linha);
    snprintf(linha, sizeof(linha), "sizeRecord %d",    size_record);  WLINE(linha);
    snprintf(linha, sizeof(linha), "sizeBlock %d",     size_block);   WLINE(linha);
    snprintf(linha, sizeof(linha), "offsetKey %d",     0);            WLINE(linha);
    snprintf(linha, sizeof(linha), "sizeKey %d",       MAX_CHAVE);    WLINE(linha);
    snprintf(linha, sizeof(linha), "offsetTable %d",   (int)HEADER_SIZE); WLINE(linha);
    snprintf(linha, sizeof(linha), "offsetBuckets %d", 0);            WLINE(linha);
    snprintf(linha, sizeof(linha), "offsetOverflow %d", -1);          WLINE(linha);
 
    /* Diretório */
    WLINE("* Dump table");
    for(int i = 0; i < total; i++){
        snprintf(linha, sizeof(linha), "[%d] %d", i, dir[i]);
        WLINE(linha);
    }
 
    /* Buckets */
    WLINE("*Dump buckets");
    int bloco = 0;
    for(int i = 0; i < total; i++){
        int offset = dir[i];
        int idx    = offset / (int)sizeof(Bucket);
 
        if(visitado[idx]) continue;
        visitado[idx] = 1;
 
        Bucket balde;
        ler_bucket(hash, offset, &balde);
 
        snprintf(linha, sizeof(linha), "BLOCO: %d", bloco++);
        WLINE(linha);
 
        for(int k = 0; k < hash->cap_bucket; k++){
            char dados_u[MAX_DADOS];
            strncpy(dados_u, balde.registros[k].dados, MAX_DADOS - 1);
            dados_u[MAX_DADOS - 1] = '\0';
            for(int m = 0; dados_u[m]; m++)
                if(dados_u[m] == ' ') dados_u[m] = '_';
 
            snprintf(linha, sizeof(linha), "%d | %s | %s | %d |",
                balde.registros[k].ativo,
                balde.registros[k].chave,
                dados_u,
                balde.profundidade_local);
            WLINE(linha);
        }
    }
 
    WLINE("FIM DUMP");
 
    #undef WC
    #undef W
    #undef WLINE
 
    free(visitado);
    free(dir);
    fclose(f);
}

void iterarHash(HashExtensivel *hash,
                void (*callback)(const char *chave, const char *dados, void *cmd),
                void *cmd) {
    if (!hash || !callback) return;

    int total_dir = 1 << hash->profundidade_global;

    /* Carrega o diretório inteiro em memória */
    int *dir = (int *)malloc((size_t)total_dir * sizeof(int));
    if (!dir) return;

    fseek(hash->arquivo_cabecalho, HEADER_SIZE, SEEK_SET);
    fread(dir, sizeof(int), (size_t)total_dir, hash->arquivo_cabecalho);

    /* Array de visitados indexado pelo offset */
    fseek(hash->arquivo_dados, 0, SEEK_END);
    int tam_arquivo = (int)ftell(hash->arquivo_dados);
    int max_buckets = tam_arquivo / (int)sizeof(Bucket) + 1;

    char *visitado = (char *)calloc((size_t)max_buckets, 1);
    if (!visitado) { free(dir); return; }

    for (int i = 0; i < total_dir; i++) {
        int offset = dir[i];
        int idx = offset / (int)sizeof(Bucket);

        if (visitado[idx]) continue;
        visitado[idx] = 1;

        Bucket b;
        ler_bucket(hash, offset, &b);

        for (int k = 0; k < hash->cap_bucket; k++) {
            if (b.registros[k].ativo)
                callback(b.registros[k].chave, b.registros[k].dados, cmd);
        }
    }

    free(visitado);
    free(dir);
}