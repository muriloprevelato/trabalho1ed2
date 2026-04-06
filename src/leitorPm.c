#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leitorPm.h"
#include "pessoa.h"
#include "hash_extensivel.h"

#define LINHA_MAX 256

static int processar_linha_p(const char *linha, HashExtensivel *hash) {
    char cmd[4];
    char cpf[PESSOA_CPF_MAX];
    char nome[PESSOA_NOME_MAX];
    char sobrenome[PESSOA_NOME_MAX];
    char sexo_str[4];
    char nasc[PESSOA_NASC_MAX];

    if(sscanf(linha, "%3s %14s %49s %49s %1s %10s", cmd, cpf, nome, sobrenome, sexo_str, nasc) != 6)
        return 0;

    Pessoa *p = criarPessoa(cpf, nome, sobrenome, sexo_str[0], nasc);
    if(!p) return 0;

    char buffer[PESSOA_SERIAL_MAX];
    int ok = serializarPessoa(p, buffer, sizeof(buffer));
    destruirPessoa(p);

    if(ok != PESSOA_OK) return 0;

    return inserirHash(hash, cpf, buffer) == HASH_OK ? 1 : 0;
}

static int processar_linha_m(const char *linha, HashExtensivel *hash) {
    char cmd[4];
    char cpf[PESSOA_CPF_MAX];
    char cep[PESSOA_CEP_MAX];
    char face_str[4];
    char num_str[10];
    char complemento[PESSOA_COMPL_MAX];

    if (sscanf(linha, "%3s %14s %19s %1s %15s %19s", cmd, cpf, cep, face_str, num_str, complemento) != 6)
    return 0;

    char buffer[PESSOA_SERIAL_MAX];
    if(buscarHash(hash, cpf, buffer, sizeof(buffer)) != HASH_OK)
        return 1;

    Pessoa *p = desserializarPessoa(buffer);
    if(!p) return 0;

    int num = atoi(num_str);
    associarEndereco(p, cep, face_str[0], num, complemento);

    char buffer_atualizado[PESSOA_SERIAL_MAX];
    int ok = serializarPessoa(p, buffer_atualizado, sizeof(buffer_atualizado));
    destruirPessoa(p);

    if(ok != PESSOA_OK) return 0;

    removerHash(hash, cpf);
    return inserirHash(hash, cpf, buffer_atualizado) == HASH_OK ? 1 : 0;
}

int lerArqPm(const char *caminhoArqPm, HashExtensivel *HashPessoas){
    if(!caminhoArqPm || !HashPessoas) return LEITOR_PM_ERRO;

    FILE *f = fopen(caminhoArqPm, "r");
    if(!f) return LEITOR_PM_ERRO;

    char linha[LINHA_MAX];

    while(fgets(linha, sizeof(linha), f)){
        linha[strcspn(linha, "\n")] = '\0';

        if(linha[0] == '\0' || linha[0] == '#') continue;

        if(strncmp(linha, "p ", 2) == 0){
            processar_linha_p(linha, HashPessoas);
        } else if(strncmp(linha, "m ", 2) == 0){
            processar_linha_m(linha, HashPessoas);
        }
    }
    
    fclose(f);
    return LEITOR_PM_OK;
}