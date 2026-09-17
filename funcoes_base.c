#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funcionalidades.h"
#include "funcoes_base.h"

FILE *verificar_arquivo(char *arquivo_binario, char *modo_de_leitura)
{

    char temp_status; // variavel temporaria para guardar o valor lido do status

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


Criterios ler_criterios(int qtd_criterios)
{
    
    Criterios C; // inicia a struct dos criterios

        // inicio os criterios com valor -2, ja que 0 é um valor de busca valido e -1 corresponde ao nulo
        C.idPoPs = -2;
        C.idPoPsConectado = -2;
        C.velocidade = -2;
        C.unidadeMedida = -2;

        // laco que vai verificar os criterios a serem buscados
        for (int criterio_atual = 0; criterio_atual < qtd_criterios; criterio_atual++)
        {
            char campo[30];
            char valor[30];
            scanf("%s", campo);

            // vai verificar qual e o criterio a ser usado por meio de comparacoes
            // se o valor e nulo, o criterio respectivo assume -1
            if (strcmp(campo, "idPoPs") == 0)
            {
                scanf("%s", valor);
                if (strcmp(valor, "NULO") == 0)
                    C.idPoPs = -1;
                else
                    C.idPoPs = atoi(valor);
            }

            else if (strcmp(campo, "idPoPsConectado") == 0)
            {
                scanf("%s", valor);
                if (strcmp(valor, "NULO") == 0)
                    C.idPoPsConectado = -1;
                else
                    C.idPoPsConectado = atoi(valor);
            }

            else if (strcmp(campo, "velocidade") == 0)
            {
                scanf("%s", valor);
                if (strcmp(valor, "NULO") == 0)
                    C.velocidade = -1;
                else
                    C.velocidade = atoi(valor);
            }

            // verificao para a unidade de medida e diferente, ja que e uma string com apas
            // para isso e utilizada a funcao ScanQuoteString que faz a leitura da string e decide internamente
            // se o que veio da entrada e NULO, um valor entre aspas ou algo sem aspas
            else if (strcmp(campo, "unidadeMedida") == 0)
            {
                ScanQuoteString(valor);
                if (strcmp(valor, "") == 0)
                    C.unidadeMedida = '$';
                else
                    C.unidadeMedida = valor[0];
            }
        }
        return C;
    }



//funcao para printar
int escreve_arquivo(FILE *binario, Registro *Reg)
{
    fwrite(&Reg->removido,sizeof(char),1,binario);
    fwrite(&Reg->encadeamento,sizeof(int),1,binario);
    fwrite(&Reg->idPoPs,sizeof(int),1,binario);
    fwrite(&Reg->idPoPsConectado,sizeof(int),1,binario);
    fwrite(&Reg->velocidade,sizeof(int),1,binario);
    fwrite(&Reg->unidade_medida,sizeof(char),1,binario);
    return 1; 
}

int escreve_cabecalho(FILE *binario, RegCabecalho *Cab)
{
    fwrite(&Cab->status,sizeof(char),1,binario);
    fwrite(&Cab->topo_pilha,sizeof(int),1,binario);
    fwrite(&Cab->proxRRN,sizeof(int),1,binario);
    fwrite(&Cab->nroRegRem,sizeof(int),1,binario);
    fwrite(&Cab->nroPares,sizeof(int),1,binario);
    return 1;
}

// funcoes dadas na plataforma

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
