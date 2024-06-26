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

void extrair_endereco(uint32_t endereco, int *indice, int *tag, int bits_indice, int bits_deslocamento) {
    int bits_tag = TAM_ENDERECO - bits_indice - bits_deslocamento;
    *indice = (endereco >> bits_deslocamento) & ((1 << bits_indice) - 1);
    *tag = endereco >> (bits_indice + bits_deslocamento);
}

void simular_cache(Cache *cache, uint32_t endereco, char operacao, int *cont_acertos, int *cont_falhas, int *leituras_memoria, int *escritas_memoria) {
    int bits_deslocamento = __builtin_ctz(cache->tamanho_linha); //bit counting
    int bits_indice = __builtin_ctz(cache->quantidade_conjuntos);
    int indice, tag;
    extrair_endereco(endereco, &indice, &tag, bits_indice, bits_deslocamento);

    ConjuntoCache *conjunto = &cache->conjuntos[indice];
    int acerto = 0;
    int linha_vazia = -1;
    int linha_substituir = 0;

    // Verificar se há um acerto e encontrar uma linha vazia ou candidata para substituição
    for (int i = 0; i < cache->linhas_por_conjunto; i++) {
        if (conjunto->linhas[i].valido && conjunto->linhas[i].tag == tag) {
            acerto = 1;
            conjunto->linhas[i].contador_lru = 0;
            conjunto->linhas[i].contador_lfu++;
            if (operacao == 'W' && cache->politica_escrita == 1) {
                conjunto->linhas[i].sujo = 1;
            }
            break;
        }
        if (!conjunto->linhas[i].valido && linha_vazia == -1) {
            linha_vazia = i;
        }
        if (cache->politica_substituicao == 1 && conjunto->linhas[i].contador_lru > conjunto->linhas[linha_substituir].contador_lru) {
            linha_substituir = i;
        } else if (cache->politica_substituicao == 2 && conjunto->linhas[i].contador_lfu < conjunto->linhas[linha_substituir].contador_lfu) {
            linha_substituir = i;
        }
    }

    // Se houver acerto, atualizar o contador de acertos
    if (acerto) {
        (*cont_acertos)++;
    } else {
        // Lida com a falha
        (*cont_falhas)++;
        if (operacao == 'R') {
            (*leituras_memoria)++;
        } else if (operacao == 'W' && cache->politica_escrita == 0) {
            (*escritas_memoria)++;
        }

        int linha_usar = (linha_vazia != -1) ? linha_vazia : linha_substituir;

        if (cache->politica_escrita == 1 && conjunto->linhas[linha_usar].valido && conjunto->linhas[linha_usar].sujo) {
            (*escritas_memoria)++;
        }

        conjunto->linhas[linha_usar].valido = 1;
        conjunto->linhas[linha_usar].tag = tag;
        conjunto->linhas[linha_usar].contador_lru = 0;
        conjunto->linhas[linha_usar].contador_lfu = 1;
        conjunto->linhas[linha_usar].sujo = (operacao == 'W') ? 1 : 0;
    }

    // Atualizar contadores LRU
    for (int i = 0; i < cache->linhas_por_conjunto; i++) {
        if (conjunto->linhas[i].valido) {
            conjunto->linhas[i].contador_lru++;
        }
    }
}

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

void processar_arquivo_entrada(char *arquivo_entrada, Cache *cache, int *total_enderecos, int *cont_leituras, int *cont_escritas, int *cont_acertos, int *cont_falhas, int *leituras_memoria, int *escritas_memoria) {
    FILE *arquivo = fopen(arquivo_entrada, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        exit(1);
    }

    char linha[100];
    while (fgets(linha, sizeof(linha), arquivo)) {
        uint32_t endereco;
        char operacao;
        sscanf(linha, "%x %c", &endereco, &operacao);
        (*total_enderecos)++;
        if (operacao == 'R') {
            (*cont_leituras)++;
        } else if (operacao == 'W') {
            (*cont_escritas)++;
        }
        simular_cache(cache, endereco, operacao, cont_acertos, cont_falhas, leituras_memoria, escritas_memoria);
    }
    fclose(arquivo);
}

