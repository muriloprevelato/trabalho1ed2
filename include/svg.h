#ifndef SVG_H
#define SVG_H

/*
Interface do módulo de escrita do SVG (saída).

Módulo que tem a responsabilidade de criar e escrever arquivos .svg que representam
visualmente o mapa/estado da cidade. Os comando do qry adicionam detalhes visuais 
às quadras desenhadas.

Os desenhos/representações possuem como base as necessidades impostas pelos comandos
do qry.
*/

#include "quadra.h"

// Códigos de retorno padrão para facilitar os testes.
#define SVG_OK 1
#define SVG_ERRO 0

// Tipo opaco.
typedef struct ArqSvg ArqSvg;

/**
 * @brief Função que cria o arquivo .svg e inicializa o cabeçalho XML/SVG.
 * @param caminho Caminho do arquivo a criar.
 * @return Ponteiro válido para o arquivo SVG.
 * @details NULL em caso de erro. * Svg se ajusta automaticamente.
 */
ArqSvg* abrirSvg(const char *caminho);

/**
 * @brief Escreve a tag de fechamento.
 * @param svg Ponteiro para o arquivo svg.
 * @details NULL é tolerado.
 */
void fecharSvg(ArqSvg *svg);

/**
 * @brief Função que desenha uma quadra com suas respectivas cores.
 * @details  Usa getQuadraX/Y/W/H/Sw/CFill/CStrk para obter os atributos
 * e gera um elemento <rect> no SVG.
 * @param svg Ponteiro do arquivo SVG.
 * @param q Ponteiro para quadra a ser desenhada.
 * @return SVG_OK -> 1 (Sucesso) / SVG_ERRO -> 0 (Erro).
 */
int desenharQuadra(ArqSvg *svg, const Quadra *q);

// Marcadores do .qry

/**
 * @brief Função que desenha um pequeno X vermelho na âncora da quadra
 * a ser removida
 * @details Usado pelo comando rq (remoção de quadra).
 * @param svg Ponteiro do arquivo SVG.
 * @param x Coordenada X do âncora.
 * @param y Coordenada Y do âncora.
 * @param w Largura da quadra.
 * @param h Altura da quadra.
 * @return SVG_OK -> 1 (Sucesso) / SVG_ERRO -> 0 (Erro).
 */
int desenharMarcadorX(ArqSvg *svg, double x, double y, double w, double h);

/**
 * @brief Função que coloca uma pequena cruz no endereço (se ela não for um sem-teto) 
 * da pessoa que morreu.
 * @details Usado pelo comando rip.
 * @param svg Ponteiro para o ArqSvg.
 * @param x Coordenada x do centro do marcador.
 * @param y Coordenada y do centro do marcador.
 * @return SVG_OK -> 1 (Sucesso) / SVG_ERRO -> 0 (Erro).
 */
int desenharMarcadorCruz(ArqSvg *svg, double x, double y);

/**
 * @brief Função que desenha um pequeno círculo preto na posição de despojo.
 * @details Usado pelo comando dspj.
 * @param svg Ponteiro para o ArqSvg.
 * @param x Coordenada x do centro do marcador.
 * @param y Coordenada y do centro do marcador.
 * @return SVG_OK -> 1 (Sucesso) / SVG_ERRO -> 0 (Erro).
 */
int desenharMarcadorCirculo(ArqSvg *svg, double x, double y);

/**
 * @brief Função que desenha um pequeno quadrado vermelho com o CPF dentro.
 * @details Usado pelo comando mud (mudança). * CPF escrito em fonte minúscula.
 * @param svg Ponteiro para o ArqSvg.
 * @param x Coordenada x do âncora (mesmo padrão da quadra) do marcador.
 * @param y Coordenada y do âncora (mesmo padrão da quadra) do marcador.
 * @param cpf CPF da pessoa que será exibido dentro do quadrado.
 * @param deslocamento varíavel para cuidar de múltiplas marcações e sobreposições.
 * @return SVG_OK -> 1 (Sucesso) / SVG_ERRO -> 0 (Erro). 
 */
int desenharMarcadorQuadrado(ArqSvg *svg, double x, double y, const char *cpf, int deslocamento);

/**
 * @brief Função que escreve um texto centralizado numa região retangular.
 * @details Usado pelo comando pq. Exibe o total de moradores da quadra (face e total).
 * @param svg Ponteiro para o ArqSvg.
 * @param cx Coordenada x do centro da região.
 * @param cy Coordenada y do centro da região. 
 * @param texto Texto a ser escrito.
 * @return SVG_OK -> 1 (Sucesso) / SVG_ERRO -> 0 (Erro)
 */
int desenharTextoCentro(ArqSvg *svg, double cx, double cy, const char *texto);

/**
 * @brief Função que escreve um texto.
 * @details Usado para posicionar texto próximo às faces da quadra (pq)
 * ou em qualquer posição arbitrária.
 * @param svg Ponteiro para o ArqSvg.
 * @param x Coordenada x do texto.
 * @param y Coordenada y do texto. 
 * @param texto Texto a ser escrito.
 * @return SVG_OK -> 1 (Sucesso) / SVG_ERRO -> 0 (Erro)
 */
int desenharTexto(ArqSvg *svg, double x, double y, const char *texto);

#endif