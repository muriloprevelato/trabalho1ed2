#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "pessoa.h"

// Sem arquivos em disco então setUp e tearDown sem estado global neste módulo.
void setUp(void)    {}
void tearDown(void) {}

// Só para não ficar repetindo parâmetros em cada teste!
static Pessoa* pessoa_padrao(void) {
    return criarPessoa("123.456.789-00", "Murilo", "Prevelato", 'M', "26/04/2007");
}


// Inicialização e destruição

void test_criar_DeveRetornarPonteiroValido(void) {
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);
    destruirPessoa(p);
}

void test_criar_ComCpfNulo_DeveRetornarNull(void) {
    Pessoa *p = criarPessoa(NULL, "Joao", "Silva", 'M', "08/05/1990");
    TEST_ASSERT_NULL(p);
}

void test_criar_ComNomeNulo_DeveRetornarNull(void) {
    Pessoa *p = criarPessoa("123.456.789-00", NULL, "Silva", 'M', "08/05/1990");
    TEST_ASSERT_NULL(p);
}

void test_criar_ComSobrenomeNulo_DeveRetornarNull(void) {
    Pessoa *p = criarPessoa("123.456.789-00", "Joao", NULL, 'M', "08/05/1990");
    TEST_ASSERT_NULL(p);
}

void test_criar_ComNascNulo_DeveRetornarNull(void) {
    Pessoa *p = criarPessoa("123.456.789-00", "Joao", "Silva", 'M', NULL);
    TEST_ASSERT_NULL(p);
}

void test_criar_ComSexoInvalido_DeveRetornarNull(void) {
    Pessoa *p = criarPessoa("123.456.789-00", "Joao", "Silva", 'X', "08/05/1990");
    TEST_ASSERT_NULL(p);
}

void test_criar_ComNascInvalido_DeveRetornarNull(void) {
    // Formato inválido — não segue dd/mm/aaaa 
    Pessoa *p = criarPessoa("123.456.789-00", "Joao", "Silva", 'M', "1990-05-08");
    TEST_ASSERT_NULL(p);
}

void test_destruir_ComNulo_NaoCrasha(void) {
    destruirPessoa(NULL);
}

// Getters

void test_getters_DeveRetornarValoresCorretos(void) {
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);

    TEST_ASSERT_EQUAL_STRING("123.456.789-00", getPessoaCpf(p));
    TEST_ASSERT_EQUAL_STRING("Murilo", getPessoaNome(p));
    TEST_ASSERT_EQUAL_STRING("Prevelato", getPessoaSobrenome(p));
    TEST_ASSERT_EQUAL_INT('M', getPessoaSexo(p));
    TEST_ASSERT_EQUAL_STRING("26/04/2007", getPessoaNasc(p));

    destruirPessoa(p);
}

void test_getters_ComNulo_DeveRetornarSentinelas(void) {
    TEST_ASSERT_NULL(getPessoaCpf(NULL));
    TEST_ASSERT_NULL(getPessoaNome(NULL));
    TEST_ASSERT_NULL(getPessoaSobrenome(NULL));
    TEST_ASSERT_EQUAL_INT('\0', getPessoaSexo(NULL));
    TEST_ASSERT_NULL(getPessoaNasc(NULL));
}

// Validações.

void test_sexoValido_Validos(void) {
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, sexoValido('M'));
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, sexoValido('F'));
}

void test_sexoValido_Invalidos(void) {
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, sexoValido('X'));
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, sexoValido('m')); // minúscula não vale 
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, sexoValido('\0'));
}

void test_nascValido_FormatoCorreto(void) {
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, nascValido("08/05/1990"));
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, nascValido("01/01/2000"));
}

void test_nascValido_FormatoIncorreto(void) {
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, nascValido("1990-05-08")); // ISO, não aceito 
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, nascValido("8/5/90"));     // sem zero à esquerda 
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, nascValido(NULL));
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, nascValido(""));
}

// Endereço.

void test_novaPessoa_NaoEhMorador(void) {
    // Toda pessoa recém-criada começa como sem-teto 
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, ehMorador(p));
    destruirPessoa(p);
}

void test_associarEndereco_TornaMorador(void) {
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);

    TEST_ASSERT_EQUAL_INT(PESSOA_OK, associarEndereco(p, "cep15", 'S', 45, "apto 2"));
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, ehMorador(p));

    destruirPessoa(p);
}

