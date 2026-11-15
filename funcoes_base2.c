#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funcionalidades2.h"
#include "funcoes_base2.h"

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

    //verifica se o fread consiguiu ler o byte antes de comparar o seu valor
    if (fread(&temp_status, sizeof(char), 1, binario) != 1 || temp_status != '1')
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(binario);
        return NULL;
    }

    return binario;
}

RegCabecalho Leitura_Cabecalho2(FILE *binario)
{
    RegCabecalho cabecalho; // define o nome da struct
    // Le todos os dados do registro de cabecalho
    fseek(binario, 0, SEEK_SET);
    fread(&cabecalho.status, sizeof(char), 1, binario);
    fread(&cabecalho.topoLista, sizeof(int64_t), 1, binario);
    fread(&cabecalho.proxByteOffset, sizeof(int64_t), 1, binario);
    fread(&cabecalho.nroRegRem, sizeof(int), 1, binario);
    fread(&cabecalho.nroPoPs, sizeof(int), 1, binario);

    return cabecalho;
}

// funcao para fazer a leitura de um registro completo
// fazendo a verificao de quando o arquivo se encerra tambem


Criterios ler_criterios(int qtd_criterios)
{

    Criterios C; // inicia a struct dos criterios

    // inicio os criterios com valor -2, ja que 0 é um valor de busca valido e -1 corresponde ao nulo
    C.idPoPs = -2;
    strcpy(C.siglaPais, "NUL");
    strcpy(C.nomePoPs, "NUL");
    strcpy(C.nomePais, "NUL");

    

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

        else if (strcmp(campo, "siglaPais") == 0)
        {
            ScanQuoteString(valor);
            strcpy(C.siglaPais, valor); // Se for NULO, a ScanQuoteString retorna ""
        }
        else if (strcmp(campo, "nomePoPs") == 0)
        {
            ScanQuoteString(valor);
            strcpy(C.nomePoPs, valor);
        }
        else if (strcmp(campo, "nomePais") == 0)
        {
            ScanQuoteString(valor);
            strcpy(C.nomePais, valor);
        }

        
    }
    return C;
}
int verificar_encontro(Criterios *C, Registro *Reg)
{
    //essa funcao vai retornar se os criterios desejados foram encontrados nos registros ou nao
    int encontro = 1;

    if (C->idPoPs != -2 && C->idPoPs != Reg->idPoPs) encontro = 0;
    if (strcmp(C->nomePoPs,"NUL")!=0 && strcmp(C->nomePoPs,Reg->nomePoPs)!=0)  encontro = 0;
    if (strcmp(C->siglaPais,"NUL")!=0 && strcmp(C->siglaPais,Reg->siglaPais)!=0)  encontro = 0;
    if (strcmp(C->nomePais,"NUL")!=0 && strcmp(C->nomePais,Reg->nomePais)!=0)  encontro = 0;
        
    return encontro;
}
RegIndex *criar_vetor(FILE *index, int *total_indices)
{
    int max_indices = 2000;
    RegIndex *vetor_indices = (RegIndex *) malloc(max_indices * sizeof(RegIndex));
    *total_indices = 0;

    // Garante que o cursor está logo após o byte de status (posição 1)
    fseek(index, 1, SEEK_SET);

    // Enquanto conseguir ler um idPoPs (4 bytes), é porque tem um registro
    while (fread(&vetor_indices[*total_indices].idPoPs, sizeof(int), 1, index) == 1)
    {
        fread(&vetor_indices[*total_indices].byteoffset, sizeof(int64_t), 1, index);
        (*total_indices)++;
    }

    return vetor_indices;
}
void print_registro2(Registro *Reg)
{
    printf("%d ", Reg->idPoPs);

    if (Reg->nomePoPs[0] == '\0' || Reg->nomePoPs[0] == '|')
    {
        printf("NULO ");
    }
    else
    {
        printf("%s ", Reg->nomePoPs);
    }

    if (Reg->nomePais[0] == '\0' || Reg->nomePais[0] == '|')
    {
        printf("NULO ");
    }
    else
    {
        printf("%s ", Reg->nomePais);
    }

    if (Reg->siglaPais[0] == '$' || Reg->siglaPais[0] == '\0')
    {
        printf("NULO\n");
    }
    else
    {
        printf("%.2s\n", Reg->siglaPais);
    }
}

