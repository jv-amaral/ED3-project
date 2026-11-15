// coloquei esse nome so para começarmos a fazer as funcoes novas aqui
// depois ajustamos onde cada uma melhor se encaixa

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <funcionalidades2.h>
#include "funcoes_base2.h"

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

    while ((fgets(linha_csv, sizeof(linha_csv), arq_csv)) != NULL)
    {
         // atribui os valores do csv para os campos do registro
        // strtok é usada para procurar a virgula, e atoi converte o texto para o numero inteiro correspondente
        char *primeira_virgula = strchr(linha_csv, ',');
        char *segunda_virgula = strchr(primeira_virgula + 1, ',');
        char *terceira_virgula = strchr(segunda_virgula + 1, ',');
        char *quarta_virgula = strchr(terceira_virgula + 1, ',');

        Reg.idPoPs = atoi(strtok(linha_csv, ","));
        Reg.idPoPsConectado = atoi(strtok(NULL, ","));

        char *velocidade_ptr = segunda_virgula + 1;
        *terceira_virgula = '\0';

        // verifica se o campo velocidade é nulo e atribui -1 se for o caso, caso contrário, o valor lido do csv é copiado para a variável Reg.velocidade
        if (velocidade_ptr[0] == '\0' || velocidade_ptr[0] == ' ')
        {
            Reg.velocidade = -1; // atribui -1 se o campo for nulo
        }
        else
        {
            Reg.velocidade = atoi(velocidade_ptr);
        }

        char *unidade_medida_ptr = terceira_virgula + 1;
        unidade_medida_ptr[strcspn(unidade_medida_ptr, "\n\r")] = '\0';

        // verifica se o campo de unidade de medida é nulo e atribui '$' para ser usado como lixo
        if (unidade_medida_ptr[0] == '\0' || unidade_medida_ptr[0] == ' ')
        {
            Reg.unidade_medida = '$';
        }
        else
        {
            Reg.unidade_medida = unidade_medida_ptr[0];
        }

        // escreve os campos do registro no arquivo binário
        escreve_registro(binario, &Reg);

        // atualiza os valores do cabeçalho do proxRRN e do numero de pares
        cabecalho.proxRRN++;
        cabecalho.nroPares++;
    }

    // escreve o cabecalho atualizado
    fseek(binario, 0, SEEK_SET);
    escreve_cabecalho(binario, &cabecalho);

    // atualiza o status para 1, ou seja, consistente
    cabecalho.status = '1';

    // escreve o status por ultimo, para garantir a integridade do arquivo
    fseek(binario, 0, SEEK_SET);
    fwrite(&cabecalho.status, sizeof(char), 1, binario);

    fclose(csv);
    fclose(binario);

    BinarioNaTela(arquivo_binario);
}
    }
}



void create_index()
{
    char arquivo_binario[50]; // variaveis para os arquivos
    char arquivo_index[50];
    scanf("%s", &arquivo_binario); // le o nome dos arquivos
    scanf("%s",&arquivo_index);
    RegIndex registro;       // declaracao da struct do registro

     FILE *binario = verificar_arquivo(arquivo_binario, "rb");
    if (binario == NULL) // Se encontrar algum erro, ele retorna
    {
        return;
    }
    fseek(binario, 25, SEEK_SET);





}



