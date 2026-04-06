#ifndef LEITORPM_H
#define LEITORPM_H

#include "hash_extensivel.h"
#include "pessoa.h"

/*
TAD que será utilizado para leitura e processamento do arquivo .pm.
Esse TAD, basicamente, será capaz de ler os comandos .pm e, dessa maneira,
inicializar as pessoas/moradores e seus respectivos atributos.
Comandos reconhecidos do .pm:
- p cpf nome sobrenome sexo nasc -> insere habitante
- m cpf cep face num compl -> associa endereço a um habitante existente, tornando-o morador
Dados serão guardadas em um hashfile.
*/

// Códigos de retorno padrão para facilitar os testes.
#define LEITOR_PM_OK 1
#define LEITOR_PM_ERRO 0

/**
 * @brief Abre o arquivo .pm, lê linha por linha e insere cada pessoa (ou morador) no Hashfile fornecido. 
 * @param caminhoArqPm Nome do arquivo .pm a ser lido (diretório)
 * @param HashQuadras Hashfile aberto onde as quadras serão inseridas. 
 * @return LEITOR_PM_OK -> Sucesso / LEITOR_PM_ERRO -> caso .pm não puder ser aberto ou erro no HashPessoas.
 * @details Comando 'p' cria e insere a pessoa no hashfile. Comando 'm' busca a pessoa pelo CPF, associa o endereço e atualiza o registro no hashfile. 
 */
int lerArqPm(const char *caminhoArqPm, HashExtensivel *HashPessoas);

#endif