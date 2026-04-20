#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leitorQry.h"
#include "hash_extensivel.h"
#include "svg.h"
#include "quadra.h"
#include "pessoa.h"

#define LINHA_MAX 512

// Contexto - rq
typedef struct{
    const char* cep_removido;
    HashExtensivel* hashPessoas;
    FILE* txt;
} CtxRq;

// Contexto - censo
typedef struct{
    int total;
    int moradores;
    int homens;
    int mulheres;
    int homens_moradores;
    int mulheres_moradores;
} CtxCenso;

// Contexto - pq
typedef struct{
    const char* cep;
    int n, s, l, o;
} CtxPq;

// Funções de callback

static void cb_rq(const char* chave, const char* dados, void* cmd){
    CtxRq *ctx = (CtxRq*) cmd;

    Pessoa* p = desserializarPessoa(dados);
    if(!p) return;

    if(ehMorador(p) == PESSOA_OK && strcmp(getPessoaEnderecoCep(p), ctx->cep_removido) == 0){
        fprintf(ctx->txt, " %s %s %s\n", getPessoaCpf(p), getPessoaNome(p), getPessoaSobrenome(p));
        removerEndereco(p);
        
        char buffer[PESSOA_SERIAL_MAX];
        if(serializarPessoa(p, buffer, sizeof(buffer)) == PESSOA_OK){
            removerHash(ctx->hashPessoas, chave);
            inserirHash(ctx->hashPessoas, chave, buffer);
        }
    }

    destruirPessoa(p);
}

static void cb_censo(const char* chave, const char* dados, void* cmd){
    (void)chave;
    CtxCenso* ctx = (CtxCenso*) cmd;

    Pessoa* p = desserializarPessoa(dados);
    if(!p) return;

    ctx->total++;
    char sexo = getPessoaSexo(p);
    if(sexo == 'M') ctx->homens++;
    else ctx->mulheres++;

    if(ehMorador(p) == PESSOA_OK){
        ctx->moradores++;
        if(sexo == 'M') ctx->homens_moradores++;
        else ctx->mulheres_moradores++;
    }

    destruirPessoa(p);
}

static void cb_pq(const char* chave, const char* dados, void* cmd){
    (void)chave;
    CtxPq* ctx = (CtxPq*) cmd;

    Pessoa* p = desserializarPessoa(dados);
    if(!p) return;

    if(ehMorador(p) == PESSOA_OK && strcmp(getPessoaEnderecoCep(p), ctx->cep) == 0){
        char face = getPessoaEnderecoFace(p);
        if(face == 'N') ctx->n++;
        else if(face == 'S') ctx->s++;
        else if(face == 'L') ctx->l++;
        else if(face == 'O') ctx->o++;
    }

    destruirPessoa(p);
}


// Funções auxiliares (cada comando).

static void cmd_rq(const char* linha, HashExtensivel* quadras, HashExtensivel* pessoas, ArqSvg* svg, FILE* txt){
    char cmd[6], cep[20]; // Deixei uma margem bem grande até no cep. Nos arquivos testes não passa de 5 caracteres.
    if(sscanf(linha, "%5s %19s", cmd, cep) != 2) return;

    // Buscar a quadra e desserializar ela.
    char buffer[QUADRA_SERIAL_MAX];
    if(buscarHash(quadras, cep, buffer, sizeof(buffer)) != HASH_OK) return;

    Quadra* q = desserializarQuadra(buffer);
    if(!q) return;

    double ax = getQuadraX(q);
    double ay = getQuadraY(q);
    destruirQuadra(q);

    fprintf(txt, "[*] rq %s\n", cep);

    CtxRq ctx = { cep, pessoas, txt };
    iterarHash(pessoas, cb_rq, &ctx);

    removerHash(quadras, cep);
    desenharMarcadorX(svg, ax, ay); // X na quadra a ser removida.
}

