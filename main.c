#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TAM_ENDERECO 32

typedef struct {
    int valido;
    int sujo;
    uint32_t tag;
    int contador_lru; // Para LRU
    int contador_lfu; // Para LFU
} LinhaCache;

typedef struct {
    LinhaCache *linhas;
} ConjuntoCache;

typedef struct {
    ConjuntoCache *conjuntos;
    int quantidade_conjuntos;
    int linhas_por_conjunto;
    int tamanho_linha;
    int politica_escrita;
    int tempo_acesso;
    int politica_substituicao;
} Cache;

Cache* inicializar_cache(int quantidade_conjuntos, int linhas_por_conjunto, int tamanho_linha, int politica_escrita, int tempo_acesso, int politica_substituicao) {
    Cache *cache = (Cache *)malloc(sizeof(Cache));
    cache->quantidade_conjuntos = quantidade_conjuntos;
    cache->linhas_por_conjunto = linhas_por_conjunto;
    cache->tamanho_linha = tamanho_linha;
    cache->politica_escrita = politica_escrita;
    cache->tempo_acesso = tempo_acesso;
    cache->politica_substituicao = politica_substituicao;

    cache->conjuntos = (ConjuntoCache *)malloc(quantidade_conjuntos * sizeof(ConjuntoCache));
    for (int i = 0; i < quantidade_conjuntos; i++) {
        cache->conjuntos[i].linhas = (LinhaCache *)malloc(linhas_por_conjunto * sizeof(LinhaCache));
        for (int j = 0; j < linhas_por_conjunto; j++) {
            cache->conjuntos[i].linhas[j].valido = 0;
            cache->conjuntos[i].linhas[j].sujo = 0;
            cache->conjuntos[i].linhas[j].tag = 0;
            cache->conjuntos[i].linhas[j].contador_lru = 0;
            cache->conjuntos[i].linhas[j].contador_lfu = 0;
        }
    }
    return cache;
}

void liberar_cache(Cache *cache) {
    for (int i = 0; i < cache->quantidade_conjuntos; i++) {
        free(cache->conjuntos[i].linhas);
    }
    free(cache->conjuntos);
    free(cache);
}

void ler_parametros(int *politica_escrita, int *tamanho_linha, int *num_linhas, int *associatividade, int *tempo_acesso, int *tempo_leitura_memoria, int *tempo_escrita_memoria, int *politica_substituicao, char *arquivo_entrada, char *arquivo_saida) {
    printf("Informe a politica de escrita (0 para write-through, 1 para write-back): ");
    scanf("%d", politica_escrita);
    printf("Informe o tamanho da linha (bytes): ");
    scanf("%d", tamanho_linha);
    printf("Informe o numero de linhas: ");
    scanf("%d", num_linhas);
    printf("Informe a associatividade: ");
    scanf("%d", associatividade);
    printf("Informe o tempo de acesso (ns): ");
    scanf("%d", tempo_acesso);
    printf("Informe o tempo de leitura da memoria principal (ns): ");
    scanf("%d", tempo_leitura_memoria);
    printf("Informe o tempo de escrita da memoria principal (ns): ");
    scanf("%d", tempo_escrita_memoria);
    printf("Informe a politica de substituicao (0 para Aleatoria, 1 para LRU, 2 para LFU): ");
    scanf("%d", politica_substituicao);
    
    getchar();
    
    printf("Informe o nome do arquivo de entrada (com caminho): ");
    fgets(arquivo_entrada, 100, stdin);
    arquivo_entrada[strcspn(arquivo_entrada, "\n")] = 0;

    printf("Informe o nome do arquivo de saida: ");
    fgets(arquivo_saida, 100, stdin);
    arquivo_saida[strcspn(arquivo_saida, "\n")] = 0;
}

int main() {

    int politica_escrita, tamanho_linha, num_linhas, associatividade, tempo_acesso, tempo_leitura_memoria, tempo_escrita_memoria, politica_substituicao;
    char arquivo_entrada[100], arquivo_saida[100];

    ler_parametros(&politica_escrita, &tamanho_linha, &num_linhas, &associatividade, &tempo_acesso, &tempo_leitura_memoria, &tempo_escrita_memoria, &politica_substituicao, arquivo_entrada, arquivo_saida);

    int quantidade_conjuntos = num_linhas / associatividade;
    Cache *cache = inicializar_cache(quantidade_conjuntos, associatividade, tamanho_linha, politica_escrita, tempo_acesso, politica_substituicao);

    int total_enderecos = 0, cont_leituras = 0, cont_escritas = 0, cont_acertos = 0, cont_falhas = 0, leituras_memoria = 0, escritas_memoria = 0;

}
