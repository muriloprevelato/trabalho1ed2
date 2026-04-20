#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "leitorQry.h"
#include "hash_extensivel.h"
#include "quadra.h"
#include "pessoa.h"
#include "svg.h"

static const char* QRY_TEMP =  "temp_teste.qry";
static const char* SVG_TEMP =  "temp_teste_qry.svg";
static const char* TXT_TEMP =  "temp_teste_qry.txt";
static const char* HASH_QUADRAS =  "temp_teste_quadras";
static const char* HASH_PESSOAS =  "temp_teste_pessoas";

static void removerArquivosTemp(void){
    remove(QRY_TEMP);
    remove(SVG_TEMP);
    remove(TXT_TEMP);

    char path[256];
    snprintf(path, sizeof(path), "%s.hf", HASH_QUADRAS); remove(path);
    snprintf(path, sizeof(path), "%s.hfc", HASH_QUADRAS); remove(path);
    snprintf(path, sizeof(path), "%s.hfd", HASH_QUADRAS); remove(path);
    snprintf(path, sizeof(path), "%s.hf", HASH_PESSOAS); remove(path);
    snprintf(path, sizeof(path), "%s.hfc", HASH_PESSOAS); remove(path);
    snprintf(path, sizeof(path), "%s.hfd", HASH_PESSOAS); remove(path);
}

void setUp(void) { removerArquivosTemp(); }
void tearDown(void) { removerArquivosTemp (); }

// Verifica se dado arquivo TXT contém uma string.
static int txtContem(const char* busca) {
    FILE* f = fopen(TXT_TEMP, "r");
    if(!f) return 0;
    char linha[512];
    while(fgets(linha, sizeof(linha), f)){
        if(strstr(linha, busca)) { fclose(f); return 1; }
    }
    fclose(f);
    return 0;
}

// Ambiente para cada teste.
static void montar_ambiente(HashExtensivel** hq, HashExtensivel** hp, ArqSvg** svg, FILE** txt){
    *hq = abrirHash(HASH_QUADRAS, 5);
    *hp = abrirHash(HASH_PESSOAS, 5);
    *svg = abrirSvg(SVG_TEMP);
    *txt = fopen(TXT_TEMP, "w");
}

static void desmontar_ambiente(HashExtensivel* hq, HashExtensivel* hp, ArqSvg* svg, FILE* txt) {
    fecharHash(hq);
    fecharHash(hp);
    fecharSvg(svg);
    if(txt) fclose(txt);
}

static void inserir_quadra(HashExtensivel* hq, const char* cep, double x, double y, double w, double h){
    Quadra* q = criarQuadra(cep, x, y, w, h, 1.0, "orange", "black");
    char buffer[QUADRA_SERIAL_MAX];
    serializarQuadra(q, buffer, sizeof(buffer));
    inserirHash(hq, cep, buffer);
    destruirQuadra(q);
}

static void inserir_pessoa(HashExtensivel* hp, const char* cpf, const char* nome, const char* sobrenome, char sexo, const char* nasc){
    Pessoa* p = criarPessoa(cpf, nome, sobrenome, sexo, nasc);
    char buffer[PESSOA_SERIAL_MAX];
    serializarPessoa(p, buffer, sizeof(buffer));
    inserirHash(hp, cpf, buffer);
    destruirPessoa(p);
}

static void inserir_morador(HashExtensivel* hp, const char* cpf, const char* nome, const char* sobrenome, char sexo, const char* nasc, const char* cep, char face, int num, const char* compl){
    Pessoa* p = criarPessoa(cpf, nome, sobrenome, sexo, nasc);
    associarEndereco(p, cep, face, num, compl);
    char buffer[PESSOA_SERIAL_MAX];
    serializarPessoa(p, buffer, sizeof(buffer));
    inserirHash(hp, cpf, buffer);
    destruirPessoa(p);
}

// Caminhos infelizes