static void cmd_pq(const char* linha, HashExtensivel* quadras, HashExtensivel* pessoas, ArqSvg* svg, FILE* txt){
    char cmd[6], cep[20];
    if(sscanf(linha, "%5s %19s", cmd, cep) != 2) return;

    char buffer[QUADRA_SERIAL_MAX];
    if(buscarHash(quadras, cep, buffer, sizeof(buffer)) != HASH_OK) return;

    Quadra* q = desserializarQuadra(buffer);
    if(!q) return;

    double x = getQuadraX(q);
    double y = getQuadraY(q);
    double w = getQuadraW(q);
    double h = getQuadraH(q);
    destruirQuadra(q);

    CtxPq ctx = { cep, 0, 0, 0, 0};
    iterarHash(pessoas, cb_pq, &ctx);

    int total = ctx.n + ctx.s + ctx.l + ctx.o;
    char texto[32];

    snprintf(texto, sizeof(texto), "N:%d", ctx.n);
    desenharTexto(svg, x + w/2, y + h + 8, texto);
    snprintf(texto, sizeof(texto), "S:%d", ctx.s);
    desenharTexto(svg, x + w/2, y - 3, texto);
    snprintf(texto, sizeof(texto), "L:%d", ctx.l);
    desenharTexto(svg, x + 10, y + h/2, texto);
    snprintf(texto, sizeof(texto), "O:%d", ctx.o);
    desenharTexto(svg, x + w + 2, y + h/2, texto);

    snprintf(texto, sizeof(texto), "%d", total);
    desenharTextoCentro(svg, x + w/2, y + h/2, texto);

    fprintf(txt, "[*] pq %s\n", cep);
    fprintf(txt, "N:%d S:%d L:%d O:%d total:%d\n", ctx.n, ctx.s, ctx.l, ctx.o, total);
}

static void cmd_censo(HashExtensivel* pessoas, FILE* txt){
    CtxCenso ctx = { 0, 0, 0, 0, 0, 0 };
    iterarHash(pessoas, cb_censo, &ctx);

    int sem_teto = ctx.total - ctx.moradores;
    int homens_sem_teto = ctx.homens - ctx.homens_moradores;
    int mulheres_sem_teto = ctx.mulheres - ctx.mulheres_moradores;

    double prop_moradores = ctx.total > 0 ? (double)ctx.moradores / ctx.total * 100.0 : 0.0;
    double pct_homem = ctx.total > 0 ? (double)ctx.homens / ctx.total * 100.0 : 0.0;
    double pct_mulher = ctx.total > 0 ? (double)ctx.mulheres / ctx.total * 100.0 : 0.0;
    double pct_homem_morador = ctx.moradores > 0 ? (double)ctx.homens_moradores / ctx.moradores * 100.0 : 0.0;
    double pct_mulher_morador = ctx.moradores > 0 ? (double)ctx.mulheres_moradores / ctx.total * 100.0 : 0.0;
    double pct_homem_sem_teto = sem_teto > 0 ? (double)homens_sem_teto / sem_teto * 100.0 : 0.0;
    double pct_mulher_sem_teto = sem_teto > 0 ? (double)mulheres_sem_teto / sem_teto * 100.0 : 0.0;

    fprintf(txt, "[*] censo\n");
    fprintf(txt, "total habitantes: %d\n", ctx.total);
    fprintf(txt, "total moradores: %d\n", ctx.moradores);
    fprintf(txt, "proporcao moradores: %.2f\n", prop_moradores);
    fprintf(txt, "homens: %d\n", ctx.homens);
    fprintf(txt, "mulheres: %d\n", ctx.mulheres);
    fprintf(txt, "%% habitantes homens: %.2f%%\n", pct_homem);
    fprintf(txt, "%% habitantes mulheres: %.2f%%\n", pct_mulher);
    fprintf(txt, "%% moradores homens: %.2f%%\n", pct_homem_morador);
    fprintf(txt, "%% moradores mulheres: %.2f%%\n", pct_mulher_morador);
    fprintf(txt, "total sem-teto: %d\n", sem_teto);
    fprintf(txt, "%% sem-teto homens: %.2f%%\n", pct_homem_sem_teto);
    fprintf(txt, "%% sem-teto mulheres:%.2f%%\n", pct_mulher_sem_teto);
}

