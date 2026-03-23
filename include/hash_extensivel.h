#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

// Códigos de retorno padrão para facilitar os testes.
#define HASH_OK 1
#define HASH_ERRO 0


// Tipo opaco para representar o HashFile.
typedef struct HashExtensivel HashExtensivel;

/**
 * @brief Inicializa a estrutura do Hash File e abre/cria os arquivos no disco.
 * @param nome_arq Nome base para os arquivos de diretório e dados.
 * @return Ponteiro para o controlador do Hash.
 * @details NULL em caso de erro.
 */
HashExtensivel* abrirHash(const char* nome_arq);

/**
 * @brief Insere um novo registro no Hash File.
 * @param hash Ponteiro do Hash.
 * @param id Chave de busca do registro.
 * @param dados Informações (na forma de string) a serem salvas.
 * @return HASH_OK (1) em caso de sucesso ou HASH_ERRO (0) caso falhar. 
 */
int inserirHash(HashExtensivel* hash, int id, const char *dados);

/**
 * @brief Busca um registro atráves de sua chave.
 * @param hash Ponteiro do Hash.
 * @param id Chave de busca do registro.
 * @param buffer_saida Array onde a string encontrada será copiada.
 * @param tam_buffer Tamanho máximo desse array, a fim de evitar um overflow.
 * @return HASH_OK (1) em caso de sucesso ou HASH_ERRO (0) caso falhar. 
 */
int buscarHash(HashExtensivel* hash, int id, char* buffer_saida, int tam_buffer);

/**
 * @brief Remove fisicamente (ou logicamente) um registro do disco.
 * @param hash Ponteiro do Hash. 
 * @param id Chave de busca do registro.
 * @return HASH_OK (1) em caso de sucesso ou HASH_ERRO (0) caso falhar. 
 * @details Lógica ou fisicamente, pois pode ser removido permanentemente ou apenas marcado o registro como excluída permitindo desfazer a ação.
 */
int removerHash(HashExtensivel* hash, int id);

/**
 * @brief Fecha arquivos e libera memória do Hash File.
 * @param hash Ponteiro do Hash.
 */
void fecharHash(HashExtensivel* hash);

/**
 * @brief Função de Dump para imprimir o conteúdo, de maneira legível, do HashFile.
 * @param hash Ponteiro do Hash.
 * @details Essa função visa ajudar na verificação de certos casos, erros e, portanto, na depuração.
 */
void dumpHash(HashExtensivel* hash);

#endif