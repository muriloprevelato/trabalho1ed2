#ifndef PESSOA_H
#define PESSOA_H

/*
Interface do módulo Pessoa.

Todo habitante da cidade é uma pessoa (evidentemente), a qual é identificada por CPF.
Um subconjunto são os moradores. "Todo morador é habitante, mas nem todo habitante é morador".
Caso especial: sem-teto!

Pessoa -> cpf, nome, sobrenome, sexo, nascimento.
Morador -> Pessoa + endereço (cep/face/num/complemento).
Dados serão gravados em um hashfile.
*/

// Constantes.
#define PESSOA_CPF_MAX 15 // Limite máximo do CPF.
#define PESSOA_NOME_MAX 50 // Nome ou sobrenome.
#define PESSOA_NASC_MAX 11 // (dd/mm/aaaa)\0.
#define PESSOA_CEP_MAX 20 // Limite máximo do CEP.
#define PESSOA_COMPL_MAX 20 // Limite máximo do complemento

// Códigos de retorno padrão para facilitar os testes.
#define PESSOA_OK 1
#define PESSOA_ERRO 0

// Capacidade do buffer de serialização.
#define PESSOA_SERIAL_MAX 256

// Sexo
typedef enum{
    SEXO_M = 'M',
    SEXO_F = 'F'
} Sexo;

// Tipo opaco para representar a pessoa.
typedef struct Pessoa Pessoa;

/**
 * @brief Função que cria uma pessoa e inicializa seus respectivos atributos.
 * @param cpf CPF - Identificador. Formato: "000.000.011-63"
 * @param nome Nome da pessoa.
 * @param sobrenome Sobrenome da pessoa.
 * @param sexo Caracter para o sexo da pessoa -> F-feminino / M-masculino
 * @param nasc Data de nascimento. Formato: "08/05/1976"
 * @return Ponteiro para a pessoa criada.
 * @details NULL em caso de erro.
 */
Pessoa* criarPessoa(const char *cpf, const char *nome, const char *sobrenome, char sexo, const char *nasc);

/**
 * @brief Função que libera a memória associada a uma pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada. 
 */
void destruirPessoa(Pessoa *p);

// Getters

/**
 * @brief Função que retorna o CPF de determinada pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada. 
 * @return String que representa o CPF.
 */
const char* getPessoaCpf(const Pessoa *p);

/**
 * @brief Função que retorna o nome de determinada pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada. 
 * @return String que representa o nome.
 */
const char* getPessoaNome(const Pessoa *p);

/**
 * @brief Função que retorna o sobrenome de determinada pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada. 
 * @return String que representa o sobrenome.
 */
const char* getPessoaSobrenome(const Pessoa *p);

/**
 * @brief Função que retorna o sexo de determinada pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada. 
 * @return Char que representa o sexo.
 * @details F - Feminino / M - Masculino.
 */
char getPessoaSexo(const Pessoa *p);

/**
 * @brief Função que retorna a data de nascimento de determinada pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada.  
 * @return String que representa a data de nascimento.
 */
const char* getPessoaNasc(const Pessoa *p);

// Endereço -> Moradores.

/**
 * @brief Função que indica se determinada pessoa é ou não é morador.
 * @param Pessoa Ponteiro para a pessoa analisada. 
 * @return 1 - Morador / 0 - Sem teto.
 * @details No caso, vamos usar PESSOA_OK -> 1 e PESSOA_ERRO -> 0 ou NULL.
 */
int ehMorador(const Pessoa *p);

/**
 * @brief Função que associa um endereço à pessoa, tornando-a moradora.
 * @param Pessoa Ponteiro para a pessoa analisada. 
 * @param cep CEP (alfanumérico) da quadra.
 * @param face Face da quadra ('N', 'S', 'L' ou 'O').
 * @param num Número do imóvel.
 * @param complemento Complemento (apto, bloco, etc.). Pode ser string vazia (opcional).
 * @return PESSOA_OK | PESSOA_ERRO.
 */
int associarEndereco(Pessoa *p, const char *cep, char face, int num, const char *complemento);

/**
 * @brief Remove o endereço da pessoa. Vira Sem-Teto.
 * @param Pessoa Ponteiro para a pessoa analisada.
 * @return PESSOA_OK | PESSOA_ERRO.
 */
int removerEndereco(Pessoa *p);

/**
 * @brief Função que retorna o CEP do endereço de determinada pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada.
 * @return String que representa o CEP. 
 */
const char* getPessoaEnderecoCep(const Pessoa *p);

/**
 * @brief Função que retorna a face da quadra onde o imóvel de determinada pessoa está.
 * @param Pessoa Ponteiro para a pessoa analisada.
 * @return char que representa a face. 
 */
char getPessoaEnderecoFace(const Pessoa *p);

/**
 * @brief Função que retorna o número do imóvel de determinada pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada.
 * @return Inteiro que representa o número do imóvel. -1 se não for morador ou ponteiro NULL.
 */
int getPessoaEnderecoNum(const Pessoa *p);

/**
 * @brief Função que retorna o complemento do endereço de determinada pessoa.
 * @param Pessoa Ponteiro para a pessoa analisada.
 * @return String que representa o endereço.
 */
const char* getPessoaEnderecoCompl(const Pessoa *p);

/**
 * @brief Verifica se o sexo é aceito ('F' ou 'M').
 * @param sexo Caracter que representa o sexo.
 * @return PESSOA_OK | PESSOA_ERRO.
 */
int sexoValido(char sexo);

/**
 * @brief Verifica se a data de nascimento é valida (dd/mm/aaaa).
 * @param nasc String da data de nascimento.
 * @return PESSOA_OK | PESSOA_ERRO.
 */
int nascValido(const char *nasc);

/**
 * @brief Serializa ("Empacota") todos os atributos da pessoa em uma string delimitada por "|" (pode ser também por outros modos, ex. ";"), adequada para armazenamento no HashFile.
 * @details Formato: "cpf|nome|sobrenome|sexo|nasc|0" -> sem endereço
 *          Formato: "cpf|nome|sobrenome|sexo|nasc|1|cep|face|num|compl"
 * sexto campo é flag de morador.
 * @param Pessoa Ponteiro para pessoa. 
 * @param buffer Destino da string serializada.
 * @param tam Tamanho do Buffer
 * @return PESSOA_OK | PESSOA_ERRO.
 */
int serializarPessoa(const Pessoa *p, char *buffer, int tam);

/**
 * @brief Reconstrói uma pessoa a partir de uma string serializada pelo formato de serializarQuadra().
 * @param buffer String no formato serializado.
 * @return Nova pessoa alocada em memória.
 */
Pessoa* desserializarPessoa(const char *buffer);

#endif