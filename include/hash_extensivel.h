#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

/*
TAD do HashFile dinâmico. Três tipos de arquivos podem ser gerados
<nome_arq>.hf - arquivo de dados (buckets)
<nome_arq>.hfc - arquivo de cabeçalho/diretório 
<nome_arq>.hfd - o dump.
*/ 

// Códigos de retorno padrão para facilitar os testes.
#define HASH_OK 1
#define HASH_ERRO 0


// Tipo opaco para representar o HashFile.
typedef struct HashExtensivel HashExtensivel;

/**
 * @brief Inicializa a estrutura do Hash File e abre/cria os arquivos no disco.
 * @param nome_arq Nome base para os arquivos de diretório e dados.
 * @param cap_bucket Capacidade máxima de registros por bucket. Essa inicilização permite o controle dos splits,
 * facilitando os testes. 
 * @return Ponteiro para o controlador do Hash.
 * @details NULL em caso de erro.
 */
HashExtensivel* abrirHash(const char* nome_arq, int cap_bucket);

/**
 * @brief Insere um novo registro no Hash File.
 * @param hash Ponteiro do Hash.
 * @param chave Chave de busca do registro (quadras ou pessoas).
 * @param dados Informações (na forma de string) a serem salvas.
 * @return HASH_OK (1) em caso de sucesso ou HASH_ERRO (0) caso falhar. 
 */
int inserirHash(HashExtensivel* hash, const char *chave, const char *dados);

/**
 * @brief Busca um registro atráves de sua chave.
 * @param hash Ponteiro do Hash.
 * @param chave Chave de busca do registro.
 * @param buffer_saida Array onde a string encontrada será copiada.
 * @param tam_buffer Tamanho máximo desse array, a fim de evitar um overflow.
 * @return HASH_OK (1) em caso de sucesso ou HASH_ERRO (0) caso falhar. 
 */
int buscarHash(HashExtensivel* hash, const char *chave, char* buffer_saida, int tam_buffer);

/**
 * @brief Remove fisicamente (ou logicamente) um registro do disco.
 * @param hash Ponteiro do Hash. 
 * @param chave Chave de busca do registro.
 * @return HASH_OK (1) em caso de sucesso ou HASH_ERRO (0) caso falhar. 
 * @details Lógica ou fisicamente, pois pode ser removido permanentemente ou apenas marcado o registro como excluída permitindo desfazer a ação.
 */
int removerHash(HashExtensivel* hash, const char *chave);

/**
 * @brief Fecha arquivos e libera memória do Hash File.
 * @param hash Ponteiro do Hash.
 */
void fecharHash(HashExtensivel* hash);

/**
 * @brief Gera arquivo .hfd com representação legível do hash e histórico de expansões.
 * @param hash Ponteiro do Hash.
 * @param nome_arq Nome base para o arquivo de saída (será criado nome_arq.hfd).
 */
void dumpHash(HashExtensivel* hash, const char* nome_arq);

#endif