#include "unity.h"
#include "leitorPm.h"
#include "hash_extensivel.h"
#include "pessoa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Arquivos temporários.
static const char *PM_TEMP    = "temp_teste.pm";
static const char *HASH_PREFIX = "temp_hash_pm";

// Função simples para limpar arquivos antes do teste.
static void removerArquivosTemp(void) {
    remove(PM_TEMP);
    char path[256];
    snprintf(path, sizeof(path), "%s.hf",  HASH_PREFIX); remove(path);
    snprintf(path, sizeof(path), "%s.hfc", HASH_PREFIX); remove(path);
    snprintf(path, sizeof(path), "%s.hfd", HASH_PREFIX); remove(path);
}

void setUp(void)    { removerArquivosTemp(); }
void tearDown(void) { removerArquivosTemp(); }

// Padrão para auxiliar.
static HashExtensivel* abrirHashTeste(void) {
    return abrirHash(HASH_PREFIX, 5);
}

// Helper
static Pessoa* buscarPessoa(HashExtensivel *h, const char *cpf) {
    char buffer[PESSOA_SERIAL_MAX];
    if (buscarHash(h, cpf, buffer, sizeof(buffer)) != HASH_OK)
        return NULL;
    return desserializarPessoa(buffer);
}

void test_lerArqPm_ComHashNulo_DeveRetornarErro(void) {
    TEST_ASSERT_EQUAL_INT(LEITOR_PM_ERRO, lerArqPm(PM_TEMP, NULL));
}

void test_lerArqPm_ComCaminhoNulo_DeveRetornarErro(void) {
    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_ERRO, lerArqPm(NULL, h));

    fecharHash(h);
}

void test_lerArqPm_ArquivoInexistente_DeveRetornarErro(void) {
    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_ERRO, lerArqPm("nao_existe.pm", h));

    fecharHash(h);
}

// Inserção de habitantes.
 
void test_lerArqPm_ComandoP_DeveInserirHabitante(void) {
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 123.456.789-00 Joao Silva M 08/05/1990\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    Pessoa *p = buscarPessoa(h, "123.456.789-00");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_STRING("Joao", getPessoaNome(p));
    TEST_ASSERT_EQUAL_STRING("Silva", getPessoaSobrenome(p));
    TEST_ASSERT_EQUAL_INT('M', getPessoaSexo(p));
    TEST_ASSERT_EQUAL_STRING("08/05/1990", getPessoaNasc(p));
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, ehMorador(p));  // recém-criado = sem-teto 

    destruirPessoa(p);
    fecharHash(h);
}

