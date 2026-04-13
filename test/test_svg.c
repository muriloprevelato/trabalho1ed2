#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "quadra.h"
#include "svg.h"

static const char* SVG_TEMP = "temp_teste.svg";

// Função simples para limpar arquivos temporário.
static void removerArquivosTemp(void){
    remove(SVG_TEMP);
}

void setUp(void) { removerArquivosTemp(); }
void tearDown(void) { removerArquivosTemp(); }

// Lê o conteúdo do arquivo SVG gerado para verificação
static int arquivoContemString(const char *caminho, const char *busca) {
    FILE *f = fopen(caminho, "r");
    if (!f) return 0;
 
    char linha[512];
    while (fgets(linha, sizeof(linha), f)) {
        if (strstr(linha, busca)) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

// Só para não ficar repetindo parâmetros em cada teste!
static Quadra* quadra_padrao(void){
    return criarQuadra("b01", 10.0, 20.0, 50.0, 30.0, 2.0, "orange", "black");
}

// Inicialização e fechamento.

void test_abrirSvg_deveRetornarPonteiroValido(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    fecharSvg(svg);
}

void test_abrirSvg_deveGerarArquivoNoDisco(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    fecharSvg(svg);

    FILE* f = fopen(SVG_TEMP, "r");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Arquivo .svg nao foi criado em disco");
    if(f) fclose(f);
}

void test_abrirSvg_comCaminhoNull_deveRetornarNull(void){
    ArqSvg *svg = abrirSvg(NULL);
    TEST_ASSERT_NULL(svg);
}

void test_fecharSvg_comNull_naoCrasha(void){
    fecharSvg(NULL);
}

void test_abrirSvg_deveConterCabecalhoSvg(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    fecharSvg(svg);

    // Arquivo deve ter a tag de abertura svg.
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "<svg"));
}

void test_fecharSvg_deveConterTagFechamento(void) {
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    fecharSvg(svg);

    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "</svg>"));
}

// Quadra

void test_desenharQuadra_deveRetornarOk(void) {
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    TEST_ASSERT_EQUAL_INT(SVG_OK, desenharQuadra(svg, q));

    destruirQuadra(q);
    fecharSvg(svg);
}

void test_desenharQuadra_deveGerarTag(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);

    Quadra *q = quadra_padrao();
    desenharQuadra(svg, q);
    destruirQuadra(q);
    fecharSvg(svg);

    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "<rect"));
}

void test_desenharQuadra_deveConterCoresCorretas(void) {
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
 
    Quadra *q = quadra_padrao(); /* cfill=orange, cstrk=black */
    desenharQuadra(svg, q);
    destruirQuadra(q);
    fecharSvg(svg);
 
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "orange"));
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "black"));
}

void test_desenharQuadra_comSvgNull_deveRetornarErro(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_EQUAL_INT(SVG_ERRO, desenharQuadra(NULL, q));
    destruirQuadra(q);
}

void test_desenharQuadra_comQuadraNull_deveRetornarErro(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);

    TEST_ASSERT_EQUAL_INT(SVG_ERRO, desenharQuadra(svg, NULL));

    fecharSvg(svg);
}

// Marcadores

void test_desenharMarcadorX_deveRetornarOK(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);

    TEST_ASSERT_EQUAL_INT(SVG_OK, desenharMarcadorX(svg, 60.0, 20.0)); // Medidas aleatórias

    fecharSvg(svg);
}

void test_desenharMarcadorX_deveGerarLinhasVermelhas(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    desenharMarcadorX(svg, 60.0, 20.0);
    fecharSvg(svg);

    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "<line"));
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "red"));
}

void test_desenharMarcadorCruz_deveRetornarOK(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);

    TEST_ASSERT_EQUAL_INT(SVG_OK, desenharMarcadorCruz(svg, 60.0, 20.0));
    
    fecharSvg(svg); 
}

void test_desenharMarcadorCruz_deveGerarLinhasVermelhas(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    desenharMarcadorCruz(svg, 60.0, 20.0);
    fecharSvg(svg);

    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "<line"));
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "red"));
}

void test_desenharMarcadorCirculo_deveRetornarOK(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);

    TEST_ASSERT_EQUAL_INT(SVG_OK, desenharMarcadorCirculo(svg, 60.0, 20.0));

    fecharSvg(svg);
}

void test_desenharMarcadorCirculo_deveGerarTag(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    desenharMarcadorCirculo(svg, 60.0, 20.0);
    fecharSvg(svg);

    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "<circle"));
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "black"));
}