void test_lerQryComArgsNull_deveRetornarErro(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    TEST_ASSERT_EQUAL_INT(QRY_ERRO, processarQry(NULL, hq, hp, svg, txt));
    TEST_ASSERT_EQUAL_INT(QRY_ERRO, processarQry(QRY_TEMP, NULL, hp, svg, txt));
    TEST_ASSERT_EQUAL_INT(QRY_ERRO, processarQry(QRY_TEMP, hq, NULL, svg, txt));
    TEST_ASSERT_EQUAL_INT(QRY_ERRO, processarQry(QRY_TEMP, hq, hp, NULL, txt));
    TEST_ASSERT_EQUAL_INT(QRY_ERRO, processarQry(QRY_TEMP, hq, hp, svg, NULL));

    desmontar_ambiente(hq, hp, svg, txt);
}

void test_lerQryComArquivoInexistente_deveRetornarErro(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    TEST_ASSERT_EQUAL_INT(QRY_ERRO, processarQry("caminhoinvalido.qry", hq, hp, svg, txt));

    desmontar_ambiente(hq, hp, svg, txt);
}

void test_lerQryComArquivoVazio_deveRetornarOK(void){
    FILE *f = fopen(QRY_TEMP, "w"); fclose(f); // Abriu e fechou imediatamente. Arquivo vazio.
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    TEST_ASSERT_EQUAL_INT(QRY_OK, processarQry(QRY_TEMP, hq, hp, svg, txt));

    desmontar_ambiente(hq, hp, svg, txt);
}

// Comando rq

void test_rq_deveRemoverQuadra(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_quadra(hq, "cep1", 10.0, 20.0, 30.0, 40.0);

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "rq cep1\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);

    char buffer[QUADRA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(hq, "cep1", buffer, sizeof(buffer))); // Já tem que ter sido removida

    desmontar_ambiente(hq, hp, svg, txt);
}

void test_rq_deveTornarSemTeto(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_quadra(hq, "cep1", 10.0, 20.0, 30.0, 40.0);
    inserir_morador(hp, "123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007", "cep1", 'S', 10, "-");

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "rq cep1\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);

    // Pessoa fica no hash, mas sem endereço.
    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hp, "123.456.789-00", buffer, sizeof(buffer)));

    Pessoa* p = desserializarPessoa(buffer);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, ehMorador(p));
    destruirPessoa(p);

    desmontar_ambiente(hq, hp, svg, txt);
}

void test_rq_deveProduzirSaidaTxt(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_quadra(hq, "cep1", 10.0, 20.0, 30.0, 40.0);

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "rq cep1\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);
    desmontar_ambiente(hq, hp, svg, txt);

    TEST_ASSERT_TRUE(txtContem("[*] rq cep1"));
}

// Comando h

void test_h_deveExibirDadosDaPessoa(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_pessoa(hp, "123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007");

    FILE *f = fopen(QRY_TEMP, "w");
    fprintf(f, "h? 123.456.789-00\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);
    desmontar_ambiente(hq, hp, svg, txt);

    TEST_ASSERT_TRUE(txtContem("123.456.789-00"));
    TEST_ASSERT_TRUE(txtContem("Murilo"));
}

void test_h_pessoaInexistente_deveNaoEncontrar(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    FILE *f = fopen(QRY_TEMP, "w");
    fprintf(f, "h? 000.000.000-67\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);
    desmontar_ambiente(hq, hp, svg, txt);

    TEST_ASSERT_TRUE(txtContem("habitante nao encontrado"));
}

// Comando nasc

void test_nasc_deveInserirPessoa(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    FILE *f = fopen(QRY_TEMP, "w");
    fprintf(f, "nasc 000.000.067-00 Isadora Ribeiro F 30/04/2008\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);

    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hp, "000.000.067-00", buffer, sizeof(buffer)));

    Pessoa* p = desserializarPessoa(buffer);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_STRING("Isadora", getPessoaNome(p));
    destruirPessoa(p);

    desmontar_ambiente(hq, hp, svg, txt);
}

// Comando rip

void test_rip_deveRemoverPessoa(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_pessoa(hp, "123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007");

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "rip 123.456.789-00\n");
    fclose(f);
    
    processarQry(QRY_TEMP, hq, hp, svg, txt);

    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(hp, "123.456.789-00", buffer, sizeof(buffer)));

    desmontar_ambiente(hq, hp, svg, txt);
}

