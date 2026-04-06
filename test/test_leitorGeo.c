#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "leitorGeo.h"
#include "hash_extensivel.h"
#include "quadra.h"

// Arquivos temporários.
static const char *GEO_TEMP = "temp_teste.geo";
static const char *HASH_PREFIX = "temp_hash_geo";

// Função simples para limpar arquivos antes do teste.
static void removerArquivos_temp(void){
    remove(GEO_TEMP);
    char path[256];
    snprintf(path, sizeof(path), "%s.hf", HASH_PREFIX); remove(path);
    snprintf(path, sizeof(path), "%s.hfc", HASH_PREFIX); remove(path);
    snprintf(path, sizeof(path), "%s.hfd", HASH_PREFIX); remove(path);
}

void setUp(void){ removerArquivos_temp(); }
void tearDown(void){ removerArquivos_temp(); }

// Padrão para auxiliar.
static HashExtensivel* abrirHashTeste(void){
    return abrirHash(HASH_PREFIX, 5);
}

void test_abrirGeo_comHashNulo_deveDarErro(void){
    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_ERRO, lerArqGeo(GEO_TEMP, NULL));
}

void test_abrirGeo_comCaminhoNulo_deveDarErro(void){
    HashExtensivel* h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_ERRO, lerArqGeo(NULL, h));
    fecharHash(h);
}

void test_abrirGeo_comCaminhoInvalido_deveDarErro(void){
    HashExtensivel* h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_ERRO, lerArqGeo("teste_invalido.geo", h));
    fecharHash(h);
}

// Inserção de Quadras.

void test_lerArqGeo_umaQuadra_deveInserirNoHash(void){
    // /.geo com um estilo e uma quadra
    FILE* f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "cq 1.0px gold MediumAquamarine\n");
    fprintf(f, "q b01.1 95.000000 95.000000 120.000000 80.000000\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_OK, lerArqGeo(GEO_TEMP, h));
    char buffer[QUADRA_SERIAL_MAX];

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.1", buffer, sizeof(buffer)));

    fecharHash(h);
}

void test_lerArqGeo_multiplasQuadras_deveInserirNoHash(void){
    FILE* f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "cq 1.0px gold MediumAquamarine\n");
    fprintf(f, "q b01.1 95.000000 95.000000 120.000000 80.000000\n");
    fprintf(f, "q b01.2 230.000000 95.000000 120.000000 80.000000\n");
    fprintf(f, "q b01.3 365.000000 95.000000 120.000000 80.000000\n");
    fprintf(f, "q b01.4 500.000000 95.000000 120.000000 80.000000\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_OK, lerArqGeo(GEO_TEMP, h));
    char buffer[QUADRA_SERIAL_MAX];

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.1", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.2", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.3", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.4", buffer, sizeof(buffer)));

    fecharHash(h);
}

void test_lerArqGeo_semComandoCQ_usarEstiloPadrao(void){
    FILE* f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "q b01.1 95.000000 95.000000 120.000000 80.000000\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_OK, lerArqGeo(GEO_TEMP, h));
    char buffer[QUADRA_SERIAL_MAX];

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.1", buffer, sizeof(buffer)));

    fecharHash(h);
}

// Estilo 

