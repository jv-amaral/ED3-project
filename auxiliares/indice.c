#include <stdlib.h>
#include "indice.h"
#include "arquivos.h"

// carrega o indice em memoria e devolve a quantidade pelo parametro.
// retorna o vetor alocado ou null; move o cursor para o fim do arquivo.
IndicePoPs *carregar_indice(FILE *arquivo, int *quantidade)
{
    fseek(arquivo, 0, SEEK_END);
    *quantidade = (ftell(arquivo) - TAM_CABECALHO_INDICE) / TAM_REGISTRO_INDICE;
    IndicePoPs *indice = malloc((*quantidade + 1) * sizeof(IndicePoPs));
    if (indice == NULL)
        return NULL;

    fseek(arquivo, TAM_CABECALHO_INDICE, SEEK_SET);
    for (int i = 0; i < *quantidade; i++)
    {
        fread(&indice[i].idPoPs, sizeof(int), 1, arquivo);
        fread(&indice[i].byteoffset, sizeof(int64_t), 1, arquivo);
    }
    return indice;
}

// faz busca binaria no vetor ordenado; retorna a posicao encontrada ou -1.
// nao altera o vetor e nao movimenta o cursor de nenhum arquivo.
int buscar_no_indice(IndicePoPs *indice, int quantidade, int id)
{
    int inicio = 0;
    int fim = quantidade - 1;

    while (inicio <= fim)
    {
        int meio = (inicio + fim) / 2;
        if (indice[meio].idPoPs == id)
            return meio;
        if (indice[meio].idPoPs < id)
            inicio = meio + 1;
        else
            fim = meio - 1;
    }
    return -1;
}

// compara duas chaves para o qsort, retornando negativo, zero ou positivo.
// nao altera os registros e nao acessa arquivos.
int comparar_indices(const void *primeiro, const void *segundo)
{
    const IndicePoPs *indice_a = primeiro;
    const IndicePoPs *indice_b = segundo;
    return (indice_a->idPoPs > indice_b->idPoPs) - (indice_a->idPoPs < indice_b->idPoPs);
}

// retira uma entrada deslocando as seguintes e diminuindo a quantidade.
// nao retorna valor; preserva a ordem crescente e nao acessa arquivos.
void retirar_do_indice(IndicePoPs *indice, int *quantidade, int posicao)
{
    if (posicao == -1)
        return;

    for (int i = posicao; i < *quantidade - 1; i++)
        indice[i] = indice[i + 1];
    (*quantidade)--;
}

// recria o indice somente com as entradas ativas, eliminando a sobra do arquivo.
// abre e fecha o binario internamente; retorna 1 em sucesso ou 0 em falha.
int salvar_indice(char *nome, IndicePoPs *indice, int quantidade)
{
    FILE *arquivo = fopen(nome, "wb");
    if (arquivo == NULL)
        return 0;

    char status = '0';
    fwrite(&status, sizeof(char), 1, arquivo);
    for (int i = 0; i < quantidade; i++)
    {
        fwrite(&indice[i].idPoPs, sizeof(int), 1, arquivo);
        fwrite(&indice[i].byteoffset, sizeof(int64_t), 1, arquivo);
    }

    int sucesso = !ferror(arquivo) && fflush(arquivo) == 0;
    if (sucesso)
        alterar_status(arquivo, '1');
    if (fclose(arquivo) != 0)
        sucesso = 0;
    return sucesso;
}
