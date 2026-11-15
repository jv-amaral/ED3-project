// coloquei esse nome so para começarmos a fazer as funcoes novas aqui
// depois ajustamos onde cada uma melhor se encaixa

#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionalidades2.h"
#include "funcoes_base2.h"

// Funcionalidade 8

void create_table(void)
{
    char nome_arq_entrada[256];
    char nome_arq_saida[256];

    if (scanf("%255s %255s", nome_arq_entrada, nome_arq_saida) != 2 ||
        strcmp(nome_arq_entrada, nome_arq_saida) == 0)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *arq_csv = fopen(nome_arq_entrada, "r");

    if (arq_csv == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // A linha cresce conforme necessario, sem truncar nomes de tamanho variavel.
    char *linha_csv = NULL;
    size_t capacidade = 0;
    char *campos[5]; // Quatro campos de dados e uma possivel coluna de numeracao.
    const char *nomes_campos[] = {"idPoPs", "nomePoPs", "nomePais", "siglaPais"};
    int colunas[4] = {-1, -1, -1, -1};
    int qtd_colunas = 0;
    int sucesso = ler_linha_csv(arq_csv, &linha_csv, &capacidade) == 1;

    // Descobre a ordem pelo cabecalho, inclusive quando ha uma coluna inicial extra.
    if (sucesso)
    {
        qtd_colunas = separar_campos_csv(linha_csv, campos, 5);
        for (int i = 0; i < qtd_colunas; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (strcmp(campos[i], nomes_campos[j]) == 0)
                    colunas[j] = i;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            if (colunas[i] == -1)
                sucesso = 0;
        }
    }

    if (!sucesso)
    {
        free(linha_csv);
        fclose(arq_csv);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *arq_binario = fopen(nome_arq_saida, "wb");
    if (arq_binario == NULL)
    {
        free(linha_csv);
        fclose(arq_csv);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    RegCabecalho cabecalho;
    cabecalho.status = '0';
    cabecalho.topoLista = -1;
    cabecalho.proxByteOffset = 0;
    cabecalho.nroRegRem = 0;
    cabecalho.nroPoPs = 0;

    escreve_cabecalho2(arq_binario, &cabecalho);
    // O primeiro byte disponivel, depois do cabecalho, e o de offset 25.
    cabecalho.proxByteOffset = 25;
    sucesso = !ferror(arq_binario) && fflush(arq_binario) == 0;

    int leitura = 0;
    while (sucesso && (leitura = ler_linha_csv(arq_csv, &linha_csv, &capacidade)) == 1)
    {
        if (separar_campos_csv(linha_csv, campos, 5) != qtd_colunas)
        {
            sucesso = 0;
            break;
        }

        Registro registro;
        registro.removido = '0';
        registro.encadeamentoLista = -1;

        // strtol permite verificar se o id realmente representa um inteiro de 4 bytes.
        char *fim_id;
        errno = 0;
        long id = strtol(campos[colunas[0]], &fim_id, 10);
        if (fim_id == campos[colunas[0]] || *fim_id != '\0' ||
            errno == ERANGE || id < INT_MIN || id > INT_MAX || id == -1)
        {
            sucesso = 0;
            break;
        }
        registro.idPoPs = (int)id;

        // Os nomes apontam para a linha e sao gravados antes da proxima leitura.
        registro.nomePoPs = campos[colunas[1]];
        registro.nomePais = campos[colunas[2]];
        char *sigla = campos[colunas[3]];
        if (sigla[0] != '\0' && strlen(sigla) != 2)
        {
            sucesso = 0;
            break;
        }
        memcpy(registro.siglaPais, sigla[0] == '\0' ? "$$" : sigla, 2);
        registro.siglaPais[2] = '\0'; // Terminador somente em memoria.

        // 19 bytes fixos + dois delimitadores + os bytes dos dois nomes.
        size_t tamanho = 21 + strlen(registro.nomePoPs) + strlen(registro.nomePais);
        if (tamanho > INT_MAX || cabecalho.nroPoPs == INT_MAX)
        {
            sucesso = 0;
            break;
        }
        registro.tamanhoRegistro = (int)tamanho;

        if (!escreve_registro2(arq_binario, &registro))
        {
            sucesso = 0;
            break;
        }

        cabecalho.proxByteOffset += registro.tamanhoRegistro;
        cabecalho.nroPoPs++;
    }

    if (leitura == -1)
        sucesso = 0;
    if (fclose(arq_csv) != 0)
        sucesso = 0;
    free(linha_csv);

    // Atualiza o cabecalho ainda inconsistente e descarrega os dados antes do status 1.
    if (sucesso)
    {
        sucesso = fseek(arq_binario, 0, SEEK_SET) == 0;
        if (sucesso)
        {
            escreve_cabecalho2(arq_binario, &cabecalho);
            sucesso = !ferror(arq_binario) && fflush(arq_binario) == 0;
        }
    }
    if (sucesso)
    {
        cabecalho.status = '1';
        sucesso = fseek(arq_binario, 0, SEEK_SET) == 0 &&
                  fwrite(&cabecalho.status, sizeof(char), 1, arq_binario) == 1;
    }
    if (fclose(arq_binario) != 0)
        sucesso = 0;

    if (sucesso)
        BinarioNaTela(nome_arq_saida);
    else
        printf("Falha no processamento do arquivo.\n");
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
