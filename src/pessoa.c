#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pessoa.h"

struct Pessoa{
    char cpf[PESSOA_CPF_MAX];
    char nome[PESSOA_NOME_MAX];
    char sobrenome[PESSOA_NOME_MAX];
    char nasc[PESSOA_NASC_MAX];
    char sexo;
    int morador; // 1 - Morador / 0 - Sem teto.

    char cep[PESSOA_CEP_MAX];
    char face;
    int num;
    char complemento[PESSOA_COMPL_MAX];
};

int sexoValido(char sexo){
    return (sexo == 'M' || sexo == 'F') ? PESSOA_OK : PESSOA_ERRO;
}

int nascValido(const char *nasc){
    if(!nasc || strlen(nasc) != 10) return PESSOA_ERRO;
    // Verificando posições das barras.
    if(nasc[2] != '/' || nasc[5] != '/') return PESSOA_ERRO;
    // Verifica se os demais campos são dígitos.
    for(int i = 0; i < 10; i++){
        if(i == 2 || i == 5) continue;
        if(nasc[i] < '0' || nasc[i] > '9') return PESSOA_ERRO;
    }
    return PESSOA_OK;
}

Pessoa* criarPessoa(const char *cpf, const char *nome, const char *sobrenome, char sexo, const char *nasc){
    if(!cpf || !nome || !sobrenome || !nasc) return NULL;
    if(sexoValido(sexo) == PESSOA_ERRO) return NULL;
    if(nascValido(nasc) == PESSOA_ERRO) return NULL;

    Pessoa *p = (Pessoa*) malloc(sizeof(Pessoa));
    if(p == NULL){
        return NULL;
    }

    strncpy(p->cpf, cpf, PESSOA_CPF_MAX - 1); p->cpf[PESSOA_CPF_MAX - 1] = '\0';
    strncpy(p->nome, nome, PESSOA_NOME_MAX - 1); p->nome[PESSOA_NOME_MAX - 1] = '\0';
    strncpy(p->sobrenome, sobrenome, PESSOA_NOME_MAX - 1); p->sobrenome[PESSOA_NOME_MAX - 1] = '\0';
    strncpy(p->nasc, nasc, PESSOA_NASC_MAX - 1); p->nasc[PESSOA_NASC_MAX - 1] = '\0';
    p->sexo = sexo;
    p->morador = 0;

    /* Zera campos de endereço para evitar lixo de memória */
    p->cep[0]  = '\0';
    p->face    = '\0';
    p->num     = -1;
    p->complemento[0]= '\0';

    return p;
}

void destruirPessoa(Pessoa *p){
    // Como foram delimitados vetores estaticamente, só precisamos dar um unico free.
    free(p);
    // NULL é tolerado!
}

// Getters.

const char* getPessoaCpf(const Pessoa *p) { return p ? p->cpf : NULL; }
const char* getPessoaNome(const Pessoa *p) { return p ? p->nome : NULL; }
const char* getPessoaSobrenome(const Pessoa *p) { return p ? p->sobrenome : NULL; }
const char* getPessoaNasc(const Pessoa *p) { return p ? p->nasc : NULL; }
char getPessoaSexo(const Pessoa *p) { return p ? p->sexo : '\0'; }

// Endereço.

int ehMorador(const Pessoa *p){
    if(!p) return PESSOA_ERRO;
    return p->morador ? PESSOA_OK : PESSOA_ERRO;
}

int associarEndereco(Pessoa *p, const char *cep, char face, int num, const char *complemento){
    if (!p || !cep || !complemento) return PESSOA_ERRO;

    strncpy(p->cep, cep, PESSOA_CEP_MAX - 1); p->cep[PESSOA_CEP_MAX - 1] = '\0';
    strncpy(p->complemento, complemento, PESSOA_COMPL_MAX - 1); p->complemento[PESSOA_COMPL_MAX - 1] = '\0';
    p->face = face;
    p->num = num;
    p->morador = 1;
    
    return PESSOA_OK;
}