int Leitura_Registro2(FILE *binario, Registro *Reg)
{
    // Lê o campo removido. Se não ler 1 byte, o arquivo acabou.
    if (fread(&Reg->removido, sizeof(char), 1, binario) != 1)
    {
        return 0;
    }

    fread(&Reg->tamanhoRegistro, sizeof(int), 1, binario);
    fread(&Reg->encadeamentoLista, sizeof(int64_t), 1, binario);
    fread(&Reg->idPoPs, sizeof(int), 1, binario);
    
    // siglaPais tem tamanho fixo de 2 bytes
    fread(Reg->siglaPais, sizeof(char), 2, binario);
    Reg->siglaPais[2] = '\0'; 

    
    int i = 0;
    char c;
    // Lê byte a byte até achar o delimitador '|'
    while (fread(&c, sizeof(char), 1, binario) == 1 && c != '|') 
    {
        Reg->nomePoPs[i] = c;
        i++;
    }
 
    Reg->nomePoPs[i] = '\0'; 
    i = 0;
    // Lê byte a byte até achar o delimitador '|'
    while (fread(&c, sizeof(char), 1, binario) == 1 && c != '|') 
    {
        Reg->nomePais[i] = c;
        i++;
    }
    // Adiciona o terminador de string
    Reg->nomePais[i] = '\0'; 

    return 1;
}

int compara_index(const void *a, const void *b) {
    RegIndex *regA = (RegIndex *)a;
    RegIndex *regB = (RegIndex *)b;
    
    return regA->idPoPs - regB->idPoPs;
}
void modifica_registro(Registro *reg,Criterios *NovosValores)
{
    if(NovosValores->idPoPs!=-2)
    {
        reg->idPoPs = NovosValores->idPoPs;
    }
    
    if(strcmp(NovosValores->nomePoPs,"NUL")!=0)
    {
         strcpy(reg->nomePoPs,NovosValores->nomePoPs);
    }

    if(strcmp(NovosValores->nomePais,"NUL")!=0)
    {
        strcpy(reg->nomePais, NovosValores->nomePais);
    }
    
    if(strcmp(NovosValores->siglaPais, "NUL") != 0)
    {
        // Se a sigla nova for "", significa que o usuário digitou NULO.
        // Como a sigla é tamanho fixo, gravamos '$$' no disco.
        if (strcmp(NovosValores->siglaPais, "") == 0)
         {

            reg->siglaPais[0] = '$';
            reg->siglaPais[1] = '$';
            reg->siglaPais[2] = '\0';
        } 
        else 
        {
            strcpy(reg->siglaPais, NovosValores->siglaPais);
        }
    }

  


}

// funcao para escrever os registros
/*void escreve_registro2(FILE *binario, Registro *Reg)
{
    fwrite(&Reg->removido, sizeof(char), 1, binario);
    fwrite(&Reg->encadeamento, sizeof(int), 1, binario);
    fwrite(&Reg->idPoPs, sizeof(int), 1, binario);
    fwrite(&Reg->idPoPsConectado, sizeof(int), 1, binario);
    fwrite(&Reg->velocidade, sizeof(int), 1, binario);
    fwrite(&Reg->unidade_medida, sizeof(char), 1, binario);
}*/



// funcao para escrever o cabecalho
void escreve_cabecalho2(FILE *binario, RegCabecalho *Cab)
{
    fwrite(&Cab->status, sizeof(char), 1, binario);
    fwrite(&Cab->topoLista, sizeof(int64_t), 1, binario);
    fwrite(&Cab->proxByteOffset, sizeof(int64_t), 1, binario);
    fwrite(&Cab->nroRegRem, sizeof(int), 1, binario);
    fwrite(&Cab->nroPoPs, sizeof(int), 1, binario);
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
