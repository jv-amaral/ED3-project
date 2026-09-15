// arquivo das funcoes

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"
#include "funcoes_base.h"

// Funcionalidade 1
void leitura_e_gravacao() // Função para ler o arquivo CSV e gravar os registros no arquivo binário
{

    // declara as variáveis para armazenar os nomes dos arquivos CSV e binário
    char arquivo_csv[50];
    char arquivo_binario[50];

    // le o nome dos arquivos CSV e binário
    scanf("%s", arquivo_csv);
    scanf("%s", arquivo_binario);

    RegCabecalho cabecalho;
    Registro Reg;

    // abre os arquivos para leitura e escrita, respectivamente

    FILE *csv = fopen(arquivo_csv, "r");
    FILE *binario = fopen(arquivo_binario, "wb");
    // verifica se os arquivos nao sao nulos
    if (csv == NULL || binario == NULL)
    {
        printf("Falha no processamento do arquivo.\n");

        if (csv != NULL) // se apenas um deles for nulo, o outro é fechado
        {
            fclose(csv);
        }
        if (binario != NULL)
        {
            fclose(binario);
        }
        return;
    }

    // atribui valores iniciais aos campos do cabeçalho e escreve o registro de cabeçalho no arquivo binário
    cabecalho.status = '0';
    cabecalho.topo_pilha = -1;
    cabecalho.proxRRN = 0;
    cabecalho.nroRegRem = 0;
    cabecalho.nroPares = 0;

    fwrite(&cabecalho.status, sizeof(char), 1, binario);
    fwrite(&cabecalho.topo_pilha, sizeof(int), 1, binario);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, binario);
    fwrite(&cabecalho.nroRegRem, sizeof(int), 1, binario);
    fwrite(&cabecalho.nroPares, sizeof(int), 1, binario);

    // atribuicao inicial de valores aos campos do registro
    Reg.removido = '0';
    Reg.encadeamento = -1;
    char linha_csv[100];
    fgets(linha_csv, sizeof(linha_csv), csv); // le a primeira linha do arquivo csv e descarta, porque é so os titulos dos campos

    // inicia o loop, fazendo a verificação de nulo
    while ((fgets(linha_csv, sizeof(linha_csv), csv)) != NULL)
    {

        // atribui os valores do csv para os campos do registro
        // strtok é usada para procurar a virgula, e atoi converte o texto para o numero inteiro correspondente
        Reg.idPoPs = atoi(strtok(linha_csv, ","));
        Reg.idPoPsConectado = atoi(strtok(NULL, ","));

        char *velocidade_ptr = strtok(NULL, ",");
        // verifica se o campo velocidade é nulo e atribui -1 se for o caso, caso contrário, o valor lido do csv é copiado para a variável Reg.velocidade
        if (velocidade_ptr == NULL || velocidade_ptr[0] == ' ')
        {
            Reg.velocidade = -1; // atribui -1 se o campo for nulo
        }
        else
        {
            Reg.velocidade = atoi(velocidade_ptr);
        }

        char *unidade_medida_ptr = strtok(NULL, ",\n\r");
        // verifica se o campo de unidade de medida é nulo e atribui '$' para ser usado como lixo
        if (unidade_medida_ptr == NULL || unidade_medida_ptr[0] == ' ')
        {
            Reg.unidade_medida = '$';
        }
        else
        {
            Reg.unidade_medida = unidade_medida_ptr[0];
        }

        // escreve os campos do registro no arquivo binário

        fwrite(&Reg.removido, sizeof(char), 1, binario);
        fwrite(&Reg.encadeamento, sizeof(int), 1, binario);
        fwrite(&Reg.idPoPs, sizeof(int), 1, binario);
        fwrite(&Reg.idPoPsConectado, sizeof(int), 1, binario);
        fwrite(&Reg.velocidade, sizeof(int), 1, binario);
        fwrite(&Reg.unidade_medida, sizeof(char), 1, binario);

        // atualiza os valores do cabeçalho do proxRRN e do numero de pares
        cabecalho.proxRRN++;
        cabecalho.nroPares++;
    }

    // atualiza o status para 1 e grava no byteoffset 0 do arquivo binario
    cabecalho.status = '1';
    fseek(binario, 0, SEEK_SET);
    fwrite(&cabecalho.status, sizeof(char), 1, binario);

    // grava no byteoffset 5 o valor atualizado do proxRRN
    fseek(binario, 5, SEEK_SET);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, binario);

    // grava no byteoffset 13 o valor atualizado do nroPares
    fseek(binario, 13, SEEK_SET);
    fwrite(&cabecalho.nroPares, sizeof(int), 1, binario);

    fclose(csv);
    fclose(binario);
    BinarioNaTela(arquivo_binario);
}

