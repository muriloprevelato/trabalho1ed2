#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quadra.h"
#include "unity.h"

// Sem arquivos em disco então setUp e tearDown sem estado global neste módulo.
void setUp(void){}
void tearDown(void){}

// Só para não ficar repetindo parâmetros em cada teste!
static Quadra* quadra_padrao(void){
    return criarQuadra("cep15", 10.0, 20.0, 50.0, 30.0, 2.0, "orange", "black");
}

// Inicialização e destruição

void test_criarQuadra_DeveRetornarPonteiroValido(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);
    destruirQuadra(q);
}

void test_criar_ComCEPNulo_DeveRetornarNull(void){
    Quadra *q = criarQuadra(NULL, 10.0, 20.0, 50.0, 30.0, 2.0, "orange", "black");
    TEST_ASSERT_NULL(q);
}

void test_criar_ComCFillNulo_DeveRetornarNull(void){
    Quadra *q = criarQuadra("cep15", 10.0, 20.0, 50.0, 30.0, 2.0, NULL, "black");
    TEST_ASSERT_NULL(q);
}

void test_criar_ComCStrkNulo_DeveRetornarNull(void){
    Quadra *q = criarQuadra("cep15", 10.0, 20.0, 50.0, 30.0, 2.0, "orange", NULL);
    TEST_ASSERT_NULL(q);
}

void test_criar_ComDimensoeNegativas_DeveRetornarNull(void){
    Quadra *q = criarQuadra("cep01", 10.0, 20.0, -5.0, 30.0, 2.0, "orange", "black");
    TEST_ASSERT_NULL(q);
}

void test_destruirComParametroNull_naoQuebrar(void){
    destruirQuadra(NULL);
}

// Getters

void test_getters_RetornarValoresCorretos(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    TEST_ASSERT_EQUAL_STRING("cep15",  getQuadraCep(q));
    TEST_ASSERT_EQUAL_STRING("orange", getQuadraCFill(q));
    TEST_ASSERT_EQUAL_STRING("black",  getQuadraCStrk(q));
 
    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, getQuadraX(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 20.0, getQuadraY(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 50.0, getQuadraW(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 30.0, getQuadraH(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001,  2.0, getQuadraSw(q));
 
    destruirQuadra(q);
}

void test_getters_ParametrosNull_naoQuebrar(void){
    TEST_ASSERT_NULL(getQuadraCep(NULL));
    TEST_ASSERT_NULL(getQuadraCFill(NULL));
    TEST_ASSERT_NULL(getQuadraCStrk(NULL));

    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getQuadraX(NULL));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getQuadraY(NULL));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getQuadraW(NULL));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getQuadraH(NULL));
}

// Geometria

void test_contemPonto_PontoCentral_DeveEstarDentro(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, quadraContemPonto(q, 35.0, 35.0));
    destruirQuadra(q);
}

void test_contemPonto_PontoAncora_DeveEstarDentro(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, quadraContemPonto(q, 10.0, 20.0));
    destruirQuadra(q);
}

void test_contemPonto_CantoOposto_DeveEstarDentro(void) {
    /* Canto oposto (x+w, y+h) = (60, 50) também pertence à quadra */
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, quadraContemPonto(q, 60.0, 50.0));
    destruirQuadra(q);
}

void test_contemPonto_PontoFora_DeveRetornarErro(void) {
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, quadraContemPonto(q,  0.0,  0.0));  /* antes da âncora */
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, quadraContemPonto(q, 61.0, 51.0));  /* além do canto oposto */
    destruirQuadra(q);
}

void test_contemPonto_ComQuadraNula_DeveRetornarErro(void) {
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, quadraContemPonto(NULL, 10.0, 10.0));
}

void test_faceValida_FacesCorretas(void) {
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, faceValida('N'));
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, faceValida('S'));
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, faceValida('L'));
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, faceValida('O'));
}

void test_faceValida_FacesInvalidas(void) {
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, faceValida('X'));
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, faceValida('n')); /* minúscula não vale */
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, faceValida('\0'));
}

// Serialização e desserialização

void test_serializar_deveProduzirStringNaoVazia(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    char buffer[QUADRA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, serializarQuadra(q, buffer, sizeof(buffer)));
    TEST_ASSERT_TRUE(strlen(buffer) > 0);

    destruirQuadra(q);
}

