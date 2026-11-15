#include <stdlib.h>
#include "arquivos.h"

// auxiliares compartilhadas pelos trabalhos 1 e 2

// abre o binario e confere o status. se falhar, mostra a mensagem e devolve null.
// quando o arquivo esta consistente, devolve o ponteiro no inicio.
FILE *abrir_arquivo(char *nome, char *modo)
{
    FILE *arquivo = fopen(nome, modo);
    char status;

    if (arquivo == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return NULL;
    }

    // o primeiro byte e o status; um arquivo inconsistente nao pode ser utilizado
    if (fread(&status, sizeof(char), 1, arquivo) != 1 || status != '1')
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivo);
        return NULL;
    }

    // devolve o cursor ao inicio para a proxima rotina poder ler o cabecalho completo
    rewind(arquivo);
    return arquivo;
}

// muda o primeiro byte e deixa o cursor logo depois dele.
// o fflush envia essa marcacao ao arquivo antes das outras alteracoes.
void alterar_status(FILE *arquivo, char status)
{
    // o status ocupa o mesmo primeiro byte nos arquivos de dados e no indice
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, arquivo);
    fflush(arquivo);
}

// soma os bytes do binario e mostra o resultado dividido por 100.
void BinarioNaTela(char *arquivo)
{
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb")))
    {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++)
    {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

// auxiliares trab. 1

// le os campos do cabecalho de conexoes e devolve a estrutura preenchida.
// depois da leitura, o cursor fica no byte 17.
CabecalhoConexoes ler_cabecalho_conexoes(FILE *arquivo)
{
    CabecalhoConexoes cabecalho;
    fseek(arquivo, 0, SEEK_SET);
    // a leitura segue a ordem dos campos nos 17 bytes iniciais
    fread(&cabecalho.status, sizeof(char), 1, arquivo);
    fread(&cabecalho.topo_pilha, sizeof(int), 1, arquivo);
    fread(&cabecalho.proxRRN, sizeof(int), 1, arquivo);
    fread(&cabecalho.nroRegRem, sizeof(int), 1, arquivo);
    fread(&cabecalho.nroPares, sizeof(int), 1, arquivo);
    return cabecalho;
}

// volta ao inicio para gravar o cabecalho das conexoes.
// o cursor termina no comeco dos registros de dados.
void escrever_cabecalho_conexoes(FILE *arquivo, CabecalhoConexoes *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET);
    // escreve os campos separadamente para nao incluir o padding da estrutura
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topo_pilha, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->proxRRN, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroRegRem, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroPares, sizeof(int), 1, arquivo);
}

// auxiliares trab. 2

// busca o cabecalho dos pops no inicio do arquivo e devolve seus valores.
// a leitura termina no byte 25.
CabecalhoPoPs ler_cabecalho(FILE *arquivo)
{
    CabecalhoPoPs cabecalho;

    fseek(arquivo, 0, SEEK_SET);
    fread(&cabecalho.status, sizeof(char), 1, arquivo);
    // os offsets usam oito bytes, pois indicam posicoes em bytes no arquivo
    fread(&cabecalho.topoLista, sizeof(int64_t), 1, arquivo);
    fread(&cabecalho.proxByteOffset, sizeof(int64_t), 1, arquivo);
    fread(&cabecalho.nroRegRem, sizeof(int), 1, arquivo);
    fread(&cabecalho.nroPoPs, sizeof(int), 1, arquivo);
    return cabecalho;
}

// grava os campos do cabecalho dos pops a partir do inicio do arquivo.
// o status usado e o que ja esta na estrutura; o cursor fica no byte 25.
void escrever_cabecalho(FILE *arquivo, CabecalhoPoPs *cabecalho)
{
    fseek(arquivo, 0, SEEK_SET);
    // o status ja deve estar definido em memoria junto com os contadores finais
    fwrite(&cabecalho->status, sizeof(char), 1, arquivo);
    fwrite(&cabecalho->topoLista, sizeof(int64_t), 1, arquivo);
    fwrite(&cabecalho->proxByteOffset, sizeof(int64_t), 1, arquivo);
    fwrite(&cabecalho->nroRegRem, sizeof(int), 1, arquivo);
    fwrite(&cabecalho->nroPoPs, sizeof(int), 1, arquivo);
}