void test_rip_deveProduzirSaidaTxt(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_pessoa(hp, "123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007");

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "rip 123.456.789-00\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);
    desmontar_ambiente(hq, hp, svg, txt);

    TEST_ASSERT_TRUE(txtContem("[*] rip"));
    TEST_ASSERT_TRUE(txtContem("Murilo"));
}

// Comando mud

void test_mud_deveAtualizarEndereco(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_quadra(hq, "cep1", 10.0, 20.0, 30.0, 40.0);
    inserir_morador(hp, "123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007", "cep1", 'S', 10, "-");

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "mud 123.456.789-00 cep2 L 9 bloco2\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);

    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hp, "123.456.789-00", buffer, sizeof(buffer)));

    Pessoa* p = desserializarPessoa(buffer);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_STRING("cep2", getPessoaEnderecoCep(p));
    TEST_ASSERT_EQUAL('L', getPessoaEnderecoFace(p));
    destruirPessoa(p);

    desmontar_ambiente(hq, hp, svg, txt);
}

// Comando dspj

void test_dspj_deveTornarSemTeto(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_quadra(hq, "cep1", 10.0, 20.0, 30.0, 40.0);
    inserir_morador(hp, "123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007", "cep1", 'S', 10, "-");

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "dspj 123.456.789-00\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);

    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hp, "123.456.789-00", buffer, sizeof(buffer)));

    Pessoa* p = desserializarPessoa(buffer);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, ehMorador(p));
    destruirPessoa(p);

    desmontar_ambiente(hq, hp, svg, txt);
}

void test_dspj_deveProduzirSaidaTxt(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_quadra(hq, "cep1", 10.0, 20.0, 30.0, 40.0);
    inserir_morador(hp, "123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007", "cep1", 'S', 10, "-");

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "dspj 123.456.789-00\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);
    desmontar_ambiente(hq, hp, svg, txt);

    TEST_ASSERT_TRUE(txtContem("[*] dspj"));
    TEST_ASSERT_TRUE(txtContem("Murilo"));
}

void test_censo_deveTotalizarHabitantes(void){
    HashExtensivel *hq, *hp;
    ArqSvg* svg;
    FILE* txt;
    montar_ambiente(&hq, &hp, &svg, &txt);

    inserir_pessoa(hp, "123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007");
    inserir_pessoa(hp, "000.000.000-00", "Daniela", "Oliveira", 'F', "27/01/1982");
    inserir_morador(hp, "111.111.111-11", "Caio", "Idalgo", 'M', "30/03/2005", "cep1", 'N', 21, "-");

    FILE* f = fopen(QRY_TEMP, "w");
    fprintf(f, "censo\n");
    fclose(f);

    processarQry(QRY_TEMP, hq, hp, svg, txt);
    desmontar_ambiente(hq, hp, svg, txt);

    TEST_ASSERT_TRUE(txtContem("total habitantes: 3"));
    TEST_ASSERT_TRUE(txtContem("total moradores: 1"));
    TEST_ASSERT_TRUE(txtContem("total sem-teto: 2"));
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_lerQryComArgsNull_deveRetornarErro);
    RUN_TEST(test_lerQryComArquivoInexistente_deveRetornarErro);
    RUN_TEST(test_lerQryComArquivoVazio_deveRetornarOK);
    RUN_TEST(test_rq_deveRemoverQuadra);
    RUN_TEST(test_rq_deveTornarSemTeto);
    RUN_TEST(test_rq_deveProduzirSaidaTxt);
    RUN_TEST(test_h_deveExibirDadosDaPessoa);
    RUN_TEST(test_h_pessoaInexistente_deveNaoEncontrar);
    RUN_TEST(test_nasc_deveInserirPessoa);
    RUN_TEST(test_rip_deveRemoverPessoa);
    RUN_TEST(test_rip_deveProduzirSaidaTxt);
    RUN_TEST(test_mud_deveAtualizarEndereco);
    RUN_TEST(test_dspj_deveTornarSemTeto);
    RUN_TEST(test_dspj_deveProduzirSaidaTxt);
    RUN_TEST(test_censo_deveTotalizarHabitantes);

    return UNITY_END();
}