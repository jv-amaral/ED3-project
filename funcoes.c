//arquivo funcoes


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"



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

    // abre os arquivos para leitura e escrita
    
    FILE *csv = fopen(arquivo_csv, "r");
    FILE *binario = fopen(arquivo_binario, "wb"); 
    // verifica se os arquivos nao sao nulos
    if (csv == NULL || binario == NULL) {
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
    fwrite(&cabecalho.topo_pilha , sizeof(int), 1,binario);
    fwrite(&cabecalho.proxRRN , sizeof(int), 1, binario);
    fwrite(&cabecalho.nroRegRem , sizeof(int), 1, binario);
    fwrite(&cabecalho.nroPares , sizeof(int), 1, binario);


    // atribuicao inicial de valores aos campos do registro
    Reg.removido = '0';
    Reg.encadeamento = -1;
    char linha_csv[100];
        fgets(linha_csv, sizeof(linha_csv),csv ); // le a primeira linha do arquivo csv e descarta, porque é so os titulos dos campos

    while((fgets(linha_csv, sizeof(linha_csv),csv )) != NULL)
    {    
        
        //atribui os valores do csv para os campos do registro
        Reg.idPoPs = atoi(strtok(linha_csv, ","));
        Reg.idPoPsConectado = atoi(strtok(NULL, ","));


        char *velocidade_ptr = strtok(NULL, ",");
        // verifica se o campo velocidade é nulo e atribui -1 se for o caso, caso contrário, o valor lido do csv é copiado para a variável Reg.velocidade
        if (velocidade_ptr == NULL || velocidade_ptr[0] == ' ')
         {
            Reg.velocidade = -1; // atribui -1 se o campo for nulo
        } else 
        {
            Reg.velocidade = atoi(velocidade_ptr);
        }


        char *unidade_medida_ptr = strtok(NULL, ",\n\r");
        // verifica se o campo de unidade de medida é nulo e atribui '$' para ser usado como lixo 
        if (unidade_medida_ptr == NULL || unidade_medida_ptr[0] == ' ')
        {
            Reg.unidade_medida = '$'; 
        } else 
        {
            Reg.unidade_medida = unidade_medida_ptr[0];
        }
    


        //escreve os campos do registro no arquivo binário

        fwrite(&Reg.removido , sizeof(char), 1, binario);
        fwrite(&Reg.encadeamento, sizeof(int), 1, binario);
        fwrite(&Reg.idPoPs, sizeof(int), 1, binario);
        fwrite(&Reg.idPoPsConectado, sizeof(int), 1, binario);
        fwrite(&Reg.velocidade, sizeof(int), 1, binario);
        fwrite(&Reg.unidade_medida, sizeof(char), 1, binario);

        //atualiza os valores do cabeçalho do proxRRN e do numero de pares
        cabecalho.proxRRN++;
        cabecalho.nroPares++;
        
    }
    // atualiza o status para 1 e grava no byteoffset 0 do arquivo binario
    cabecalho.status = '1';
    fseek(binario,0,SEEK_SET); 
    fwrite(&cabecalho.status , sizeof(char), 1, binario);



    // grava no byteoffset 5 o valor atualizado do proxRRN
    fseek(binario,5,SEEK_SET);
    fwrite(&cabecalho.proxRRN , sizeof(int), 1, binario);

    // grava no byteoffset 13 o valor atualizado do nroPares
    fseek(binario,13,SEEK_SET);
    fwrite(&cabecalho.nroPares , sizeof(int), 1, binario);

    fclose(csv);
    fclose(binario);
    BinarioNaTela(arquivo_binario);
    
}


void recuperacao_dados() //funcao para recuperar os dados do arquivo binario e imprimir na tela
{
    // declara a variavel para armazenar o nome do arquivo e o lê
    char arquivo_binario[50];
    scanf("%s", arquivo_binario);

    RegCabecalho cabecalho; // definicao das structs  para a segunda funcao
    Registro Reg; 

    // abre o arquivo para leitura
    FILE *binario = fopen(arquivo_binario, "rb");
    if (binario == NULL) {
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

    while(fread(&Reg.removido, sizeof(char), 1, binario) == 1) // se o registro nao estiver removido, ele sera lido
    {
        if (Reg.removido == '1') // se o registro estiver removido, ele é ignorado
        {
            fseek(binario, 17, SEEK_CUR); // pula para o proximo registro
            continue;
        }
        registros_lidos++;
        // leitura dos bytes do arquivo binario para os campos do registro
        fread(&Reg.encadeamento, sizeof(int),1,binario);
        fread(&Reg.idPoPs, sizeof(int),1,binario);
        fread(&Reg.idPoPsConectado, sizeof(int),1,binario);
        fread(&Reg.velocidade, sizeof(int),1,binario);
        fread(&Reg.unidade_medida, sizeof(char),1,binario);
        // atribuicao dos valores nulos na hora de printar na tela 
        if( Reg.velocidade == -1)
        {
            printf("%d  %d  %s  \"%c\"\n", Reg.idPoPs, Reg.idPoPsConectado, "NULO", Reg.unidade_medida);
        }
        else if(Reg.unidade_medida == '$')
        {
            printf("%d  %d  %d  \"%s\"\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.velocidade, "NULO");
        }
        else{// caso nao seja nenhum valor nulo, printa normalmente
        printf("%d  %d  %d  \"%c\"\n", Reg.idPoPs, Reg.idPoPsConectado, Reg.velocidade, Reg.unidade_medida);
        }

}
        if(registros_lidos == 0) // printa na tela caso nenhum registro tenha sido lido
        {
            printf("Registro inexistente.\n");
        }
    fclose(binario);


    }












/*
 * Você não precisa entender o código dessa função.
 *
 * Use essa função para comparação no run.codes.
 * Lembre-se de ter fechado (fclose) o arquivo anteriormente.
 *
 * Ela vai abrir de novo para leitura e depois fechar
 * (você não vai perder pontos por isso se usar ela).
 */
void BinarioNaTela(char *arquivo) {
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
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
    for (unsigned long i = 0; i < fl; i++) {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

/*
 *	Use essa função para ler um campo string delimitado entre aspas (").
 *	Chame ela na hora que for ler tal campo. Por exemplo:
 *
 *	A entrada está da seguinte forma:
 *		nomeDoCampo "MARIA DA SILVA"
 *
 *	Para ler isso para as strings já alocadas str1 e str2 do seu programa,
 * você faz: scanf("%s", str1); // Vai salvar nomeDoCampo em str1
 *		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2
 * (sem as aspas)
 *
 */
void ScanQuoteString(char *str) {
    char R;

    while ((R = getchar()) != EOF && isspace(R))
        ; // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar();         // ignorar aspas fechando
    } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre
                           // aspas! Fazer leitura normal %s então, pois deve
                           // ser algum inteiro ou algo assim...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}