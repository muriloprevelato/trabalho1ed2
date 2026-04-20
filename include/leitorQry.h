#ifndef LEITORQRY_H
#define LEITORQRY_H

/*
Módulo que vai processar os comandos do arquivo .qry.

É responsabilidade desse modo apenas ler e processar os comandos e,
a partir das instruções, consultar/modificar os hashfiles. Além disso,
o leitorQry deve produzir saídas para o relatório .txt e respostas visuais do .svg.

Comandos que o módulo tem que reconhecer:
- rq cep -> remover quadra / moradores viram sem-teto.
- pq cep -> número de moradores da quadra / face e total.
- censo -> estatísticas gerais.
- h? cpf -> dados do habitante especificado pelo cpf.
- nasc cpf nome sobrenome sexo nasc -> novo habitante.
- rip cpf -> morte de um habitante.
- mud cpf cep face num compl -> mudar endereço.
- dspj cpf -> morador é despejado / vira sem-teto.
*/

#include <stdio.h>
#include "hash_extensivel.h"
#include "svg.h"

// Códigos de retorno padrão para facilitar os testes.
#define QRY_OK 1
#define QRY_ERRO 0

/**
 * @brief Função que abre o arquivo .qry, processa os comandos e realiza as ações necessárias.
 * @param caminhoQry Caminho do arquivo .qry
 * @param quadras Hashfile das quadras.
 * @param pessoas Hashfile das pessoas.
 * @param svg Arquivo SVG (saída).
 * @param txt Arquivo TXT (saída).
 * @return QRY_OK -> 1 (sucesso) / QRY_ERRO -> 0 (Algum erro).
 */
int processarQry(const char* caminhoQry, HashExtensivel* quadras, HashExtensivel* pessoas, ArqSvg* svg, FILE* txt);

#endif