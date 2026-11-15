#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funcionalidades2.h"
#include "funcoes_base2.h"

// Retorna 1 para uma linha, 0 no fim do arquivo e -1 em erro de leitura/alocacao.
int ler_linha_csv(FILE *csv, char **linha, size_t *capacidade)
{
    size_t tamanho = 0;
    int caractere;

    while ((caractere = fgetc(csv)) != EOF)
    {
        // Reserva espaco tambem para o terminador '\0' usado em memoria.
        if (tamanho + 1 >= *capacidade)
        {
            if (*capacidade > SIZE_MAX / 2)
                return -1;
            size_t nova_capacidade = *capacidade == 0 ? 128 : *capacidade * 2;
            char *nova_linha = realloc(*linha, nova_capacidade);
            if (nova_linha == NULL)
                return -1;
            *linha = nova_linha;
            *capacidade = nova_capacidade;
        }
        if (caractere == '\n')
            break;
        (*linha)[tamanho++] = (char)caractere;
    }

    if (ferror(csv))
        return -1;
    if (caractere == EOF && tamanho == 0)
        return 0;
    // Aceita tanto a quebra de linha do Linux (LF) quanto a do Windows (CRLF).
    if (tamanho > 0 && (*linha)[tamanho - 1] == '\r')
        tamanho--;
    (*linha)[tamanho] = '\0';
    return 1;
}

// Separa o CSV simples da disciplina sem pular campos vazios, inclusive o ultimo.
// Os ponteiros resultantes pertencem a linha; nao devem ser liberados separadamente.
int separar_campos_csv(char *linha, char *campos[], int max_campos)
{
    int quantidade = 0;
    char *inicio = linha;

    while (quantidade < max_campos)
    {
        campos[quantidade++] = inicio;
        char *virgula = strchr(inicio, ',');
        if (virgula != NULL)
            *virgula = '\0';

        // Um campo vazio ou composto apenas de espacos representa um valor nulo.
        char *caractere = inicio;
        while (isspace((unsigned char)*caractere))
            caractere++;
        if (*caractere == '\0')
            inicio[0] = '\0';

        if (virgula == NULL)
            return quantidade;
        inicio = virgula + 1;
    }
    return -1; // Mais colunas do que o esperado.
}

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

/*RegCabecalho Leitura_Cabecalho2(FILE *binario)

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
}*/

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

// Grava campo a campo: nao inclui padding da struct nem terminadores '\0'.
// Retorna 0 se alguma escrita falhar e 1 se o registro inteiro for gravado.
int escreve_registro2(FILE *binario, const Registro *Reg)
{
    char delimitador = '|';
    size_t tamanho_nome = strlen(Reg->nomePoPs);
    size_t tamanho_pais = strlen(Reg->nomePais);

    return fwrite(&Reg->removido, sizeof(char), 1, binario) == 1 &&
           fwrite(&Reg->tamanhoRegistro, sizeof(int), 1, binario) == 1 &&
           fwrite(&Reg->encadeamentoLista, sizeof(int64_t), 1, binario) == 1 &&
           fwrite(&Reg->idPoPs, sizeof(int), 1, binario) == 1 &&
           fwrite(Reg->siglaPais, sizeof(char), 2, binario) == 2 &&
           fwrite(Reg->nomePoPs, sizeof(char), tamanho_nome, binario) == tamanho_nome &&
           fwrite(&delimitador, sizeof(char), 1, binario) == 1 &&
           fwrite(Reg->nomePais, sizeof(char), tamanho_pais, binario) == tamanho_pais &&
           fwrite(&delimitador, sizeof(char), 1, binario) == 1;
}



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
