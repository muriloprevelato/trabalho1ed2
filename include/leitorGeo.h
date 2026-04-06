#ifndef LEITORGEO_H
#define LEITORGEO_H

#include "hash_extensivel.h"
#include "quadra.h"

/*
TAD que será utilizado para leitura e processamento do arquivo .geo.
Esse TAD, basicamente, será capaz de ler os comandos .geo e, dessa maneira,
inicializar as quadras e seus respectivos atributos, bem como os atributos estilísticos.
Comandos reconhecidos do .geo:
- q  cep x y w h -> insere quadra com estilo corrente
- cq sw cfill cstrk -> atualiza estilo corrente (vale para as próximas q's)
As quadras serão guardadas em um hashfile.
*/

// Códigos de retorno padrão para facilitar os testes.
#define LEITOR_GEO_OK 1
#define LEITOR_GEO_ERRO 0

/**
 * @brief Abre o arquivo .geo, lê linha por linha e insere cada quadra no Hashfile fornecido. 
 * @param caminhoAqrGeo Nome do arquivo .geo a ser lido (diretório).
 * @param HashQuadras Hashfile aberto onde as quadras serão inseridas.
 * @return LEITOR_GEO_OK -> Sucesso / LEITOR_GEO_ERRO -> caso .geo não puder ser aberto ou erro no HashQuadras.
 * @details um comando 'cq' atualiza o estilo e todos os comandos 'q' seguintes usam esse estilo até o próximo 'cq'.
 */
int lerArqGeo(const char *caminhoAqrGeo, HashExtensivel *HashQuadras);

#endif