void calcular_metricas(int total_enderecos, int cont_leituras, int cont_escritas, int cont_acertos, int cont_falhas, int leituras_memoria, int escritas_memoria, int tempo_acesso, int tempo_leitura_memoria, int tempo_escrita_memoria, char *arquivo_saida, int politica_escrita, int tamanho_linha, int num_linhas, int associatividade, int politica_substituicao) {
    double taxa_acerto_leitura = (double)cont_acertos / cont_leituras * 100;
    double taxa_acerto_escrita = (double)cont_acertos / cont_escritas * 100;
    double taxa_acerto_global = (double)cont_acertos / total_enderecos * 100;
    double tempo_medio_acesso = ((double)cont_acertos * tempo_acesso + (double)cont_falhas * (tempo_leitura_memoria + tempo_acesso)) / total_enderecos;

    FILE *saida = fopen(arquivo_saida, "w");
    if (saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        exit(1);
    }

    fprintf(saida, "Política de Escrita: %d\n", politica_escrita);
    fprintf(saida, "Tamanho da Linha: %d bytes\n", tamanho_linha);
    fprintf(saida, "Número de Linhas: %d\n", num_linhas);
    fprintf(saida, "Associatividade: %d\n", associatividade);
    fprintf(saida, "Tempo de Acesso: %d ns\n", tempo_acesso);
    fprintf(saida, "Tempo de Leitura da Memória Principal: %d ns\n", tempo_leitura_memoria);
    fprintf(saida, "Tempo de Escrita da Memória Principal: %d ns\n", tempo_escrita_memoria);
    fprintf(saida, "Política de Substituição: %d\n", politica_substituicao);
    fprintf(saida, "Total de Endereços: %d\n", total_enderecos);
    fprintf(saida, "Total de Leituras: %d\n", cont_leituras);
    fprintf(saida, "Total de Escritas: %d\n", cont_escritas);
    fprintf(saida, "Leituras na Memória Principal: %d\n", leituras_memoria);
    fprintf(saida, "Escritas na Memória Principal: %d\n", escritas_memoria);
    fprintf(saida, "Taxa de Acerto em Leituras: %.4f%% (%d)\n", taxa_acerto_leitura, cont_acertos);
    fprintf(saida, "Taxa de Acerto em Escritas: %.4f%% (%d)\n", taxa_acerto_escrita, cont_acertos);
    fprintf(saida, "Taxa de Acerto Global: %.4f%% (%d)\n", taxa_acerto_global, cont_acertos);
    fprintf(saida, "Tempo Médio de Acesso: %.4f ns\n", tempo_medio_acesso);

    fclose(saida);
}

int main() {

    int politica_escrita, tamanho_linha, num_linhas, associatividade, tempo_acesso, tempo_leitura_memoria, tempo_escrita_memoria, politica_substituicao;
    char arquivo_entrada[100], arquivo_saida[100];

    ler_parametros(&politica_escrita, &tamanho_linha, &num_linhas, &associatividade, &tempo_acesso, &tempo_leitura_memoria, &tempo_escrita_memoria, &politica_substituicao, arquivo_entrada, arquivo_saida);

    int quantidade_conjuntos = num_linhas / associatividade;
    Cache *cache = inicializar_cache(quantidade_conjuntos, associatividade, tamanho_linha, politica_escrita, tempo_acesso, politica_substituicao);

    int total_enderecos = 0, cont_leituras = 0, cont_escritas = 0, cont_acertos = 0, cont_falhas = 0, leituras_memoria = 0, escritas_memoria = 0;

    processar_arquivo_entrada(arquivo_entrada, cache, &total_enderecos, &cont_leituras, &cont_escritas, &cont_acertos, &cont_falhas, &leituras_memoria, &escritas_memoria);

    calcular_metricas(total_enderecos, cont_leituras, cont_escritas, cont_acertos, cont_falhas, leituras_memoria, escritas_memoria, tempo_acesso, tempo_leitura_memoria, tempo_escrita_memoria, arquivo_saida, politica_escrita, tamanho_linha, num_linhas, associatividade, politica_substituicao);

    liberar_cache(cache);

    return 0;
}