// aqui se encerra a funcionalidade 1 / Create Table
//_______________________________

// Funcionalidade 2
void recuperacao_dados() // funcao para recuperar os dados do arquivo binario e imprimir na tela
{
    // declara a variavel para armazenar o nome do arquivo e o lê
    char arquivo_binario[50];
    scanf("%s", arquivo_binario);

    RegCabecalho cabecalho; // definicao das structs  para a segunda funcao
    Registro Reg;

    // abre o arquivo para leitura
    FILE *binario = fopen(arquivo_binario, "rb");
    if (binario == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fread(&cabecalho.status, sizeof(char), 1, binario);
    if (cabecalho.status != '1') // verifica se o arquivo está consistente
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(binario);
        return;
    }

    // pula para o byteoffset 17 do arquivo (pois é onde começam os registros) e os lê
    fseek(binario, 17, SEEK_SET);

    int registros_lidos = 0; // contador para verificar se algum registro foi lido

    while (fread(&Reg.removido, sizeof(char), 1, binario) == 1) // se o registro existir ele será lido
    {
        if (Reg.removido == '1') // se o registro estiver removido, ele é ignorado
        {
            fseek(binario, 17, SEEK_CUR); // pula para o proximo registro
            continue;
        }
        registros_lidos++;
        // leitura dos bytes do arquivo binario para os campos do registro
        fread(&Reg.encadeamento, sizeof(int), 1, binario);
        fread(&Reg.idPoPs, sizeof(int), 1, binario);
        fread(&Reg.idPoPsConectado, sizeof(int), 1, binario);
        fread(&Reg.velocidade, sizeof(int), 1, binario);
        fread(&Reg.unidade_medida, sizeof(char), 1, binario);
        // atribuicao dos valores nulos na hora de printar na tela
        if (Reg.velocidade == -1)
        {
            printf("%d  %d  %s  \"%c\"\n", Reg.idPoPs, Reg.idPoPsConectado, "NULO", Reg.unidade_medida);
        }
        else if (Reg.unidade_medida == '$')
        {
            printf("%d  %d  %d  \"%s\"\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.velocidade, "NULO");
        }
        else
        { // caso nao seja nenhum valor nulo, printa normalmente
            printf("%d  %d  %d  \"%c\"\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.velocidade, Reg.unidade_medida);
        }
    }
    if (registros_lidos == 0) // printa na tela caso nenhum registro tenha sido lido
    {
        printf("Registro inexistente.\n");
    }
    fclose(binario);
}

// aqui se encerra a funcionalidade 2
//_______________________________

//______________________
// Funcionalidade 3

void busca_condicional()
{
    // crio as variaveis do nome do arquivo e do numero de repeticoes da busca
    char arquivo_binario[50];
    int repeticoes;

    scanf("%s %d", arquivo_binario, &repeticoes);

    // se ao tentar abrir o arquivo um erro for encontrado, uma mensagem sera exibida
    FILE *binario = fopen(arquivo_binario, "rb");
    if (binario == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // irá ser feita a leitura do status do cabecalho para verificar a consistencia do arquivo
    RegCabecalho cabecalho;
    fread(&cabecalho.status, sizeof(char), 1, binario);
    if (cabecalho.status != '1')
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(binario);
        return;
    }

    // inicia o laco externo de n buscas
    for (int busca_atual = 0; busca_atual < repeticoes; busca_atual++)
    {
        // vai ver quantos criterios serao levados em conta na busca
        int qtd_criterios;
        scanf("%d", &qtd_criterios);

        // inicio os criterios com valor -2, ja que 0 é um valor de busca valido e -1 corresponde ao nulo
        int criterio_idPoPs = -2;
        int criterio_idPoPsConectado = -2;
        int criterio_velocidade = -2;
        char criterio_unidadeMedida = -2;

        for (int criterio_atual = 0; criterio_atual < qtd_criterios; criterio_atual++)
        {
            char campo[30];
            char valor[30];
            scanf("%s", campo);

            // vai verificar qual e o criterio a ser usado por meio de comparacoes
            // se o valor e nulo, o criterio respectivo assume -1
            if (strcmp(campo, "idPops") == 0)
            {
                scanf("%s", valor);
                if (strcmp(valor, "NULO") == 0)
                    criterio_idPoPs = -1;
                else
                    criterio_idPoPs = atoi(valor);
            }

            else if (strcmp(campo, "idPopsConectado") == 0)
            {
                scanf("%s", valor);
                if (strcmp(valor, "NULO") == 0)
                    criterio_idPoPsConectado = -1;
                else
                    criterio_idPoPsConectado = atoi(valor);
            }

            else if (strcmp(campo, "velocidade") == 0)
            {
                scanf("%s", valor);
                if (strcmp(valor, "NULO") == 0)
                    criterio_velocidade = -1;
                else
                    criterio_velocidade = atoi(valor);
            }

            // verificao para a unidade de medida e diferente, ja que e uma string com apas
            // para isso e utilizada a funcao ScanQuoteString que faz a leitura da string e decide internamente
            // se o que veio da entrada e NULO, um valor entre aspas ou algo sem aspas
            else if (strcmp(campo, "unidadeMedida") == 0)
            {
                ScanQuoteString(valor);
                if (strcmp(valor, "") == 0)
                    criterio_unidadeMedida = '$';
                else
                    criterio_unidadeMedida = valor[0];
            }
        }

        // pula para o byteoffset 17 do arquivo (pois é onde começam os registros)
        fseek(binario, 17, SEEK_SET);
        Registro Reg;

        /////////////////////////////////////////////
        int matches_found = 0;

        while (fread(&Reg.removido, sizeof(char), 1, binario) == 1)
        {
            fread(&Reg.encadeamento, sizeof(int), 1, binario);
            fread(&Reg.idPoPs, sizeof(int), 1, binario);
            fread(&Reg.idPoPsConectado, sizeof(int), 1, binario);
            fread(&Reg.velocidade, sizeof(int), 1, binario);
            fread(&Reg.unidade_medida, sizeof(char), 1, binario);

            if (Reg.removido == '1')
                continue;

            int match = 1;
            if (c_idPoPs != -2 && Reg.idPoPs != c_idPoPs)
                match = 0;
            if (c_idPoPsConectado != -2 && Reg.idPoPsConectado != c_idPoPsConectado)
                match = 0;
            if (c_velocidade != -2 && Reg.velocidade != c_velocidade)
                match = 0;
            if (c_unidade != -2 && Reg.unidade_medida != c_unidade)
                match = 0;

            if (match)
            {
                matches_found++;
                if (Reg.velocidade == -1 && Reg.unidade_medida == '$')
                {
                    printf("%d %d NULO NULO\n", Reg.idPoPs, Reg.idPoPsConectado);
                }
                else if (Reg.velocidade == -1)
                {
                    printf("%d %d NULO \"%c\"\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.unidade_medida);
                }
                else if (Reg.unidade_medida == '$')
                {
                    printf("%d %d %d NULO\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.velocidade);
                }
                else
                {
                    printf("%d %d %d \"%c\"\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.velocidade, Reg.unidade_medida);
                }
            }
        }

        if (matches_found == 0)
        {
            printf("Registro inexistente.\n");
        }
    }

    fclose(binario);
}
// aqui se encerra a funcionalidade 3
//_______________________________

// Funcionalidade 4
void busca_RRN()
{
    int RRN;
    char arquivo_binario[50];

    scanf("%s", arquivo_binario);

    FILE *binario = verificar_arquivo(arquivo_binario, "rb");

    if (binario == NULL)
    return;
    // verifica qual o RRN desejado pelo usuario
    scanf("%d", &RRN);

    // sabe-se que cada registro tem 1 + 4 + 4 + 4 + 4 + 1 (char, int, int, int, int, char) bytes = 18 bytes

    // comeco a procurar o RRN a partir do fim do cabecalho
    fseek(binario, 17 + RRN * 18, SEEK_SET);

    Registro Reg = Leitura_registro(binario, RRN);

    if (Reg.removido == '1')
    {
        printf("Registro inexistente.\n");
    }

    else if (Reg.removido == '0')
    {
    printf("%d ", Reg.idPoPs);
    printf("%d ", Reg.idPoPsConectado);
    printf("%d ", Reg.velocidade);
    printf("%s", Reg.unidade_medida);
    }

    fclose(binario);
}

// aqui se encerra a funcionalidade 4
//_______________________________

// Funcionalidade 6
void insercao()

{
    // declara a variavel para armazenar o nome do arquivo e ler
    char arquivo_binario[50];
    scanf("%s", arquivo_binario);

    RegCabecalho cabecalho; // definicao das structs para a sexta funcao
    Registro Reg;

    // abre o arquivo para leitura e escrita
    FILE *binario = fopen(arquivo_binario, "rb+");
    if (binario == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fread(&cabecalho.status, sizeof(char), 1, binario);
    if (cabecalho.status != '1')
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(binario);
        return;
    }

    // Le do arquivo os valores dos campos do cabeçalho
    fread(&cabecalho.topo_pilha, sizeof(int), 1, binario);
    fread(&cabecalho.proxRRN, sizeof(int), 1, binario);
    fread(&cabecalho.nroRegRem, sizeof(int), 1, binario);
    fread(&cabecalho.nroPares, sizeof(int), 1, binario);

    cabecalho.status = '0'; // como vamos escrever no arquivo, o status deve estar inconsistente
    fseek(binario, 0, SEEK_SET);
    fwrite(&cabecalho.status, sizeof(char), 1, binario);

    // le do teclado os valores dos campos a serem inseridos

    // cria strings temporarias para utilizar a funcao scanQuoteString para trabalhar com valores nulos e caracteres entre aspas
    // dos campos de velocidade e unidade de medida
    char temp_velocidade[20];
    char temp_unidade_medida[20];
    int n;
    scanf("%d", &n); // le o numero de registros a serem inseridos

    for (int i = 0; i < n; i++)
    { // le os valores passados pelo teclado para os campos do registro
        scanf("%d", &Reg.idPoPs);
        scanf("%d", &Reg.idPoPsConectado);
        ScanQuoteString(temp_velocidade);
        ScanQuoteString(temp_unidade_medida);
        // faz a comparação das strings temporarias com o valor nulo, se for verdadeiro atribui -1 a velocidade e $ a unidade
        // de medida, se for falso, ele atribui o valor lido do teclado para velocidade e o caractere lido sem aspas
        if (strcmp(temp_velocidade, "") == 0)
        {
            Reg.velocidade = -1; // atribui -1 se o campo for nulo
        }
        else
        {
            Reg.velocidade = atoi(temp_velocidade);
        }
        if (strcmp(temp_unidade_medida, "") == 0)
        {
            Reg.unidade_medida = '$'; // atribui '$' se o campo for nulo
        }
        else
        {
            Reg.unidade_medida = temp_unidade_medida[0];
        }
        if (cabecalho.topo_pilha == -1) // verifica se há registros removidos pelo topo da pilha
        {

            fseek(binario, 17 + cabecalho.proxRRN * 18, SEEK_SET);
            Reg.removido = '0';
            Reg.encadeamento = -1;
            fwrite(&Reg.removido, sizeof(char), 1, binario);
            fwrite(&Reg.encadeamento, sizeof(int), 1, binario);
            fwrite(&Reg.idPoPs, sizeof(int), 1, binario);
            fwrite(&Reg.idPoPsConectado, sizeof(int), 1, binario);
            fwrite(&Reg.velocidade, sizeof(int), 1, binario);
            fwrite(&Reg.unidade_medida, sizeof(char), 1, binario);
            cabecalho.proxRRN++;
            cabecalho.nroPares++;

            // Atualiza os valores do cabecalho que foram alterados dentro da condicional
        }
        else
        {
            int proximo_registro;

            fseek(binario, 17 + cabecalho.topo_pilha * 18 + 1, SEEK_SET);
            fread(&proximo_registro, sizeof(int), 1, binario); // verifica qual o proximo registro
            int temp = cabecalho.topo_pilha;                   // guarda o valor do topo da pilha em uma variavel temporaria
            cabecalho.topo_pilha = proximo_registro;           // removido e atualiza o topo da pilha para esse valor
            Reg.encadeamento = -1;                             // atualiza o encadeamento para -1 indicando que o registro não está mais removido

            fseek(binario, 17 + temp * 18, SEEK_SET);
            Reg.removido = '0'; // atribui o valor 0 ao novo registro inserido e escreve no arquivo
            fwrite(&Reg.removido, sizeof(char), 1, binario);
            fwrite(&Reg.encadeamento, sizeof(int), 1, binario); // escreve no arquivo o valor do encadeamento
            fwrite(&Reg.idPoPs, sizeof(int), 1, binario);       // escreve no arquivo os campos lidos do teclado
            fwrite(&Reg.idPoPsConectado, sizeof(int), 1, binario);
            fwrite(&Reg.velocidade, sizeof(int), 1, binario);
            fwrite(&Reg.unidade_medida, sizeof(char), 1, binario);
            cabecalho.nroRegRem--; // atualiza o numero de registros removidos no cabecalho
            cabecalho.nroPares++;  // atualiza o numero de pares no cabecalho
        }
    }
    // Atualiza os valores do cabecalho que foram alterados
    cabecalho.status = '1';
    fseek(binario, 0, SEEK_SET);
    fwrite(&cabecalho.status, sizeof(char), 1, binario);
    fwrite(&cabecalho.topo_pilha, sizeof(int), 1, binario);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, binario);
    fwrite(&cabecalho.nroRegRem, sizeof(int), 1, binario);
    fwrite(&cabecalho.nroPares, sizeof(int), 1, binario);

    fclose(binario);
    BinarioNaTela(arquivo_binario); // chama a funcao para imprimir o arquivo binario na tela
}

// aqui se encerra a funcionalidade 6
//_______________________________