void test_gettersEndereco_DeveRetornarValoresCorretos(void) {
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);

    associarEndereco(p, "cep15", 'S', 45, "apto 2");

    TEST_ASSERT_EQUAL_STRING("cep15", getPessoaEnderecoCep(p));
    TEST_ASSERT_EQUAL_INT('S', getPessoaEnderecoFace(p));
    TEST_ASSERT_EQUAL_INT(45, getPessoaEnderecoNum(p));
    TEST_ASSERT_EQUAL_STRING("apto 2", getPessoaEnderecoCompl(p));

    destruirPessoa(p);
}

void test_removerEndereco_ViraSemTeto(void) {
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);

    associarEndereco(p, "cep15", 'S', 45, "apto 2");
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, ehMorador(p));

    TEST_ASSERT_EQUAL_INT(PESSOA_OK, removerEndereco(p));
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, ehMorador(p));

    destruirPessoa(p);
}

void test_gettersEndereco_SemTeto_DeveRetornarSentinelas(void) {
    // Getters de endereço em sem-teto devem retornar sentinelas, não crashar 
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);

    TEST_ASSERT_NULL(getPessoaEnderecoCep(p));
    TEST_ASSERT_EQUAL_INT('\0', getPessoaEnderecoFace(p));
    TEST_ASSERT_EQUAL_INT(-1, getPessoaEnderecoNum(p));
    TEST_ASSERT_NULL(getPessoaEnderecoCompl(p));

    destruirPessoa(p);
}

void test_associarEndereco_ComPessoaNula_DeveRetornarErro(void) {
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, associarEndereco(NULL, "cep15", 'S', 45, ""));
}

void test_removerEndereco_ComPessoaNula_DeveRetornarErro(void) {
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, removerEndereco(NULL));
}

void test_associarEndereco_PodeTrocarEndereco(void) {
    // Um morador pode mudar de endereço 
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);

    associarEndereco(p, "cep15", 'S', 45, "");
    associarEndereco(p, "cep20", 'N', 10, "bloco B");

    TEST_ASSERT_EQUAL_STRING("cep20", getPessoaEnderecoCep(p));
    TEST_ASSERT_EQUAL_INT('N', getPessoaEnderecoFace(p));
    TEST_ASSERT_EQUAL_INT(10, getPessoaEnderecoNum(p));
    TEST_ASSERT_EQUAL_STRING("bloco B", getPessoaEnderecoCompl(p));

    destruirPessoa(p);
}

void test_ehMorador_ComNulo_DeveRetornarErro(void) {
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, ehMorador(NULL));
}

// Serialização e desserialização.

void test_serializar_SemTeto_DeveProuzirStringNaoVazia(void) {
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);

    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, serializarPessoa(p, buffer, sizeof(buffer)));
    TEST_ASSERT_TRUE(strlen(buffer) > 0);

    destruirPessoa(p);
}

void test_serializar_Morador_DeveConterFlag1(void) {
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);
    associarEndereco(p, "cep15", 'S', 45, "apto 2");

    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, serializarPessoa(p, buffer, sizeof(buffer)));

    // O flag de morador (campo 6) deve ser '1'
    TEST_ASSERT_NOT_NULL(strstr(buffer, "|1|"));

    destruirPessoa(p);
}

void test_serializar_ComNulo_DeveRetornarErro(void) {
    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, serializarPessoa(NULL, buffer, sizeof(buffer)));
}

void test_serializar_ComBufferInsuficiente_DeveRetornarErro(void) {
    Pessoa *p = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(p);

    char buffer_pequeno[5];
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO,
    serializarPessoa(p, buffer_pequeno, sizeof(buffer_pequeno)));

    destruirPessoa(p);
}

void test_roundtrip_SemTeto_DeveReconstruirFiel(void) {
    Pessoa *original = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(original);

    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, serializarPessoa(original, buffer, sizeof(buffer)));

    Pessoa *reconstruida = desserializarPessoa(buffer);
    TEST_ASSERT_NOT_NULL(reconstruida);

    TEST_ASSERT_EQUAL_STRING(getPessoaCpf(original), getPessoaCpf(reconstruida));
    TEST_ASSERT_EQUAL_STRING(getPessoaNome(original), getPessoaNome(reconstruida));
    TEST_ASSERT_EQUAL_STRING(getPessoaSobrenome(original), getPessoaSobrenome(reconstruida));
    TEST_ASSERT_EQUAL_INT(getPessoaSexo(original), getPessoaSexo(reconstruida));
    TEST_ASSERT_EQUAL_STRING(getPessoaNasc(original), getPessoaNasc(reconstruida));
    TEST_ASSERT_EQUAL_INT(PESSOA_ERRO, ehMorador(reconstruida));

    destruirPessoa(original);
    destruirPessoa(reconstruida);
}

