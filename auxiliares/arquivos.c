#include "arquivos.h"

// abre um binario consistente e retorna seu ponteiro, posicionado no inicio.
// em caso de falha, exibe a mensagem do enunciado e retorna null.
FILE *abrir_arquivo(char *nome, char *modo)
{
    FILE *arquivo = fopen(nome, modo);
    char status;

    if (arquivo == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return NULL;
    }

    if (fread(&status, sizeof(char), 1, arquivo) != 1 || status != '1')
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivo);
        return NULL;
    }

    rewind(arquivo);
    return arquivo;
}

// retorna o cabecalho lido campo a campo; move o cursor para o byte 25.
CabecalhoPoPs ler_cabecalho(FILE *arquivo)
{
    CabecalhoPoPs cabecalho;

    fseek(arquivo, 0, SEEK_SET);
    fread(&cabecalho.status, sizeof(char), 1, arquivo);
    fread(&cabecalho.topoLista, sizeof(int64_t), 1, arquivo);
    fread(&cabecalho.proxByteOffset, sizeof(int64_t), 1, arquivo);
    fread(&cabecalho.nroRegRem, sizeof(int), 1, arquivo);
    fread(&cabecalho.nroPoPs, sizeof(int), 1, arquivo);
    return cabecalho;
}

// grava o cabecalho desde o inicio do arquivo e deixa o cursor no byte 25.
// nao retorna valor e grava campo a campo, evitando bytes de padding.
void escrever_cabecalho(FILE *arquivo, CabecalhoPoPs *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topoLista, sizeof(int64_t), 1, arquivo);
    fwrite(&cabecalho->proxByteOffset, sizeof(int64_t), 1, arquivo);
    fwrite(&cabecalho->nroRegRem, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroPoPs, sizeof(int), 1, arquivo);
}

// altera somente o status e envia a escrita ao arquivo; nao retorna valor.
// o cursor fica no byte 1, servindo tanto para dados quanto para indice.
void alterar_status(FILE *arquivo, char status)
{
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, arquivo);
    fflush(arquivo);
}

// le e retorna o cabecalho das conexoes, movendo o cursor do inicio ate o byte 17.
CabecalhoConexoes ler_cabecalho_conexoes(FILE *arquivo)
{
    CabecalhoConexoes cabecalho;
    fseek(arquivo, 0, SEEK_SET);
    fread(&cabecalho.status, sizeof(char), 1, arquivo);
    fread(&cabecalho.topo_pilha, sizeof(int), 1, arquivo);
    fread(&cabecalho.proxRRN, sizeof(int), 1, arquivo);
    fread(&cabecalho.nroRegRem, sizeof(int), 1, arquivo);
    fread(&cabecalho.nroPares, sizeof(int), 1, arquivo);
    return cabecalho;
}

// grava o cabecalho das conexoes desde o inicio e deixa o cursor no byte 17.
// nao retorna valor; o status recebido ja deve ser o desejado para a gravacao.
void escrever_cabecalho_conexoes(FILE *arquivo, CabecalhoConexoes *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topo_pilha, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->proxRRN, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroRegRem, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroPares, sizeof(int), 1, arquivo);
}

// exibe a soma dos bytes dividida por 100, como a rotina fornecida pela disciplina.
// abre e fecha o binario internamente; nao retorna valor nem altera seu conteudo.
void BinarioNaTela(char *nome)
{
    FILE *arquivo = fopen(nome, "rb");
    if (arquivo == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    rewind(arquivo);
    unsigned long soma = 0;
    for (long i = 0; i < tamanho; i++)
        soma += (unsigned char)fgetc(arquivo);

    printf("%lf\n", soma / 100.0);
    fclose(arquivo);
}
