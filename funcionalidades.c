// arquivo das funcoes

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionalidades.h"
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

    escreve_cabecalho(binario, &cabecalho);
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

        escreve_arquivo(binario, &Reg);

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
    // declara a variavel para armazenar o nome do arquivo e o le
    char arquivo_binario[50];
    scanf("%s", arquivo_binario);

   
    Registro Reg;
    FILE *binario = verificar_arquivo(arquivo_binario, "rb");
    if (binario == NULL) // Se encontrar algum erro, ele retorna
    {
        return;
    }
    // pula para o byteoffset 17 do arquivo (pois e onde começam os registros) e os le
    fseek(binario, 17, SEEK_SET);

    int registros_lidos = 0; // contador para verificar se algum registro foi lido

    while (Leitura_Registro(binario,&Reg)==1) // se o registro existir ele será lido
    {
        if (Reg.removido == '1') // se o registro estiver removido, ele é ignorado
        {
            continue;
        }
        registros_lidos++; // incrementa se o registro for lido
        // atribuicao dos valores nulos na hora de printar na tela

    
        if (Reg.velocidade == -1 && Reg.unidade_medida == '$')
        {
            printf("%d %d %s \"%s\"\n", Reg.idPoPs, Reg.idPoPsConectado, "NULO", "NULO");
        } else if (Reg.velocidade == -1)
        {
            printf("%d %d %s \"%c\"\n", Reg.idPoPs, Reg.idPoPsConectado, "NULO", Reg.unidade_medida);
        }
        else if (Reg.unidade_medida == '$')
        {
            printf("%d %d %d \"%s\"\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.velocidade, "NULO");
        }
        else 
        { // caso nao seja nenhum valor nulo, printa normalmente
            printf("%d %d %d \"%c\"\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.velocidade, Reg.unidade_medida);
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

    //ler o nome do arquivo e o numero de repeticoes a serem levadas em conta
    scanf("%s %d", arquivo_binario, &repeticoes);

    FILE *binario = verificar_arquivo(arquivo_binario, "rb");

    if (binario == NULL)
    {
        return;
    }
    
    // inicia o laco externo de n buscas
    for (int busca_atual = 0; busca_atual < repeticoes; busca_atual++)
    {
        // vai ver quantos criterios serao levados em conta na busca
        int qtd_criterios;
        scanf("%d", &qtd_criterios);

        Criterios C = ler_criterios(qtd_criterios);
        
        // pula para o byteoffset 17 do arquivo (pois é onde começam os registros)
        fseek(binario, 17, SEEK_SET);
        Registro Reg;

        int registros_encontrados = 0;

        //comeca a percorrer todos os registros, com a condicao de que quando nao encontrar
        //registro de removido, 0 ou 1, o arquivo terminou

        while (Leitura_Registro(binario, &Reg))
        {
            //inicia a variavel encontro como 1, que torna-se 0 se no registro percorrido no momento
            //nao e encontrada o valor desejado

            if(Reg.removido == '1')
                continue;

            int encontro = 1;
            if (C.idPoPs != -2 && Reg.idPoPs != C.idPoPs)
                encontro = 0;
            if (C.idPoPsConectado != -2 && Reg.idPoPsConectado != C.idPoPsConectado)
                encontro = 0;
            if (C.velocidade != -2 && Reg.velocidade != C.velocidade)
                encontro = 0;
            if (C.unidadeMedida != -2 && Reg.unidade_medida != C.unidadeMedida)
                encontro = 0;

            //se a variavel encontro permanece 1, o numero de registros encontrados aumenta
            //e eles sao printados, respeitando os valores nulos no reg.velocidade e reg.unidade_medida

            if (encontro)
            {
                registros_encontrados++;
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

        //se nao e encontrado nenhum registro que contem o valor do
        //campo ou o campo pertence a um registro que esteja removido
        if (registros_encontrados == 0)
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

    Registro Reg;

    // vai verificar se a leitura ocorreu de fato e se o registro esta removido ou nao
    if (!Leitura_Registro(binario, &Reg) || Reg.removido == '1')
    {
        printf("Registro inexistente.\n");
    }
    // ira printar as informacoes necessarias com o tratamento de NULO e de -1
    else
    {
        printf("%d %d ", Reg.idPoPs, Reg.idPoPsConectado);

        if (Reg.velocidade == -1)
        {
            printf("NULO ");
        }
        else
        {
            printf("%d ", Reg.velocidade);
        }

        if (Reg.unidade_medida == '$')
        {
            printf("NULO\n");
        }
        else
        {
            printf("\"%c\"\n", Reg.unidade_medida);
        }
    }

    fclose(binario);
}

// aqui se encerra a funcionalidade 4
//_______________________________


// Funcionalidade 5

void remocao_logica()
{
    return;
}

// Funcionalidade 6
void insercao()

{
    // declara a variavel para armazenar o nome do arquivo e ler
    char arquivo_binario[50];
    scanf("%s", arquivo_binario);

    Registro Reg;

    // abre o arquivo para leitura e escrita e verifica se está corrompido
    FILE *binario = verificar_arquivo(arquivo_binario, "rb+");
    if (binario == NULL) // Se encontrar algum erro, ele retorna
    {
        return;
    }
    RegCabecalho cabecalho = Leitura_Cabecalho(binario); // definicao da struct e leitura do arquivo binario

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
        if (strcmp(temp_velocidade, "" ) == 0 || strcmp(temp_velocidade,"NULO")==0)
        {
            Reg.velocidade = -1; // atribui -1 se o campo for nulo
        }
        else
        {
            Reg.velocidade = atoi(temp_velocidade);
        }
        if (strcmp(temp_unidade_medida, "") == 0 || strcmp(temp_unidade_medida,"NULO") ==0)
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
            escreve_arquivo(binario,&Reg);
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
            escreve_arquivo(binario, &Reg);
            cabecalho.nroRegRem--; // atualiza o numero de registros removidos no cabecalho
            cabecalho.nroPares++;  // atualiza o numero de pares no cabecalho
        }
    }
    // Atualiza os valores do cabecalho que foram alterados
    cabecalho.status = '1';
    fseek(binario, 0, SEEK_SET);
     escreve_cabecalho(binario,&cabecalho);
   
    fclose(binario);
    BinarioNaTela(arquivo_binario); // chama a funcao para imprimir o arquivo binario na tela
}

// aqui se encerra a funcionalidade 6
//_______________________________


void atualizacao_registros()
{
    // declara e le o nome do arquivo binario a ser utilizado
    char arquivo_binario[50];
    scanf("%s", arquivo_binario);

    Registro Reg; // struct para o registro a ser lido
    RegCabecalho cab; // definicao da struct para escrever o status do cabecalho

    // abre o arquivo para leitura e escrita e verifica se está corrompido
    FILE *binario = verificar_arquivo(arquivo_binario,"rb+");
    if (binario == NULL) // Se encontrar algum erro, ele retorna
    {
        return;
    }
    cab.status = '0'; // definicao de status = 0 para indicar que sera escrito no arquivo
    fseek(binario,0,SEEK_SET);
    fwrite(&cab.status,sizeof(char),1,binario);
    
    int n; 
    scanf("%d",&n); // declara e le do teclado quantas vezes sera feita a busca e a atualizacao de registros

    for (int i = 0; i< n;i++)
    {
        int m; // declara e le quantos campos serao utilizados na busca
        scanf("%d",&m);
        Criterios busca = ler_criterios(m); // chama a funcao que vai ler e guardar os campos e valores a serem buscados
        int p;
        scanf("%d",&p); // le quantos campos serao atualizados
        Criterios atualizacao = ler_criterios(p); // le os valores que vao ser escritos no arquivo
            
        fseek(binario,17,SEEK_SET);
        while (Leitura_Registro(binario,&Reg)==1) // leitura do arquivo
        {
            if (Reg.removido == '1') continue; // se o arquivo estiver removido, ele pula para o proximo
            
            int controle = 1; // variavel para ver se e o registro correto
            // ifs para verificar se o arquivo identificado na busca é o correto
            if(busca.idPoPs != -2 && busca.idPoPs != Reg.idPoPs ) controle = 0; 
            if(busca.idPoPsConectado != -2 && busca.idPoPsConectado != Reg.idPoPsConectado ) controle = 0;
            if(busca.velocidade != -2 && busca.velocidade != Reg.velocidade ) controle = 0;
            if(busca.unidadeMedida != -2 && busca.unidadeMedida != Reg.unidade_medida ) controle = 0;

            if(controle == 1) 

            {   // se o arquivo for o correto, atualiza os valores lidos na variavel p
                if(atualizacao.idPoPs != -2)
                {
                    Reg.idPoPs = atualizacao.idPoPs;
                }
                if(atualizacao.idPoPsConectado != -2)
                {
                    Reg.idPoPsConectado = atualizacao.idPoPsConectado;
                }
                if(atualizacao.velocidade != -2)
                {
                    Reg.velocidade = atualizacao.velocidade;
                }
                if(atualizacao.unidadeMedida != -2)
                {
                    Reg.unidade_medida = atualizacao.unidadeMedida;
                }
                // Como o registro foi lido ate o final, deve-se retornar 18 bytes para escrever no inicio do registro
                fseek(binario,-18,SEEK_CUR);
                escreve_arquivo(binario,&Reg); // utiliza a funcao para escrever no arquivo
                fseek(binario,0,SEEK_CUR); // fseek necessario, pois apos a escrita no arquivo
                                           // ele volta la no while para ler novos dados     
            }
        }
        }
        // Atualiza o status do arquivo para 1 apos realizar a funcionalidade
        cab.status = '1';
        fseek(binario,0,SEEK_SET);
        fwrite(&cab.status,sizeof(char),1,binario);
        fclose(binario);
        BinarioNaTela(arquivo_binario); 
    }