int removerEndereco(Pessoa *p){
    if(!p) return PESSOA_ERRO;

    p->morador = 0;
    p->cep[0] = '\0';
    p->face = '\0';
    p->num = -1;
    p->complemento[0]= '\0';
 
    return PESSOA_OK;
}

// Getters de endereço.

const char* getPessoaEnderecoCep(const Pessoa *p) { return (p && p->morador) ? p->cep   : NULL; }
char getPessoaEnderecoFace(const Pessoa *p) { return (p && p->morador) ? p->face  : '\0'; }
int getPessoaEnderecoNum(const Pessoa *p) { return (p && p->morador) ? p->num   : -1; }
const char* getPessoaEnderecoCompl(const Pessoa *p) { return (p && p->morador) ? p->complemento : NULL; }

// Serialização e desserialização.

int serializarPessoa(const Pessoa *p, char *buffer, int tam) {
    if (!p || !buffer || tam <= 0) return PESSOA_ERRO;
 
    int escrito;
 
    if (p->morador) {
        escrito = snprintf(buffer, (size_t)tam, "%s|%s|%s|%c|%s|1|%s|%c|%d|%s", p->cpf, p->nome, p->sobrenome, p->sexo, p->nasc, p->cep, p->face, p->num, p->complemento);
    } else {
        escrito = snprintf(buffer, (size_t)tam, "%s|%s|%s|%c|%s|0", p->cpf, p->nome, p->sobrenome, p->sexo, p->nasc);
    }
 
    if (escrito < 0 || escrito >= tam) return PESSOA_ERRO;
    return PESSOA_OK;
}

Pessoa* desserializarPessoa(const char *buffer) {
    if (!buffer) return NULL;
 
    char copia[PESSOA_SERIAL_MAX];
    strncpy(copia, buffer, PESSOA_SERIAL_MAX - 1);
    copia[PESSOA_SERIAL_MAX - 1] = '\0';
 
    char cpf[PESSOA_CPF_MAX];
    char nome[PESSOA_NOME_MAX];
    char sobrenome[PESSOA_NOME_MAX];
    char nasc[PESSOA_NASC_MAX];
    char sexo_str[4];
    char flag_str[4];
 
    // Extrai os 6 campos obrigatórios
    char *campos[6] = { cpf, nome, sobrenome, sexo_str, nasc, flag_str };
    int   tamanhos[6] = { PESSOA_CPF_MAX, PESSOA_NOME_MAX, PESSOA_NOME_MAX, 4, PESSOA_NASC_MAX, 4 };
 
    char *token = strtok(copia, "|");
    for (int i = 0; i < 6; i++) {
        if (!token) return NULL;
        strncpy(campos[i], token, (size_t)(tamanhos[i] - 1));
        campos[i][tamanhos[i] - 1] = '\0';
        token = strtok(NULL, "|");
    }
 
    char sexo = sexo_str[0];
    int flag = atoi(flag_str);
 
    Pessoa *p = criarPessoa(cpf, nome, sobrenome, sexo, nasc);
    if (!p) return NULL;
 
    // Se for morador, extrai os 4 campos de endereço 
    if (flag == 1) {
        char cep[PESSOA_CEP_MAX];
        char face_str[4];
        char num_str[16];
        char complemento[PESSOA_COMPL_MAX];
 
        char *end_campos[4] = { cep, face_str, num_str, complemento };
        int end_tam[4] = { PESSOA_CEP_MAX, 4, 16, PESSOA_COMPL_MAX };
 
        for (int i = 0; i < 4; i++) {
            if (!token) { destruirPessoa(p); return NULL; }
            strncpy(end_campos[i], token, (size_t)(end_tam[i] - 1));
            end_campos[i][end_tam[i] - 1] = '\0';
            token = strtok(NULL, "|");
        }
 
        associarEndereco(p, cep, face_str[0], atoi(num_str), complemento);
    }
 
    return p;
}