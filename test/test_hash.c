#include "unity.h"
#include "hash_extensivel.h"
#include <stdlib.h>
#include <stdio.h>

// Nome padrão adotado nos testes aqui.
static const char *TEST_PREFIX = "testeHash";

// Definindo uma capacidade pequena para os buckets para forçar splits
static const int CAP_BUCKET_TESTE = 2;

// Função simples para limpar arquivos antes do teste
static void removerTest_files(void){
    char path[256];
    snprintf(path, sizeof(path), "%s.hf", TEST_PREFIX); remove(path);
    snprintf(path, sizeof(path), "%s.hfc", TEST_PREFIX); remove(path);
    snprintf(path, sizeof(path), "%s.hfd", TEST_PREFIX); remove(path);
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
    HashExtensivel* hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);

    // verifica se o ponteiro não é nulo.
    TEST_ASSERT_NOT_NULL(hash);

    // lixo.
    fecharHash(hash);
}

void test_abrirHash_ComNomeNulo_DeveRetornarNull(void) {
    /* Caminho infeliz: nome de arquivo NULL */
    HashExtensivel *hash = abrirHash(NULL, CAP_BUCKET_TESTE);
    TEST_ASSERT_NULL(hash);
    /* Não chama fecharHash: hash é NULL*/
}

void test_inserirEBuscar_DeveEncontrarDadoInserido(void){
    HashExtensivel* hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    int resultado_insercao = inserirHash(hash, "cep01", "Dados do projeto");
    TEST_ASSERT_EQUAL_INT(HASH_OK, resultado_insercao); 

    char buffer[50];
    int resultado_busca = buscarHash(hash, "cep01", buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(HASH_OK, resultado_busca);
    TEST_ASSERT_EQUAL_STRING("Dados do projeto", buffer);

    fecharHash(hash);
}

void test_inserirChaveAlfanumerica_CepECpf(void){
    // Validando que suporta chave.
    HashExtensivel *hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep15", "Quadra 15"));
    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "123.456.789-00", "Ze mane"));

    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, "cep15", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Quadra 15", buffer);

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, "123.456.789-00", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Ze mane", buffer);
    fecharHash(hash);
}

void test_inserirChaveDuplicada_DeveRetornarErro(void){
    // Inserção de chaves iguais não deve ocorrer!
    HashExtensivel *hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep01", "escrita"));
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, inserirHash(hash, "cep01", "sobrescrita"));

    // dado original não pode ter sido corrompido.
    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, "cep01", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("escrita", buffer);

    fecharHash(hash);
}

void test_persistencia_de_dados_apos_reabertura(void){
    HashExtensivel* hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep01", "Dado persistencia 1"));
    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep02", "Dado persistencia 2"));
    fecharHash(hash);

    // Vamos reabrir ele para confirmar a gravação no disco
    HashExtensivel* hash2 = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash2);

    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash2, "cep01", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Dado persistencia 1", buffer);

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash2, "cep02", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Dado persistencia 2", buffer);
    fecharHash(hash2);
}

void test_inserirHash_ForcaSplit(void){
    // Com o capacidade limitada em 2, inserimos 3 valores e isso força um split.
    HashExtensivel *hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep10", "Registro 10"));
    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep20", "Registro 20"));

    int resultado_split = inserirHash(hash, "cep30", "Registro 30 - split forcado");
    TEST_ASSERT_EQUAL_INT(HASH_OK, resultado_split);

    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, "cep10", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Registro 10", buffer);

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, "cep20", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Registro 20", buffer);

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, "cep30", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("Registro 30 - split forcado", buffer);

    fecharHash(hash);
}

void test_split_pesistenciaAposExpansao(void){
    HashExtensivel *hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep01", "1"));
    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep02", "2"));
    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep03", "3"));
    fecharHash(hash);

    HashExtensivel *hash2 = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE); 
    TEST_ASSERT_NOT_NULL(hash2);

    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash2, "cep01", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("1", buffer);

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash2, "cep02", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("2", buffer);

    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash2, "cep03", buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("3", buffer);
    fecharHash(hash2);
}

