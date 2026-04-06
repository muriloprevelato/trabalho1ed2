#ifndef QUADRA_H
#define QUADRA_H

/*
Interface do módulo da Quadra.

Basicamente é um retângulo que será identificado por um CEP (alfanúmerico).
Tem 4 faces (pontos cardeais) e a âncora é o canto SUDESTE.

Já é interessante ressalta que o endereço de um imóvel tem essa cara: CEP/FACE/NÚMERO/COMPLEMENTO.
Dados serão gravados em um hashfile.
*/

// Constantes.
#define QUADRA_CEP_MAX 20 // Limite máximo do CEP
#define QUADRA_COR_MAX 32 // Limite máximo para cor.

// Códigos de retorno padrão para facilitar os testes.
#define QUADRA_OK 1
#define QUADRA_ERRO 0

/* Capacidade máxima do buffer de serialização (usado com o hashfile, para auxiliar na gravação do arquivo) */
#define QUADRA_SERIAL_MAX 256

// Etiqueta para faces da quadra.
typedef enum{
    FACE_N = 'N',
    FACE_S = 'S',
    FACE_L = 'L',
    FACE_O = 'O',
} FaceQuadra;

// Tipo opaco para representar a quadra.
typedef struct Quadra Quadra;

/**
 * @brief Função que cria uma quadra com seus respectivos atributos.
 * @param cep CEP alfanúmerico (identificador).
 * @param x Coordenada x do âncora.
 * @param y Coordenada y do âncora.
 * @param w Largura do retângulo (quadra).
 * @param h Altura do retângulo (quadra).
 * @param sw Espessura da borda.
 * @param cfill Cor de preenchimento.
 * @param cstrk Cor da borda.
 * @return Retorna um ponteiro para a quadra criada.
 * @details NULL em caso de erro.
 */
Quadra* criarQuadra(const char* cep, double x, double y, double w, double h, double sw, const char* cfill, const char* cstrk);

/**
 * @brief Função que libera a memória associada a uma quadra.
 * @param Quadra Ponteiro para a quadra analisada. 
 */
void destruirQuadra(Quadra *q);

// Getters.

/**
 * @brief Função que retorna o CEP de determinada quadra.
 * @param Quadra Ponteiro para a quadra analisada.
 * @return String que representa o CEP (alfanumérico).
 */
const char* getQuadraCep(const Quadra *q);

/**
 * @brief Função que retorna a coordenada X do âncora (canto SUDESTE do retângulo) da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a coordenada X.
 */
double getQuadraX(const Quadra *q);

/**
 * @brief Função que retorna a coordenada Y do âncora (canto SUDESTE do retângulo) da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a coordenada Y.
 */
double getQuadraY(const Quadra *q);

/**
 * @brief Função que retorna a largura da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a largura.
 */
double getQuadraW(const Quadra *q);

/**
 * @brief Função que retorna a altura da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a altura.
 */
double getQuadraH(const Quadra *q);

/**
 * @brief Função que retorna a espessura da borda da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a espessura.
 */
double getQuadraSw(const Quadra *q);

/**
 * @brief Função que retorna a cor de preenchimento da quadra.
 * @param Quadra Ponteiro para a quadra analisada.
 * @return String que representa a cor de preenchimento.
 */
const char* getQuadraCFill(const Quadra *q);

/**
 * @brief Função que retorna a cor de borda da quadra.
 * @param Quadra Ponteiro para a quadra analisada.
 * @return String que representa a cor de borda.
 */
const char* getQuadraCStrk(const Quadra *q);

// Fim getters

// Funções geométricas

/**
 * @brief Verifica se um ponto (px, py) está dentro dos limites da quadra.
 * @param Quadra Ponteiro para a quadra analisada.
 * @param px Coordenada X do ponto. 
 * @param py Coordena Y do ponto.
 * @return QUADRA_OK -> 1 (está) / Quadra_ERRO -> 0 (não está).
 * @details X, Y ponto de ancoragem (SE). x <= px <= x+w e y <= py <= y+h
 */
int quadraContemPonto(const Quadra *q, double px, double py);

/**
 * @brief Confirma (ou nõa) se o caracter informado corresponde a uma face existente.
 * @param face Caracter que indica face.
 * @return QUADRA_OK -> 1 (válida) / Quadra_ERRO -> 0 (não válida).
 */
int faceValida(char face);

// Integração com HashFile.

/**
 * @brief Serializa ("Empacota") todos os atributos da quadra em uma string delimitada por "|" (pode ser também por outros modos, ex. ";"), adequada para armazenamento no HashFile.
 * @details Formato: "cep|x|y|w|h|sw|cfill|cstrk"
 * @param Quadra Ponteiro para quadra. 
 * @param buffer Destino da string serializada.
 * @param tam Tamanho do Buffer
 * @return QUADRA_OK / QUADRA_ERRO
 */
int serializarQuadra(const Quadra *q, char *buffer, int tam);

/**
 * @brief Reconstrói uma quadra a partir de uma string serializada pelo formato de serializarQuadra().
 * @param buffer String no formato serializado (cep|x|y|w|h|sw|cfill|cstrk).
 * @return Nova quadra alocada em memória.
 */
Quadra *desserializarQuadra(const char *buffer);

#endif