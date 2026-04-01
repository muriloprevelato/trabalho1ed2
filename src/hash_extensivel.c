#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_extensivel.h"


#define MAX_CHAVE 20 // A chave pode ser tanto um cep (9 caracteres) ou um cpf (14 caracteres). Deixei uma margem.
#define MAX_DADOS 256
#define MAX_CAP_BUCKET 10 // teto (segurança).

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
    int tamanho_antigo = 1 << hash->profundidade_global;
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

    if(balde.profundidade_local == hash->profundidade_global){
        duplicar_diretorio(hash);
        return inserirHash(hash, chave, dados); // Diretório dobrou, então chamamos a função denovo.
    }

    // Nesse caso aqui vamos tratar quando ainda dá para aumentar a profundidade local.
    // novo balde, dois baldes com profundidade++ em relação a antes. 
    hash->num_splits++;
    Bucket novo_balde; 
    novo_balde.profundidade_local = balde.profundidade_local + 1;
    novo_balde.qtd_registro = 0;
    for(int i = 0; i < MAX_CAP_BUCKET; i++) novo_balde.registros[i].ativo = 0;
    balde.profundidade_local++;

    // limpando balde velho
    Registro temporarios[MAX_CAP_BUCKET + 1];
    int qtd_temp = 0;
    for(int i = 0; i < hash->cap_bucket; i++){
        if(balde.registros[i].ativo){
            temporarios[qtd_temp++] = balde.registros[i];
            balde.registros[i].ativo = 0;
        }
    }
    strncpy(temporarios[qtd_temp].chave, chave, MAX_CHAVE - 1);
    temporarios[qtd_temp].chave[MAX_CHAVE - 1] = '\0';
    strncpy(temporarios[qtd_temp].dados, dados, MAX_DADOS - 1);
    temporarios[qtd_temp].dados[MAX_DADOS - 1] = '\0';
    temporarios[qtd_temp].ativo = 1;
    qtd_temp++;
    balde.qtd_registro = 0;

    // gravar os dois baldes no disco
    fseek(hash->arquivo_dados, 0, SEEK_END);
    int novo_offset = (int)ftell(hash->arquivo_dados);
    gravar_bucket(hash, novo_offset, &novo_balde);
    gravar_bucket(hash, offset, &balde);


    // Atualizando ponteiros
    int total_entradas = 1 << hash->profundidade_global;
    int prof_nova = balde.profundidade_local; // Já foi incrementada!!!!
    int mask_antiga = (1 << (prof_nova - 1)) - 1;
    int hash_antigo = indice & mask_antiga;
    int bit_novo = 1 << (prof_nova - 1);

    for(int i = 0; i < total_entradas; i++){
        if((i & mask_antiga) == hash_antigo){
            int offset_correto = (i & bit_novo) ? novo_offset : offset;
            gravar_offset_dir(hash, i, offset_correto);
        }
    }
    fflush(hash->arquivo_cabecalho);

    // Reinserção após divisão.
    for(int i = 0; i < qtd_temp; i++) inserirHash(hash, temporarios[i].chave, temporarios[i].dados);

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

// Função para deixar o campo de dados legíveis e com largura fixa.
static void dados_com_underline(const char *src, wchar_t *dst, int largura) {
    int i;
    for (i = 0; src[i] != '\0' && i < largura; i++)
        dst[i] = (src[i] == ' ') ? L'_' : (wchar_t)src[i];
    for (; i < largura; i++)
        dst[i] = L'_';
    dst[largura] = L'\0';
}

void dumpHash(HashExtensivel* hash, const char* nome_arq){
    if(!hash || !nome_arq) return;

    char nome_hfd[256];
    snprintf(nome_hfd, sizeof(nome_hfd), "%s.hfd", nome_arq);
    FILE* f = fopen(nome_hfd, "wb");
    if(!f) return;

    unsigned char bom[2] = {0xFF, 0xFE};
    fwrite(bom, 1, 2, f);
    fclose(f);

    f = fopen(nome_hfd, "ab");
    if(!f) return;

    fwide(f, 1);
    
    int total = 1 << hash->profundidade_global;
    int size_record = (int)sizeof(Registro);
    int size_block = (int)sizeof(Bucket);

    // Buckets únicos para numBuckets
    int num_buckets = 0;
    for(int i = 0; i < total; i++){
        int off_i = ler_offset_dir(hash, i);
        int unico = 1;
        for(int j = 0; j < i; j++){
            if(ler_offset_dir(hash, j) == off_i){
                unico = 0;
                break;
            }
        }
        if(unico) num_buckets++;
    }


    fwprintf(f, L"= DUMP =\n");
    fwprintf(f, L"* Dump cabecalho\n");
    fwprintf(f, L"numBuckets\t%d\n", num_buckets);
    fwprintf(f, L"sizeRecord\t%d\n", size_record);
    fwprintf(f, L"sizeBlock\t%d\n", size_block);
    fwprintf(f, L"offsetKey\t%d\n", 0); // Chave 1º campo do registro. ** Dar uma olhada dps no post do class.
    fwprintf(f, L"sizeKey\t%d\n", MAX_CHAVE);
    fwprintf(f, L"offsetTable\t%d\n", (int)HEADER_SIZE);
    fwprintf(f, L"offsetBuckets\t%d\n", 0);  // 1º bucket -> offset 0
    fwprintf(f, L"offsetOverflow\t%d\n", -1); // Sem área de overflow.

    
    // Diretório
    fwprintf(f, L"* Dump table\n");
    for (int i = 0; i < total; i++){
        fwprintf(f, L"[%d]\t%d\n", i, ler_offset_dir(hash, i));
    }

    // Buckets
    fwprintf(f, L"* Dump buckets\n");

    int bloco = 0;
    wchar_t dados_wide[MAX_DADOS + 1];

    for(int i = 0; i < total; i++){
        int offset = ler_offset_dir(hash, i);

        // ** Verificar se esse endereço já foi impresso
        int ja_visto = 0;
        for(int j = 0; j < i; j++){
            if(ler_offset_dir(hash, j) == offset){
                ja_visto = 1;
                break;
            }
        }
        if(ja_visto) continue;
        
        Bucket balde;
        ler_bucket(hash, offset, &balde);
        
        fwprintf(f, L"BLOCO: %d\n", bloco++);

        for(int k = 0; k < hash->cap_bucket; k++){
            dados_com_underline(balde.registros[k].dados, dados_wide, MAX_DADOS - 1);
            fwprintf(f, L"%d\t|\t%hs\t|\t%ls\t|\t%d\t|\n", 
                balde.registros[k].ativo, 
                balde.registros[k].chave, 
                dados_wide,
                balde.profundidade_local);
        }
    }

    fprintf(f, "= FIM DO DUMP =\n");
    fclose(f);
}