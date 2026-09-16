#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funcoes.h"
#include "funcoes_base.h"


FILE *verificar_arquivo(char *arquivo_binario, char *modo_de_leitura)
{

    char temp_status;

    // abre o arquivo
    FILE *binario = fopen(arquivo_binario, modo_de_leitura);
    // verificacoes basicas, de existencia e consistencia do arquivo
    if (binario == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return NULL;
    }

    fread(&temp_status, sizeof(char), 1, binario);
    if (temp_status != '1')
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(binario);
        return NULL;
    }

    return binario;
}

RegCabecalho Leitura_Cabecalho(FILE *binario)

{
    RegCabecalho cabecalho; // define o nome da struct
    // Le todos os dados do registro de cabecalho
    fseek(binario, 0, SEEK_SET);
    fread(&cabecalho.status, sizeof(char), 1, binario);
    fread(&cabecalho.topo_pilha, sizeof(int), 1, binario);
    fread(&cabecalho.proxRRN, sizeof(int), 1, binario);
    fread(&cabecalho.nroRegRem, sizeof(int), 1, binario);
    fread(&cabecalho.nroPares, sizeof(int), 1, binario);

    return cabecalho;
}

int Leitura_Registro(FILE *binario, Registro *Reg)
{
    if (fread(&Reg->removido, sizeof(char), 1, binario) != 1)
    {
        return 0;
    }

    fread(&Reg->encadeamento, sizeof(int), 1, binario);
    fread(&Reg->idPoPs, sizeof(int), 1, binario);
    fread(&Reg->idPoPsConectado, sizeof(int), 1, binario);
    fread(&Reg->velocidade, sizeof(int), 1, binario);
    fread(&Reg->unidade_medida, sizeof(char), 1, binario);

    return 1;
}



//funcoes dadas na plataforma

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

void ScanQuoteString(char *str)
{
    char R;

    while ((R = getchar()) != EOF && isspace(R))
        ; // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n')
    { // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    }
    else if (R == '\"')
    {
        if (scanf("%[^\"]", str) != 1)
        { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar(); // ignorar aspas fechando
    }
    else if (R != EOF)
    { // vc tá tentando ler uma string que não tá entre
      // aspas! Fazer leitura normal %s então, pois deve
      // ser algum inteiro ou algo assim...
        str[0] = R;
        scanf("%s", &str[1]);
    }
    else
    { // EOF
        strcpy(str, "");
    }
}
