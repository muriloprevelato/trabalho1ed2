#include "unity.h"
#include "hash_extensivel.h"
#include <stdlib.h>

// Nome padrão adotado nos testes aqui.
static const char *TEST_PREFIX = "testeHash";

// Função simples para limpar arquivos antes do teste
static void removerTest_files(void){
    char path[256];
    snprintf(path, sizeof(path), "%s.dir", TEST_PREFIX);
    remove(path);
    snprintf(path, sizeof(path), "%s.dat", TEST_PREFIX);
    remove(path);
}

// Agora as funções setUp e tearDown exigidas pela unity que garantem a "limpeza" entre os testes.
void setUp(void){
    removerTest_files();
}

void tearDown(void){
    removerTest_files();
}

void test_abrirHash_DeveRetornarPonteiroValido(void){
    // tenta criar os arquivos do hash.
    HashExtensivel* hash = abrirHash(TEST_PREFIX);

    // verifica se o ponteiro não é nulo.
    TEST_ASSERT_NOT_NULL(hash);

    // lixo.
    fecharHash(hash);
}

void test_inserirHash_e_Buscas(void){
    HashExtensivel* hash = abrirHash(TEST_PREFIX);
    TEST_ASSERT_NOT_NULL(hash);

    int resultado_insercao = inserirHash(hash, 10, "Dados do projeto");
    TEST_ASSERT_EQUAL_INT(HASH_OK, resultado_insercao); 

    char buffer[50];
    int resultado_busca = buscarHash(hash, 10, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(HASH_OK, resultado_busca);
    TEST_ASSERT_EQUAL_STRING("Dados do projeto", buffer);

    fecharHash(hash);
}

void test_persistencia_de_dados_apos_reabertura(void){
    HashExtensivel* hash = abrirHash(TEST_PREFIX);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, 100, "Dado persistencia 1"));
    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, 101, "Dado persistencia 2"));
    fecharHash(hash);

    // Vamos reabrir ele para confirmar a gravação no disco
    HashExtensivel* hash2 = abrirHash(TEST_PREFIX);
    TEST_ASSERT_NOT_NULL(hash2);

    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash2, 100, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Dado persistencia 1", buffer);
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash2, 101, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Dado persistencia 2", buffer);
    fecharHash(hash2);
}

void test_inserirHash_ForcaSplit(void){
    HashExtensivel *hash = abrirHash(TEST_PREFIX);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, 10, "Registro 10"));
    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, 20, "Registro 20"));

    int resultado_split = inserirHash(hash, 30, "Registro 30 - split forcado");
    TEST_ASSERT_EQUAL_INT(HASH_OK, resultado_split);

    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, 10, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Registro 10", buffer);
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, 20, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Registro 20", buffer);
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, 30, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Registro 30 - split forcado", buffer);

    fecharHash(hash);
}

void test_removerHash(void){
    HashExtensivel *hash = abrirHash(TEST_PREFIX);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, 99, "Dado para remover"));

    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, 99, buffer, sizeof(buffer)));
    
    TEST_ASSERT_EQUAL_INT(HASH_OK, removerHash(hash, 99));

    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(hash, 99, buffer, sizeof(buffer)));

    fecharHash(hash); 
}

void test_dumpHash(void){
    HashExtensivel* hash = abrirHash(TEST_PREFIX);
    TEST_ASSERT_NOT_NULL(hash);
    inserirHash(hash, 1, "a");
    inserirHash(hash, 2, "b");

    dumpHash(hash);
    fecharHash(hash);
}

void test_caminhoInfeliz(void){
    HashExtensivel *hash = abrirHash(TEST_PREFIX);

    // Caso negativo aqui: buscar um ID que não existe(tem que retornar Hash_Erro(0))
    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(hash, 9999, buffer, sizeof(buffer)));

    // Outro caso: passar NULL no lugar de um ponteiro válido para a função de inserir.
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, inserirHash(NULL, 10, "Caminho Infeliz"));

    // Outro: buffer de saída muito pequena!
    inserirHash(hash, 50, "Texto Longo Demais para o Buffer");
    char buffer_pequeno[5]; 
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(hash, 50, buffer_pequeno, sizeof(buffer_pequeno)));

    fecharHash(hash);
}

int main()
{
    UNITY_BEGIN();
   
    RUN_TEST(test_abrirHash_DeveRetornarPonteiroValido);
    RUN_TEST(test_inserirHash_e_Buscas);
    RUN_TEST(test_persistencia_de_dados_apos_reabertura);
    RUN_TEST(test_inserirHash_ForcaSplit);
    RUN_TEST(test_removerHash);
    RUN_TEST(test_dumpHash);
    RUN_TEST(test_caminhoInfeliz);

    return UNITY_END();
}