static void cmd_h(const char* linha, HashExtensivel* pessoas, FILE* txt){
    char cmd[6], cpf[16];
    if(sscanf(linha, "%5s %15s", cmd, cpf) != 2) return;

    fprintf(txt, "[*] h? %s\n", cpf);

    char buffer[PESSOA_SERIAL_MAX];
    if(buscarHash(pessoas, cpf, buffer, sizeof(buffer)) != HASH_OK){
        fprintf(txt, "habitante nao encontrado\n");
        return;
    }

    Pessoa* p = desserializarPessoa(buffer);
    if(!p) return;

    fprintf(txt, "CPF: %s\n", getPessoaCpf(p));
    fprintf(txt, "Nome: %s %s\n", getPessoaNome(p), getPessoaSobrenome(p));
    fprintf(txt, "Sexo: %c\n", getPessoaSexo(p));
    fprintf(txt, "Nascimento: %s\n", getPessoaNasc(p));

    if(ehMorador(p) == PESSOA_OK){
        fprintf(txt, "Endereco: %s/%c/%d/%s\n", 
            getPessoaEnderecoCep(p),
            getPessoaEnderecoFace(p),
            getPessoaEnderecoNum(p),
            getPessoaEnderecoCompl(p));
    }
    destruirPessoa(p);
}

static void cmd_nasc(const char* linha, HashExtensivel* pessoas){
    char cmd[6], cpf[16], nome[52], sobrenome[52], sexo[3], nasc[12];
    if (sscanf(linha, "%7s %15s %51s %51s %1s %11s", cmd, cpf, nome, sobrenome, sexo, nasc) != 6) return;

    Pessoa* p = criarPessoa(cpf, nome, sobrenome, sexo[0], nasc);
    if(!p) return;

    char buffer[PESSOA_SERIAL_MAX];
    if(serializarPessoa(p, buffer, sizeof(buffer)) == PESSOA_OK){
        inserirHash(pessoas, cpf, buffer);
    }
    destruirPessoa(p);
}

static void cmd_rip(const char* linha, HashExtensivel* quadras, HashExtensivel* pessoas, ArqSvg* svg, FILE* txt){
    char cmd[6], cpf[16];
    if(sscanf(linha, "%5s %15s", cmd, cpf) != 2) return;

    char buffer[PESSOA_SERIAL_MAX];
    if(buscarHash(pessoas, cpf, buffer, sizeof(buffer)) != HASH_OK) return;

    Pessoa* p = desserializarPessoa(buffer);
    if(!p) return;

    fprintf(txt, "[*] rip %s\n", cpf);
    fprintf(txt, "CPF: %s\n", getPessoaCpf(p));
    fprintf(txt, "Nome: %s %s\n", getPessoaNome(p), getPessoaSobrenome(p));

    if(ehMorador(p) == PESSOA_OK){
        fprintf(txt, "Endereco: %s/%c/%d/%s\n", 
            getPessoaEnderecoCep(p),
            getPessoaEnderecoFace(p),
            getPessoaEnderecoNum(p),
            getPessoaEnderecoCompl(p));
            
        char bufferQuadra[QUADRA_SERIAL_MAX];
        if(buscarHash(quadras, getPessoaEnderecoCep(p), bufferQuadra, sizeof(bufferQuadra)) == HASH_OK){
            Quadra* q = desserializarQuadra(bufferQuadra);
            if(q){
                desenharMarcadorCruz(svg, getQuadraX(q), getQuadraY(q));
                destruirQuadra(q);
            }
        } 
    }

    removerHash(pessoas, cpf);
    destruirPessoa(p);
}

static void cmd_mud(const char* linha, HashExtensivel* quadras, HashExtensivel* pessoas, ArqSvg* svg){
    char cmd[6], cpf[16], cep[20], face_str[4], num_str[16], compl[20];
    if (sscanf(linha, "%5s %15s %19s %1s %15s %19s", cmd, cpf, cep, face_str, num_str, compl) != 6) return;

    char buffer[PESSOA_SERIAL_MAX];
    if(buscarHash(pessoas, cpf, buffer, sizeof(buffer)) != HASH_OK) return;

    Pessoa* p = desserializarPessoa(buffer);
    if(!p) return;

    associarEndereco(p, cep, face_str[0], atoi(num_str), compl);

    char buffer_atualizarHash[PESSOA_SERIAL_MAX];
    if(serializarPessoa(p, buffer_atualizarHash, sizeof(buffer_atualizarHash)) == PESSOA_OK){
        removerHash(pessoas, cpf);
        inserirHash(pessoas, cpf, buffer_atualizarHash);
    }

    char bufferQuadra[QUADRA_SERIAL_MAX];
    if(buscarHash(quadras, cep, bufferQuadra, sizeof(bufferQuadra)) == HASH_OK){
        Quadra *q = desserializarQuadra(bufferQuadra);
        if(q){
            desenharMarcadorQuadrado(svg, getQuadraX(q), getQuadraY(q), cpf);
            destruirQuadra(q);
        }
    }

    destruirPessoa(p);
}