void test_serializar_ComNulo_deveRetonarErro(void){
    char buffer[QUADRA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, serializarQuadra(NULL, buffer, sizeof(buffer)));
}

void test_serializar_comBufferInsuficiente_ERRO(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    char buffer_pequeno[5];
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, serializarQuadra(q, buffer_pequeno, sizeof(buffer_pequeno)));
    destruirQuadra(q);
}

void test_desserializar_deveReconstruirQuadra(void){
    Quadra *original = quadra_padrao();
    TEST_ASSERT_NOT_NULL(original);

    char buffer[QUADRA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, serializarQuadra(original, buffer, sizeof(buffer)));

    Quadra *reconstruida = desserializarQuadra(buffer);
    TEST_ASSERT_NOT_NULL(reconstruida);

    TEST_ASSERT_EQUAL_STRING(getQuadraCep(original),   getQuadraCep(reconstruida));
    TEST_ASSERT_EQUAL_STRING(getQuadraCFill(original), getQuadraCFill(reconstruida));
    TEST_ASSERT_EQUAL_STRING(getQuadraCStrk(original), getQuadraCStrk(reconstruida));
    TEST_ASSERT_FLOAT_WITHIN(0.001, getQuadraX(original),  getQuadraX(reconstruida));
    TEST_ASSERT_FLOAT_WITHIN(0.001, getQuadraY(original),  getQuadraY(reconstruida));
    TEST_ASSERT_FLOAT_WITHIN(0.001, getQuadraW(original),  getQuadraW(reconstruida));
    TEST_ASSERT_FLOAT_WITHIN(0.001, getQuadraH(original),  getQuadraH(reconstruida));
    TEST_ASSERT_FLOAT_WITHIN(0.001, getQuadraSw(original), getQuadraSw(reconstruida));

    destruirQuadra(original);
    destruirQuadra(reconstruida);
}

void test_desserializar_comStringNull_deveRetornarNull(void){
    TEST_ASSERT_NULL(desserializarQuadra(NULL));
}

void test_desserializar_comStringMalFormatada_NULL(void){
    TEST_ASSERT_NULL(desserializarQuadra("cep15|10.0|5.0"));
}

void test_XeY_ConsistentesAposDesserializacao(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    char buffer[QUADRA_SERIAL_MAX];
    serializarQuadra(q, buffer, sizeof(buffer));
    Quadra *q2 = desserializarQuadra(buffer);
    TEST_ASSERT_NOT_NULL(q2);

    TEST_ASSERT_FLOAT_WITHIN(0.001, getQuadraX(q), getQuadraX(q2));
    TEST_ASSERT_FLOAT_WITHIN(0.001, getQuadraY(q), getQuadraY(q2));
 
    destruirQuadra(q);
    destruirQuadra(q2);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_criarQuadra_DeveRetornarPonteiroValido);
    RUN_TEST(test_criar_ComCEPNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComCFillNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComCStrkNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComDimensoeNegativas_DeveRetornarNull);
    RUN_TEST(test_destruirComParametroNull_naoQuebrar);
    RUN_TEST(test_getters_RetornarValoresCorretos);
    RUN_TEST(test_getters_ParametrosNull_naoQuebrar);
    RUN_TEST(test_contemPonto_PontoCentral_DeveEstarDentro);
    RUN_TEST(test_contemPonto_PontoAncora_DeveEstarDentro);
    RUN_TEST(test_contemPonto_CantoOposto_DeveEstarDentro);
    RUN_TEST(test_contemPonto_PontoFora_DeveRetornarErro);
    RUN_TEST(test_contemPonto_ComQuadraNula_DeveRetornarErro);
    RUN_TEST(test_faceValida_FacesCorretas);
    RUN_TEST(test_faceValida_FacesInvalidas);
    RUN_TEST(test_serializar_deveProduzirStringNaoVazia);
    RUN_TEST(test_serializar_ComNulo_deveRetonarErro);
    RUN_TEST(test_serializar_comBufferInsuficiente_ERRO);
    RUN_TEST(test_desserializar_deveReconstruirQuadra);
    RUN_TEST(test_desserializar_comStringNull_deveRetornarNull);
    RUN_TEST(test_desserializar_comStringMalFormatada_NULL);
    RUN_TEST(test_XeY_ConsistentesAposDesserializacao);

    return UNITY_END();
}