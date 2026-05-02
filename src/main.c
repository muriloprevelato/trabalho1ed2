#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_extensivel.h"
#include "quadra.h"
#include "pessoa.h"
#include "svg.h"
#include "leitorGeo.h"
#include "leitorPm.h" 
#include "leitorQry.h"

// Constante padrão para o tamanho dos buckets -> 50.
#define CAP_BUCKET_QUADRAS 50
#define CAP_BUCKET_PESSOAS 50

//Caminhos
#define PATH_MAX_TAM 1024 //  Aumentei o tamanho porque deu warning nos testes.
#define NOME_MAX_TAM 256

// Para garantir que o caminho não termina com '/'
static void trataPath(char* dest, int tamMax, const char* src){
    int tam = strlen(src);
    if(tam >= tamMax) tam = tamMax - 1;
    strncpy(dest, src, (size_t)tam);
    dest[tam] = '\0';
    if(tam > 0 && dest[tam - 1] == '/')
        dest[tam - 1] = '\0';
}

// Função para extrair o nome de um caminho sem sua extensão.
static void nomeBase(char* dest, int tamMax, const char* caminho){
    const char* inicio = strrchr(caminho, '/'); // Encontra a ultima '/';
    inicio = inicio ? inicio + 1 : caminho;

    const char* ponto = strrchr(inicio, '.'); // Copia até o último '.';
    int tam = ponto ? (int)(ponto - inicio) : (int)strlen(inicio);
    if(tam >= tamMax) tam = tamMax - 1;

    strncpy(dest, inicio, (size_t) tam);
    dest[tam] = '\0';
}

// Função para extrair o nome de um caminho sem diretório.
static const char* nomeArquivo(const char* caminho){
    const char* p = strrchr(caminho, '/');
    return p ? p + 1 : caminho;
}

// Auxiliar para desenho.
static void desenhar_quadra_cb(const char* chave, const char* dados, void *ctx){
    (void)chave;    
    ArqSvg *svg = (ArqSvg*)ctx;
    Quadra* q = desserializarQuadra(dados);
    if(q){
        desenharQuadra(svg, q);
        destruirQuadra(q);
    }
}