void test_lerArqPm_MultiplosHabitantes_DeveInserirTodos(void) {
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 111.111.111-11 Ana Souza F 01/01/1985\n");
    fprintf(f, "p 222.222.222-22 Carlos Lima M 15/06/1970\n");
    fprintf(f, "p 333.333.333-33 Bia Rocha F 22/11/2000\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    Pessoa *p1 = buscarPessoa(h, "111.111.111-11");
    Pessoa *p2 = buscarPessoa(h, "222.222.222-22");
    Pessoa *p3 = buscarPessoa(h, "333.333.333-33");

    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_NOT_NULL(p3);

    TEST_ASSERT_EQUAL_STRING("Ana", getPessoaNome(p1));
    TEST_ASSERT_EQUAL_STRING("Carlos", getPessoaNome(p2));
    TEST_ASSERT_EQUAL_STRING("Bia", getPessoaNome(p3));

    destruirPessoa(p1);
    destruirPessoa(p2);
    destruirPessoa(p3);
    fecharHash(h);
}

void test_lerArqPm_ArquivoVazio_DeveRetornarOk(void) {
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    fecharHash(h);
}

// Associando endereços.

void test_lerArqPm_ComandoM_DeveTornarMorador(void) {
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 123.456.789-00 Joao Silva M 08/05/1990\n");
    fprintf(f, "m 123.456.789-00 cep15 S 45 apto2\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    Pessoa *p = buscarPessoa(h, "123.456.789-00");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, ehMorador(p));
    TEST_ASSERT_EQUAL_STRING("cep15", getPessoaEnderecoCep(p));
    TEST_ASSERT_EQUAL_INT('S', getPessoaEnderecoFace(p));
    TEST_ASSERT_EQUAL_INT(45, getPessoaEnderecoNum(p));
    TEST_ASSERT_EQUAL_STRING("apto2", getPessoaEnderecoCompl(p));

    destruirPessoa(p);
    fecharHash(h);
}

void test_lerArqPm_ComandoM_SemComplemento_DeveFuncionar(void) {
    // Complemento vazio representado por "-".
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 123.456.789-00 Joao Silva M 08/05/1990\n");
    fprintf(f, "m 123.456.789-00 cep15 N 10 -\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    Pessoa *p = buscarPessoa(h, "123.456.789-00");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, ehMorador(p));
    TEST_ASSERT_EQUAL_INT('N', getPessoaEnderecoFace(p));
    TEST_ASSERT_EQUAL_INT(10, getPessoaEnderecoNum(p));

    destruirPessoa(p);
    fecharHash(h);
}

void test_lerArqPm_ComandoM_CPFInexistente_DeveIgnorar(void) {
    // 'm' sem 'p' correspondente deve ser ignorado sem crashar.
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "m 999.999.999-99 cep01 S 10 -\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    // Deve retornar OK — linha ignorada silenciosamente 
    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    // CPF não deve existir no hash 
    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(HASH_ERRO, buscarHash(h, "999.999.999-99", buffer, sizeof(buffer)));

    fecharHash(h);
}

void test_lerArqPm_MultiplosMoradores_DeveAssociarTodos(void) {
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 111.111.111-11 Ana Souza F 01/01/1985\n");
    fprintf(f, "p 222.222.222-22 Carlos Lima M 15/06/1970\n");
    fprintf(f, "m 111.111.111-11 cep10 N 5 -\n");
    fprintf(f, "m 222.222.222-22 cep20 S 100 bloco-B\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    Pessoa *p1 = buscarPessoa(h, "111.111.111-11");
    Pessoa *p2 = buscarPessoa(h, "222.222.222-22");

    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);

    TEST_ASSERT_EQUAL_INT(PESSOA_OK, ehMorador(p1));
    TEST_ASSERT_EQUAL_STRING("cep10", getPessoaEnderecoCep(p1));

    TEST_ASSERT_EQUAL_INT(PESSOA_OK, ehMorador(p2));
    TEST_ASSERT_EQUAL_STRING("cep20", getPessoaEnderecoCep(p2));

    destruirPessoa(p1);
    destruirPessoa(p2);
    fecharHash(h);
}

// Outros casos.

void test_lerArqPm_LinhasEmBrancoEComentarios_DeveIgnorar(void) {
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "\n");
    fprintf(f, "# comentario\n");
    fprintf(f, "p 123.456.789-00 Joao Silva M 08/05/1990\n");
    fprintf(f, "\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    Pessoa *p = buscarPessoa(h, "123.456.789-00");
    TEST_ASSERT_NOT_NULL(p);

    destruirPessoa(p);
    fecharHash(h);
}

void test_lerArqPm_DadosPessoaisPreservadosAposAssociarEndereco(void) {
    // Garante que nome, sexo, nasc não são corrompidos pelo comando 'm'
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 123.456.789-00 Joao Silva M 08/05/1990\n");
    fprintf(f, "m 123.456.789-00 cep15 S 45 apto2\n");
    fclose(f);

    HashExtensivel *h = abrirHashTeste();
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_INT(LEITOR_PM_OK, lerArqPm(PM_TEMP, h));

    Pessoa *p = buscarPessoa(h, "123.456.789-00");
    TEST_ASSERT_NOT_NULL(p);

    // Dados pessoais devem estar intactos após o 'm' 
    TEST_ASSERT_EQUAL_STRING("Joao", getPessoaNome(p));
    TEST_ASSERT_EQUAL_STRING("Silva", getPessoaSobrenome(p));
    TEST_ASSERT_EQUAL_INT('M', getPessoaSexo(p));
    TEST_ASSERT_EQUAL_STRING("08/05/1990", getPessoaNasc(p));

    destruirPessoa(p);
    fecharHash(h);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_lerArqPm_ComHashNulo_DeveRetornarErro);
    RUN_TEST(test_lerArqPm_ComCaminhoNulo_DeveRetornarErro);
    RUN_TEST(test_lerArqPm_ArquivoInexistente_DeveRetornarErro);
    RUN_TEST(test_lerArqPm_ComandoP_DeveInserirHabitante);
    RUN_TEST(test_lerArqPm_MultiplosHabitantes_DeveInserirTodos);
    RUN_TEST(test_lerArqPm_ArquivoVazio_DeveRetornarOk);
    RUN_TEST(test_lerArqPm_ComandoM_DeveTornarMorador);
    RUN_TEST(test_lerArqPm_ComandoM_SemComplemento_DeveFuncionar);
    RUN_TEST(test_lerArqPm_ComandoM_CPFInexistente_DeveIgnorar);
    RUN_TEST(test_lerArqPm_MultiplosMoradores_DeveAssociarTodos);
    RUN_TEST(test_lerArqPm_LinhasEmBrancoEComentarios_DeveIgnorar);
    RUN_TEST(test_lerArqPm_DadosPessoaisPreservadosAposAssociarEndereco);

    return UNITY_END();
}