static void cmd_dspj(const char* linha, HashExtensivel* quadras, HashExtensivel* pessoas, ArqSvg* svg, FILE* txt){
    char cmd[6], cpf[16];
    if(sscanf(linha,"%5s %15s", cmd, cpf) != 2) return;

    char buffer[PESSOA_SERIAL_MAX];
    if(buscarHash(pessoas, cpf, buffer, sizeof(buffer)) != HASH_OK) return;

    Pessoa* p = desserializarPessoa(buffer);
    if(!p) return;

    if(ehMorador(p) != PESSOA_OK) { destruirPessoa(p); return ; } // Se não é morador só libera do hash e já sai.

    fprintf(txt, "[*] dspj %s\n", cpf);
    fprintf(txt, "CPF: %s\n", getPessoaCpf(p));
    fprintf(txt, "Nome: %s %s\n", getPessoaNome(p), getPessoaSobrenome(p));
    fprintf(txt, "Endereco: %s/%c/%d/%s\n", 
            getPessoaEnderecoCep(p),
            getPessoaEnderecoFace(p),
            getPessoaEnderecoNum(p),
            getPessoaEnderecoCompl(p));

    char bufferQuadra[QUADRA_SERIAL_MAX];
    if(buscarHash(quadras, getPessoaEnderecoCep(p), bufferQuadra, sizeof(bufferQuadra)) == HASH_OK){
        Quadra* q = desserializarQuadra(bufferQuadra);
        if(q){
            desenharMarcadorCirculo(svg, getQuadraX(q), getQuadraY(q));
            destruirQuadra(q);
        }
    }

    removerEndereco(p);
    char buffer_atualizarHash[PESSOA_SERIAL_MAX];
    if(serializarPessoa(p, buffer_atualizarHash, sizeof(buffer_atualizarHash)) == PESSOA_OK){
        removerHash(pessoas, cpf);
        inserirHash(pessoas, cpf, buffer_atualizarHash);
    }

    destruirPessoa(p);
}

// Função Principal

int processarQry(const char* caminhoQry, HashExtensivel* quadras, HashExtensivel* pessoas, ArqSvg* svg, FILE* txt){
    if(!caminhoQry || !quadras || !pessoas || !svg || !txt) return QRY_ERRO;

    FILE* f = fopen(caminhoQry, "r");
    if(!f) return QRY_ERRO;

    char linha[LINHA_MAX];

    while(fgets(linha, sizeof(linha), f)){
        linha[strcspn(linha,"\n")] = '\0';
        if(linha[0] == '\0' || linha[0] == '#') continue; // # tratar casos de comentários. Só para proteção msm, pouco provável que o Evandro coloque.

        if(strncmp(linha, "rq ", 3) == 0){
            cmd_rq(linha, quadras, pessoas, svg, txt);
        } else if(strncmp(linha, "pq ", 3) == 0){
            cmd_pq(linha, quadras, pessoas, svg, txt);
        } else if(strncmp(linha, "censo", 5) == 0){
            cmd_censo(pessoas, txt);
        } else if(strncmp(linha, "h? ", 3) == 0){
            cmd_h(linha, pessoas, txt);
        } else if(strncmp(linha, "nasc ", 5) == 0){
            cmd_nasc(linha, pessoas);
        } else if(strncmp(linha, "rip ", 4) == 0){
            cmd_rip(linha, quadras, pessoas, svg, txt);
        } else if(strncmp(linha, "mud ", 4) == 0){
            cmd_mud(linha, quadras, pessoas, svg);
        } else if(strncmp(linha, "dspj ", 5) == 0){
            cmd_dspj(linha, quadras, pessoas, svg, txt);
        }
    }

    fclose(f);
    return QRY_OK;
}