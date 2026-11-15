#include <stdlib.h>
#include "indice.h"
#include "arquivos.h"


// auxiliares trab. 2

// calcula quantas entradas existem e devolve o vetor preenchido.
// a quantidade sai pelo parametro e o cursor termina no fim do indice.
IndicePoPs *carregar_indice(FILE *arquivo, int *quantidade)
{
    fseek(arquivo, 0, SEEK_END);
    // desconta o byte de status e divide pelos 12 bytes de cada entrada
    *quantidade = (ftell(arquivo) - TAM_CABECALHO_INDICE) / TAM_REGISTRO_INDICE;
    IndicePoPs *indice = malloc((*quantidade + 1) * sizeof(IndicePoPs));
    if (indice == NULL)
        return NULL;

    // ignora o cabecalho e carrega somente os pares de chave e offset
    fseek(arquivo, TAM_CABECALHO_INDICE, SEEK_SET);
    for (int i = 0; i < *quantidade; i++)
    {
        fread(&indice[i].idPoPs, sizeof(int), 1, arquivo);
        fread(&indice[i].byteoffset, sizeof(int64_t), 1, arquivo);
    }
    return indice;
}

// procura o id por busca binaria no vetor ja ordenado.
// o resultado e a posicao encontrada, ou -1 se a chave nao estiver no indice.
int buscar_no_indice(IndicePoPs *indice, int quantidade, int id)
{
    int inicio = 0;
    int fim = quantidade - 1;

    while (inicio <= fim)
    {
        // a ordem crescente permite descartar metade do intervalo a cada comparacao
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

// compara os ids para o qsort: menor vem antes, maior vem depois.
int comparar_indices(const void *primeiro, const void *segundo)
{
    const IndicePoPs *indice_a = primeiro;
    const IndicePoPs *indice_b = segundo;
    // chave menor resulta em valor negativo, chaves iguais em zero e maior em positivo
    return (indice_a->idPoPs > indice_b->idPoPs) - (indice_a->idPoPs < indice_b->idPoPs);
}

// fecha o espaco da entrada retirada, deslocando as seguintes para a esquerda.
// a quantidade diminui, mas a ordem do vetor continua a mesma.
void retirar_do_indice(IndicePoPs *indice, int *quantidade, int posicao)
{
    if (posicao == -1)
        return;

    // a ultima posicao deixa de contar, sem precisar realocar o vetor
    for (int i = posicao; i < *quantidade - 1; i++)
        indice[i] = indice[i + 1];
    (*quantidade)--;
}

// recria o arquivo com as entradas que ficaram no vetor e o fecha ao terminar.
// devolve 0 se nao conseguir abrir a saida e 1 depois de gravar o indice.
int salvar_indice(char *nome, IndicePoPs *indice, int quantidade)
{
    // wb elimina a versao anterior, incluindo entradas que foram removidas do vetor
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

    // como o cabecalho do indice tem um byte, sua finalizacao altera so o status
    alterar_status(arquivo, '1');
    fclose(arquivo);
    return 1;
}
