// coloquei esse nome so para começarmos a fazer as funcoes novas aqui
// depois ajustamos onde cada uma melhor se encaixa

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <funcionalidades2.h>
#include "funcoes_base.h"

// Funcionalidade 8

void create_table()
{
    char nome_arq_entrada[50];
    char nome_arq_saida[50];

    scanf("%s %s", nome_arq_entrada, nome_arq_saida);

    RegCabecalho cabecalho;
    Registro registro;

    FILE *arq_csv = fopen(nome_arq_entrada, "r");

    if (arq_csv == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *arq_binario = fopen(nome_arq_saida, "wb");
    if (arq_binario == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    cabecalho.status = '0';
    cabecalho.topoLista = -1;
    cabecalho.proxByteOffset = 0;
    cabecalho.nroRegRem = 0;
    cabecalho.nroPoPs = 0;

    escreve_cabecalho2(arq_binario, &cabecalho);

    //inicialização dos registros

    registro.removido = 0;
    registro.tamanhoRegistro = 0;
    registro.encadeamentoLista = -1;

    char linha_csv[200];

    //ler e descartar primeira linha, a qual representa o título dos campos
    fgets(linha_csv, sizeof(linha_csv), arq_csv);

    //inicia o loop fazendo a verificação de nulo

    wihile ((fgets(linha_csv, sizeof(linha_csv), arq_csv)) != NULL)
    {

    }
}