int main(int argc, char const *argv[])
{
    char dir_entrada[PATH_MAX_TAM] = ".";
    char dir_saida[PATH_MAX_TAM] = "";
    char arq_geo[NOME_MAX_TAM] = "";
    char arq_pm[NOME_MAX_TAM] = "";
    char arq_qry[NOME_MAX_TAM] = "";

    int i = 1;
    while(i < argc){
        if(strcmp(argv[i], "-e") == 0 && i + 1 < argc){
            trataPath(dir_entrada, PATH_MAX_TAM, argv[++i]);
        } else if(strcmp(argv[i], "-f") == 0 && i + 1 < argc){
            strncpy(arq_geo, argv[++i], NOME_MAX_TAM - 1);
            arq_geo[NOME_MAX_TAM - 1] = '\0';
        } else if(strcmp(argv[i], "-o") == 0 && i + 1 < argc){
            trataPath(dir_saida, PATH_MAX_TAM, argv[++i]);
        } else if(strcmp(argv[i], "-q") == 0 && i + 1 < argc){
            strncpy(arq_qry, argv[++i], NOME_MAX_TAM - 1);
            arq_qry[NOME_MAX_TAM - 1] = '\0';
        } else if(strcmp(argv[i], "-pm") == 0 && i + 1 < argc){
            strncpy(arq_pm, argv[++i], NOME_MAX_TAM - 1);
            arq_pm[NOME_MAX_TAM - 1] = '\0';
        }

        i++;
    }

    // Validando parâmetros obrigatórios: arq.geo e caminho para saída
    if(arq_geo[0] == '\0'){
        fprintf(stderr, "Parametro -f (arq.geo) eh obrigatorio. Erro!\n");
        return 1;
    }
    if(dir_saida[0] == '\0'){
        fprintf(stderr, "Parametro -o (diretorio de saida) eh obrigatorio. Erro!\n");
        return 1;
    }

    // Montando caminhos.
    char caminho_geo[PATH_MAX_TAM];
    char caminho_pm[PATH_MAX_TAM];
    char caminho_qry[PATH_MAX_TAM];

    snprintf(caminho_geo, PATH_MAX_TAM, "%s/%s", dir_entrada, arq_geo); // Obrigatório.

    // Opcionais (verifica primeiro).
    if(arq_pm[0] != '\0')
        snprintf(caminho_pm, PATH_MAX_TAM, "%s/%s", dir_entrada, arq_pm);

    if(arq_qry[0] != '\0')
        snprintf(caminho_qry, PATH_MAX_TAM, "%s/%s", dir_entrada, arq_qry);

    // Preparação dos arquivos de saída.
    char base_geo[NOME_MAX_TAM];
    char base_qry[NOME_MAX_TAM];

    nomeBase(base_geo, NOME_MAX_TAM, arq_geo);

    // Svg inicial, apos o .geo.
    char caminho_svg_geo[PATH_MAX_TAM];
    snprintf(caminho_svg_geo, PATH_MAX_TAM, "%s/%s.svg", dir_saida, base_geo);

    // Saídas com a leitura do qry.
    char caminho_svg_qry[PATH_MAX_TAM];
    char caminho_txt_qry[PATH_MAX_TAM];

    if(arq_qry[0] != '\0'){
        nomeBase(base_qry, NOME_MAX_TAM, nomeArquivo(arq_qry));
        snprintf(caminho_svg_qry, PATH_MAX_TAM, "%s/%s-%s.svg", dir_saida, base_geo, base_qry);
        snprintf(caminho_txt_qry, PATH_MAX_TAM, "%s/%s-%s.txt", dir_saida, base_geo, base_qry);
    }

    // Caminhos dos hashfiles
    char hf_quadras[PATH_MAX_TAM];
    char hf_pessoas[PATH_MAX_TAM];
    snprintf(hf_quadras, PATH_MAX_TAM, "%s/quadras", dir_saida);
    snprintf(hf_pessoas, PATH_MAX_TAM, "%s/pessoas", dir_saida);

    HashExtensivel *hashQuadras = abrirHash(hf_quadras, CAP_BUCKET_QUADRAS);
    HashExtensivel *hashPessoas = abrirHash(hf_pessoas, CAP_BUCKET_PESSOAS);

    if(!hashQuadras || !hashPessoas){
        fprintf(stderr, "Erro: nao foi possivel abrir os hashfiles.\n");
        if(hashQuadras) fecharHash(hashQuadras);
        if(hashPessoas) fecharHash(hashPessoas);
        return 1;
    }

    // Leitura do arquivo .geo
    if(lerArqGeo(caminho_geo, hashQuadras) != LEITOR_GEO_OK){
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo .geo: %s\n", caminho_geo);
        fecharHash(hashQuadras);
        fecharHash(hashPessoas);
        return 1;
    }

    // Leitura do arquivo .pm (se fornecido)
    if(arq_pm[0] != '\0'){
        if(lerArqPm(caminho_pm, hashPessoas) != LEITOR_PM_OK){
            fprintf(stderr, "Erro: nao foi possivel abrir o arquivo .pm: %s\n", caminho_pm);
        }
    }

    // Svg inicial (após leitura do .geo)
    ArqSvg* svgGeo = abrirSvg(caminho_svg_geo);
    if(!svgGeo){
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo SVG inicial: %s\n", caminho_svg_geo);
        fecharHash(hashQuadras);
        fecharHash(hashPessoas);
        return 1;
    }

    // Itera sobre todas as quadras e as desenha
    iterarHash(hashQuadras, desenhar_quadra_cb, svgGeo);
    fecharSvg(svgGeo);

    // Leitura do arquivo .qry (se fornecido)
    if(arq_qry[0] != '\0'){
        ArqSvg* svgQry = abrirSvg(caminho_svg_qry);
        FILE* txtQry = fopen(caminho_txt_qry, "w");
        if(!svgQry || !txtQry){
            fprintf(stderr, "Erro: nao foi possivel criar arquivos de saida do .qry\n");
            if(svgQry) fecharSvg(svgQry);
            if(txtQry) fclose(txtQry);
            fecharHash(hashPessoas);
            fecharHash(hashQuadras);
            return 1;
        }

        //Redesenha as quadras no novo caminho.
        iterarHash(hashQuadras, desenhar_quadra_cb, svgQry);
        // Processa as alterações e novas impressões serão delimitadas atráves dos comandos.
        processarQry(caminho_qry, hashQuadras, hashPessoas, svgQry, txtQry);
        
        fecharSvg(svgQry);
        fclose(txtQry);
    }

    // DUMPS
    dumpHash(hashQuadras, hf_quadras);
    dumpHash(hashPessoas, hf_pessoas);

    fecharHash(hashQuadras);
    fecharHash(hashPessoas);

    return 0;
}