void test_roundtrip_Morador_DeveReconstruirComEndereco(void) {
    Pessoa *original = pessoa_padrao();
    TEST_ASSERT_NOT_NULL(original);
    associarEndereco(original, "cep15", 'S', 45, "apto 2");

    char buffer[PESSOA_SERIAL_MAX];
    TEST_ASSERT_EQUAL_INT(PESSOA_OK, serializarPessoa(original, buffer, sizeof(buffer)));

    Pessoa *reconstruida = desserializarPessoa(buffer);
    TEST_ASSERT_NOT_NULL(reconstruida);

    TEST_ASSERT_EQUAL_INT(PESSOA_OK, ehMorador(reconstruida));
    TEST_ASSERT_EQUAL_STRING(getPessoaEnderecoCep(original), getPessoaEnderecoCep(reconstruida));
    TEST_ASSERT_EQUAL_INT(getPessoaEnderecoFace(original), getPessoaEnderecoFace(reconstruida));
    TEST_ASSERT_EQUAL_INT(getPessoaEnderecoNum(original), getPessoaEnderecoNum(reconstruida));
    TEST_ASSERT_EQUAL_STRING(getPessoaEnderecoCompl(original), getPessoaEnderecoCompl(reconstruida));

    destruirPessoa(original);
    destruirPessoa(reconstruida);
}

void test_desserializar_ComNulo_DeveRetornarNull(void) {
    TEST_ASSERT_NULL(desserializarPessoa(NULL));
}

void test_desserializar_ComStringMalFormada_DeveRetornarNull(void) {
    TEST_ASSERT_NULL(desserializarPessoa("123.456.789-00|Joao"));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_criar_DeveRetornarPonteiroValido);
    RUN_TEST(test_criar_ComCpfNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComNomeNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComSobrenomeNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComNascNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComSexoInvalido_DeveRetornarNull);
    RUN_TEST(test_criar_ComNascInvalido_DeveRetornarNull);
    RUN_TEST(test_destruir_ComNulo_NaoCrasha);
    RUN_TEST(test_getters_DeveRetornarValoresCorretos);
    RUN_TEST(test_getters_ComNulo_DeveRetornarSentinelas);
    RUN_TEST(test_sexoValido_Validos);
    RUN_TEST(test_sexoValido_Invalidos);
    RUN_TEST(test_nascValido_FormatoCorreto);
    RUN_TEST(test_nascValido_FormatoIncorreto);
    RUN_TEST(test_novaPessoa_NaoEhMorador);
    RUN_TEST(test_associarEndereco_TornaMorador);
    RUN_TEST(test_gettersEndereco_DeveRetornarValoresCorretos);
    RUN_TEST(test_removerEndereco_ViraSemTeto);
    RUN_TEST(test_gettersEndereco_SemTeto_DeveRetornarSentinelas);
    RUN_TEST(test_associarEndereco_ComPessoaNula_DeveRetornarErro);
    RUN_TEST(test_removerEndereco_ComPessoaNula_DeveRetornarErro);
    RUN_TEST(test_associarEndereco_PodeTrocarEndereco);
    RUN_TEST(test_ehMorador_ComNulo_DeveRetornarErro);
    RUN_TEST(test_serializar_SemTeto_DeveProuzirStringNaoVazia);
    RUN_TEST(test_serializar_Morador_DeveConterFlag1);
    RUN_TEST(test_serializar_ComNulo_DeveRetornarErro);
    RUN_TEST(test_serializar_ComBufferInsuficiente_DeveRetornarErro);
    RUN_TEST(test_roundtrip_SemTeto_DeveReconstruirFiel);
    RUN_TEST(test_roundtrip_Morador_DeveReconstruirComEndereco);
    RUN_TEST(test_desserializar_ComNulo_DeveRetornarNull);
    RUN_TEST(test_desserializar_ComStringMalFormada_DeveRetornarNull);

    return UNITY_END();
}