void test_removerHash_deveImpedirBuscar(void){
    HashExtensivel *hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_OK, inserirHash(hash, "cep99", "Dado para remover"));

    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_OK, buscarHash(hash, "cep99", buffer, sizeof(buffer)));// acha
    TEST_ASSERT_EQUAL_INT(HASH_OK, removerHash(hash, "cep99")); // remove
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(hash, "cep99", buffer, sizeof(buffer))); // não acha mais

    fecharHash(hash); 
}

void test_removerHash_chaveInexistente(void){
    HashExtensivel *hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    TEST_ASSERT_EQUAL_INT(HASH_ERRO, removerHash(hash, "chave inexistente"));
    fecharHash(hash);
}

void test_dumpHash(void){
    HashExtensivel* hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    inserirHash(hash, "cep1", "a");
    inserirHash(hash, "cep2", "b");
    inserirHash(hash, "cep3", "c");

    dumpHash(hash, TEST_PREFIX);
    fecharHash(hash);

    char path_hfd[256];
    snprintf(path_hfd, sizeof(path_hfd), "%s.hfd", TEST_PREFIX);
    FILE* f = fopen(path_hfd, "r");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Arquivo .hfd não foi gerado por dumpHash");
    
    if(f){
        fclose(f);
    }
}

// Alguns caminhos infelizes separados.

void test_caminhoInfeliz_chaveInexistente(void){
    HashExtensivel *hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    // Caso negativo aqui: buscar um ID que não existe(tem que retornar Hash_Erro(0))
    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(hash, "CepLaranja", buffer, sizeof(buffer)));
    fecharHash(hash);
}

void test_inserirHash_ponteiroNulo(void){
    // Outro caso: passar NULL no lugar de um ponteiro válido para a função de inserir e derivadas.
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, inserirHash(NULL, "cep10", "Caminho Infeliz"));
}

void test_buscarHash_ponteiroNulo(void){
    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(NULL, "cep10", buffer, sizeof(buffer)));
}

void test_removerHash_ponteiroNulo(void){
    char buffer[50];
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, removerHash(NULL, "cep02"));
}

void test_buscarHash_bufferPequeno(void){
    // Outro: buffer de saída muito pequena!
    HashExtensivel *hash = abrirHash(TEST_PREFIX, CAP_BUCKET_TESTE);
    TEST_ASSERT_NOT_NULL(hash);

    inserirHash(hash, "cep50", "Texto Longo Demais para o Buffer");
    char buffer_pequeno[5]; 
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(hash, "cep50", buffer_pequeno, sizeof(buffer_pequeno)));
    
    fecharHash(hash);
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(test_abrirHash_DeveRetornarPonteiroValido);
    RUN_TEST(test_abrirHash_ComNomeNulo_DeveRetornarNull);
    RUN_TEST(test_inserirEBuscar_DeveEncontrarDadoInserido);
    RUN_TEST(test_inserirChaveAlfanumerica_CepECpf);
    RUN_TEST(test_inserirChaveDuplicada_DeveRetornarErro);
    RUN_TEST(test_persistencia_de_dados_apos_reabertura);
    RUN_TEST(test_inserirHash_ForcaSplit);
    RUN_TEST(test_split_pesistenciaAposExpansao);
    RUN_TEST(test_removerHash_deveImpedirBuscar);
    RUN_TEST(test_removerHash_chaveInexistente);
    RUN_TEST(test_dumpHash);
    RUN_TEST(test_caminhoInfeliz_chaveInexistente);
    RUN_TEST(test_inserirHash_ponteiroNulo);
    RUN_TEST(test_buscarHash_ponteiroNulo);
    RUN_TEST(test_removerHash_ponteiroNulo);
    RUN_TEST(test_buscarHash_bufferPequeno);

    return UNITY_END();
}