void test_desenharMarcadorQuadrado_deveRetornarOK(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);

    TEST_ASSERT_EQUAL_INT(SVG_OK, desenharMarcadorQuadrado(svg, 60.0, 20.0, "123.456.789-00"));

    fecharSvg(svg);
}

void test_desenharMarcadorQuadrado_deveConterCPF(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    desenharMarcadorQuadrado(svg, 60.0, 20.0, "123.456.789-00");
    fecharSvg(svg);

    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "123.456.789-00"));
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "red"));
}

// Texto

void test_desenharTextoCentro_deveRetornarOK(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);

    TEST_ASSERT_EQUAL_INT(SVG_OK, desenharTextoCentro(svg, 60.0, 60.0, "Murilo"));

    fecharSvg(svg);
}

void test_desenharTextoCentro_deveConterTexto(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    desenharTextoCentro(svg, 60.0, 60.0, "Murilo");
    fecharSvg(svg);

    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "Murilo"));
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "<text"));
}

void test_desenharTexto_deveRetornarOK(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);

    TEST_ASSERT_EQUAL_INT(SVG_OK, desenharTexto(svg, 60.0, 60.0, "Murilo"));

    fecharSvg(svg);
}

void test_desenharTexto_deveConterTexto(void){
    ArqSvg *svg = abrirSvg(SVG_TEMP);
    TEST_ASSERT_NOT_NULL(svg);
    desenharTexto(svg, 60.0, 60.0, "Murilo");
    fecharSvg(svg);

    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "Murilo"));
    TEST_ASSERT_TRUE(arquivoContemString(SVG_TEMP, "<text"));
}

// Caminhos infelizes

void test_marcadores_comSvgNull_deveRetornarErro(void){
    TEST_ASSERT_EQUAL_INT(SVG_ERRO, desenharMarcadorX(NULL, 0.0, 0.0));
    TEST_ASSERT_EQUAL_INT(SVG_ERRO, desenharMarcadorCruz(NULL, 0.0, 0.0));
    TEST_ASSERT_EQUAL_INT(SVG_ERRO, desenharMarcadorCirculo(NULL, 0.0, 0.0));
    TEST_ASSERT_EQUAL_INT(SVG_ERRO, desenharMarcadorQuadrado(NULL, 0.0, 0.0, "cpf"));
    TEST_ASSERT_EQUAL_INT(SVG_ERRO, desenharTextoCentro(NULL, 0.0, 0.0, "txt"));
    TEST_ASSERT_EQUAL_INT(SVG_ERRO, desenharTexto(NULL, 0.0, 0.0, "txt"));
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_abrirSvg_deveRetornarPonteiroValido);
    RUN_TEST(test_abrirSvg_deveGerarArquivoNoDisco);
    RUN_TEST(test_abrirSvg_comCaminhoNull_deveRetornarNull);
    RUN_TEST(test_abrirSvg_deveConterCabecalhoSvg);
    RUN_TEST(test_fecharSvg_comNull_naoCrasha);
    RUN_TEST(test_fecharSvg_deveConterTagFechamento);
    RUN_TEST(test_desenharQuadra_deveRetornarOk);
    RUN_TEST(test_desenharQuadra_deveGerarTag);
    RUN_TEST(test_desenharQuadra_deveConterCoresCorretas);
    RUN_TEST(test_desenharQuadra_comSvgNull_deveRetornarErro);
    RUN_TEST(test_desenharQuadra_comQuadraNull_deveRetornarErro);
    RUN_TEST(test_desenharMarcadorX_deveRetornarOK);
    RUN_TEST(test_desenharMarcadorX_deveGerarLinhasVermelhas);
    RUN_TEST(test_desenharMarcadorCruz_deveRetornarOK);
    RUN_TEST(test_desenharMarcadorCruz_deveGerarLinhasVermelhas);
    RUN_TEST(test_desenharMarcadorCirculo_deveRetornarOK);
    RUN_TEST(test_desenharMarcadorCirculo_deveGerarTag);
    RUN_TEST(test_desenharMarcadorQuadrado_deveRetornarOK);
    RUN_TEST(test_desenharMarcadorQuadrado_deveConterCPF);
    RUN_TEST(test_desenharTextoCentro_deveRetornarOK);
    RUN_TEST(test_desenharTextoCentro_deveConterTexto);
    RUN_TEST(test_desenharTexto_deveRetornarOK);
    RUN_TEST(test_desenharTexto_deveConterTexto);
    RUN_TEST(test_marcadores_comSvgNull_deveRetornarErro);

    return UNITY_END();
}