void test_lerArqGeo_estiloAplicadoQuadrasSeguinte(void){
    FILE *f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "cq 1.0px gold MediumAquamarine\n");
    fprintf(f, "q b01.1 95.000000 95.000000 120.000000 80.000000\n");
    fprintf(f, "q b01.2 230.000000 95.000000 120.000000 80.000000\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_OK, lerArqGeo(GEO_TEMP, h));

    char buffer[QUADRA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.1", buffer, sizeof(buffer)));
    Quadra *q1 = desserializarQuadra(buffer);
    TEST_ASSERT_NOT_NULL(q1);
    TEST_ASSERT_EQUAL_STRING("gold", getQuadraCFill(q1));
    TEST_ASSERT_EQUAL_STRING("MediumAquamarine", getQuadraCStrk(q1));
    destruirQuadra(q1);

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.2", buffer, sizeof(buffer)));
    Quadra *q2 = desserializarQuadra(buffer);
    TEST_ASSERT_NOT_NULL(q2);
    TEST_ASSERT_EQUAL_STRING("gold", getQuadraCFill(q2));
    TEST_ASSERT_EQUAL_STRING("MediumAquamarine", getQuadraCStrk(q2));
    destruirQuadra(q2);

    fecharHash(h);
}

void test_lerArqGeo_MudancaDeEstiloNaMetade(void){
    FILE *f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "cq 1.0px orange black\n");
    fprintf(f, "q b01.2 230.000000 95.000000 120.000000 80.000000\n");
    fprintf(f, "cq 1.0px black blue\n");
    fprintf(f, "q b01.6 770.000000 95.000000 120.000000 80.000000\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_OK, lerArqGeo(GEO_TEMP, h));

    char buffer[QUADRA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.2", buffer, sizeof(buffer)));
    Quadra *q1 = desserializarQuadra(buffer);
    TEST_ASSERT_NOT_NULL(q1);
    TEST_ASSERT_EQUAL_STRING("orange", getQuadraCFill(q1));
    destruirQuadra(q1);

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "b01.6", buffer, sizeof(buffer)));
    Quadra *q2 = desserializarQuadra(buffer);
    TEST_ASSERT_NOT_NULL(q2);
    TEST_ASSERT_EQUAL_STRING("black", getQuadraCFill(q2));
    destruirQuadra(q2);

    fecharHash(h);
}

// Outros casos.

void test_lerArqGeo_ArquivoVazio_deveRetornarOk(void){
    FILE *f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_OK, lerArqGeo(GEO_TEMP, h));

    fecharHash(h);
}

void test_lerArqGeo_linhasEmBrancoEComentarios_deveIgnorar(void){
    FILE *f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "\n");
    fprintf(f, "# isto e um comentario\n");
    fprintf(f, "cq 2.0px orange black\n");
    fprintf(f, "\n");
    fprintf(f, "q cep01 10.0 20.0 50.0 30.0\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_OK, lerArqGeo(GEO_TEMP, h));

    char buffer[QUADRA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "cep01", buffer, sizeof(buffer)));

    fecharHash(h);
}

void test_lerArqGeo_CepAlfanumerico_deveInserir(void){
    FILE *f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "cq 1.0 green white\n");
    fprintf(f, "q cep15A 10.0 20.0 50.0 30.0\n");
    fclose(f);
 
    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);
 
    TEST_ASSERT_EQUAL_INT(LEITOR_GEO_OK, lerArqGeo(GEO_TEMP, h));
 
    char buffer[QUADRA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(h, "cep15A", buffer, sizeof(buffer)));
 
    fecharHash(h);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_abrirGeo_comHashNulo_deveDarErro);
    RUN_TEST(test_abrirGeo_comCaminhoNulo_deveDarErro);
    RUN_TEST(test_abrirGeo_comCaminhoInvalido_deveDarErro);
    RUN_TEST(test_lerArqGeo_umaQuadra_deveInserirNoHash);
    RUN_TEST(test_lerArqGeo_multiplasQuadras_deveInserirNoHash);
    RUN_TEST(test_lerArqGeo_semComandoCQ_usarEstiloPadrao);
    RUN_TEST(test_lerArqGeo_estiloAplicadoQuadrasSeguinte);
    RUN_TEST(test_lerArqGeo_MudancaDeEstiloNaMetade);
    RUN_TEST(test_lerArqGeo_ArquivoVazio_deveRetornarOk);
    RUN_TEST(test_lerArqGeo_linhasEmBrancoEComentarios_deveIgnorar);
    RUN_TEST(test_lerArqGeo_CepAlfanumerico_deveInserir);

    return UNITY_END();
}