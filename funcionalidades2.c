// coloquei esse nome so para começarmos a fazer as funcoes novas aqui
// depois ajustamos onde cada uma melhor se encaixa

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionalidades2.h"
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
    scanf("%s", arquivo_binario); // le o nome dos arquivos
    scanf("%s",arquivo_index);
      // declaracao da struct do registro
    Registro reg;

     FILE *binario = verificar_arquivo(arquivo_binario, "rb");
    if (binario == NULL) // Se encontrar algum erro, ele retorna
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fseek(binario, 25, SEEK_SET);
    int64_t byteOffsetAtual = 25; // byteoffset atual depois de ler o registro de cabecalho
     int max_registros = 2000; 
    RegIndex *vetor_indices = (RegIndex *) malloc(max_registros * sizeof(RegIndex)); // vetor para armazenar idPoPs e o byteoffset
     int i = 0;                                                                      // para ordenar depois
    while( fread(&reg.removido, sizeof(char), 1, binario)==1 )
    {
        fread(&reg.tamanhoRegistro,sizeof(int),1,binario);
        if(reg.removido == '0')
        {   
            fread(&reg.encadeamentoLista, sizeof(int64_t),1,binario);
            fread(&reg.idPoPs, sizeof(int),1,binario); 
            vetor_indices[i].idPoPs = reg.idPoPs;
            vetor_indices[i].byteoffset = byteOffsetAtual;
            i++;
            fseek(binario, reg.tamanhoRegistro - 17,SEEK_CUR);
        } else 
        {
            fseek(binario, reg.tamanhoRegistro - 5 ,SEEK_CUR);
        }
       
        byteOffsetAtual = byteOffsetAtual + reg.tamanhoRegistro;

    }
        

    qsort(vetor_indices, i, sizeof(RegIndex), compara_index);
     fclose(binario);

    FILE *index = fopen(arquivo_index,"wb");
    if(index == NULL)
    {
        printf("Falha no processamento do arquivo.\n"); // Mensagem padrão de erro
        free(vetor_indices);
        return;
    }
    char status_index = '0';
    fwrite(&status_index,sizeof(char),1,index);
    for ( int j = 0; j<i;j++)
    {
        fwrite(&vetor_indices[j].idPoPs,sizeof(int),1,index);
        fwrite(&vetor_indices[j].byteoffset,sizeof(int64_t),1,index);
    }
    status_index ='1';
    fseek(index,0,SEEK_SET);
    fwrite(&status_index,sizeof(char),1,index);
    fclose(index);
    free(vetor_indices);
    BinarioNaTela(arquivo_index);
}



//funcionalidade 11 (Recuperar os dados de um arquivo binario
 //                   via busca, utilizando indice caso seja requisitado)

void busca()
{
    char arquivo_binario[50]; // variaveis dos nomes dos arquivos
    char arquivo_index[50];

    scanf("%s",arquivo_binario);    //leitura dos nomes dos arquivos
    scanf("%s",arquivo_index);
    RegCabecalho cab;
    Registro reg;           // criacao das structs de registro e de registro de indice
    
    FILE *binario = verificar_arquivo(arquivo_binario,"rb");     // verifica se os arquivos nao estao corrompidos
    FILE *index = verificar_arquivo(arquivo_index,"rb");
    if (binario == NULL) // Se encontrar algum erro, ele retorna
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    if (index == NULL) // Se encontrar algum erro, ele retorna
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fread(&cab.status,sizeof(char),1,binario);

    if (cab.status == '0')                    // verifica se o arquivo esta consistente
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    char status_index;
    fread(&status_index,sizeof(char),1,index);

    if (status_index == '0')            // verifica se o arquivo de indice esta consistente
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
        // cria o vetor de indices para fazer a busca
       int max_indices = 2000;
         RegIndex *vetor_indices = (RegIndex *) malloc(max_indices * sizeof(RegIndex));
         int total_indices = 0;

         // Enquanto conseguir ler um idPoPs (4 bytes), é porque tem um registro
         while (fread(&vetor_indices[total_indices].idPoPs, sizeof(int), 1, index) == 1)
        {
        // Se leu o ID, obrigatoriamente tem o byteoffset (8 bytes) logo na frente
        fread(&vetor_indices[total_indices].byteoffset, sizeof(int64_t), 1, index);
        total_indices++;
        }

    
         fclose(index);
    int n;          // declaracao e leitura da variavel que vai verificar quantas vezes sera feita a busca
    scanf("%d",&n);

    

    for (int i = 0; i<n;i++)            // loop principal da busca que sera feita n vezes
    {   
        int m;
        scanf("%d",&m);
        Criterios C = ler_criterios(m);
        int registro_encontrado;
        if (C.idPoPs!= -2)
        {
            // (Isso entra logo depois do seu if que checou o status_index == '0')
        

     
        int inicio = 0;
        int fim = total_indices - 1;
        int64_t offset_encontrado = -1;
            while (inicio <= fim) {
                int meio = (inicio + fim) / 2;

                if (vetor_indices[meio].idPoPs == C.idPoPs) {
                    offset_encontrado = vetor_indices[meio].byteoffset; 
                    break; // Sai do while na hora
                } 
                else if (vetor_indices[meio].idPoPs < C.idPoPs) {
                    inicio = meio + 1;
                } 
                else {
                    fim = meio - 1;
                }
            }
            if(offset_encontrado == -1)
            {
                printf("Registro inexistente.\n");
            }
            else
            {
                fseek(binario,offset_encontrado,SEEK_SET);
                Leitura_Registro2(binario,&reg);
                if (reg.removido == '0' && verificar_encontro(&C, &reg) == 1)
                {
                    print_registro2(&reg);
                    registro_encontrado = 1;
                }
                
                if (registro_encontrado == 0) 
                {
                    printf("Registro inexistente.\n");
                }
               
            }
            


        }
        else
        {
            fseek(binario,25,SEEK_SET);

           while (Leitura_Registro2(binario, &reg) == 1)
            {
                if (reg.removido == '1') continue;
                
                if (verificar_encontro(&C, &reg) == 1)
                {
                    print_registro2(&reg);
                    registro_encontrado = 1;
                }
            }

            if (registro_encontrado == 0)
            {
                printf("Registro inexistente.\n");
            }
        }
    }




}
