#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "leitura.h"

// auxiliares compartilhadas pelos trabalhos 1 e 2

// le uma linha do csv, aumentando o buffer e avancando o cursor ate a proxima linha.
// altera linha e capacidade; retorna 1 se leu, 0 no fim e -1 em falha.
int ler_linha_csv(FILE *csv, char **linha, size_t *capacidade)
{
    size_t tamanho = 0;
    int caractere;

    while ((caractere = fgetc(csv)) != EOF)
    {
        // reserva espaco tambem para o terminador '\0' usado em memoria.
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
    // aceita tanto a quebra de linha do linux (lf) quanto a do windows (crlf).
    if (tamanho > 0 && (*linha)[tamanho - 1] == '\r')
        tamanho--;
    (*linha)[tamanho] = '\0';
    return 1;
}

// separa o csv simples sem pular campos vazios, inclusive o ultimo.
// altera a linha e preenche os ponteiros; retorna a quantidade de campos ou -1.
// os ponteiros pertencem a linha; nao devem ser liberados separadamente.
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

        // um campo vazio ou composto apenas de espacos representa um valor nulo.
        char *caractere = inicio;
        while (isspace((unsigned char)*caractere))
            caractere++;
        if (*caractere == '\0')
            inicio[0] = '\0';

        if (virgula == NULL)
            return quantidade;
        inicio = virgula + 1;
    }
    return -1; // mais colunas do que o esperado.
}

// le da entrada padrao um valor com ou sem aspas e retorna a string alocada.
// converte nulo sem aspas em string vazia; quem chama deve liberar o resultado.
char *ler_valor(void)
{
    int caractere;
    do
    {
        caractere = getchar();
    } while (caractere != EOF && isspace((unsigned char)caractere));

    int entre_aspas = caractere == '"';
    if (entre_aspas)
        caractere = getchar();

    int capacidade = 64;
    int tamanho = 0;
    char *valor = malloc(capacidade);
    if (valor == NULL)
        return NULL;

    while (caractere != EOF)
    {
        if ((entre_aspas && caractere == '"') ||
            (!entre_aspas && isspace((unsigned char)caractere)))
            break;

        if (tamanho + 1 == capacidade)
        {
            capacidade *= 2;
            char *novo = realloc(valor, capacidade);
            if (novo == NULL)
            {
                free(valor);
                return NULL;
            }
            valor = novo;
        }
        valor[tamanho++] = (char)caractere;
        caractere = getchar();
    }

    valor[tamanho] = '\0';
    if (!entre_aspas && strcmp(valor, "NULO") == 0)
        valor[0] = '\0';
    return valor;
}

// auxiliares trab. 1

// le uma conexao e deixa o cursor no proximo registro fixo; retorna 1 ou 0 no fim.
// em removidos, le somente a marca e pula os outros 17 bytes com fseek.
int ler_conexao(FILE *arquivo, RegistroConexao *registro)
{
    *registro = (RegistroConexao){0};
    if (fread(&registro->removido, sizeof(char), 1, arquivo) != 1)
        return 0;

    if (registro->removido == '1')
    {
        fseek(arquivo, TAM_REGISTRO_CONEXAO - 1, SEEK_CUR);
        return 1;
    }

    fread(&registro->encadeamento, sizeof(int), 1, arquivo);
    fread(&registro->idPoPs, sizeof(int), 1, arquivo);
    fread(&registro->idPoPsConectado, sizeof(int), 1, arquivo);
    fread(&registro->velocidade, sizeof(int), 1, arquivo);
    fread(&registro->unidade_medida, sizeof(char), 1, arquivo);
    return 1;
}

// le os quatro campos de uma conexao pela entrada padrao e retorna o registro.
// converte nulos, inicializa os controles e libera os textos temporarios lidos.
RegistroConexao ler_conexao_terminal(void)
{
    RegistroConexao registro;
    registro.removido = '0';
    registro.encadeamento = -1;
    scanf("%d %d", &registro.idPoPs, &registro.idPoPsConectado);
    char *velocidade = ler_valor();
    char *unidade = ler_valor();

    registro.velocidade = velocidade[0] == '\0' ? -1 : atoi(velocidade);
    registro.unidade_medida = unidade[0] == '\0' ? '$' : unidade[0];
    free(velocidade);
    free(unidade);
    return registro;
}

// Auxiliares trab. 2

// le um campo terminado por barra vertical e retorna a string alocada.
// avanca o cursor ate depois do delimitador; quem chama deve liberar a string.
static char *ler_campo_variavel(FILE *arquivo, int limite)
{
    char *texto = malloc(limite + 1);
    if (texto == NULL)
        return NULL;

    int tamanho = 0;
    int caractere;
    while (tamanho < limite && (caractere = fgetc(arquivo)) != EOF)
    {
        if (caractere == '|')
        {
            texto[tamanho] = '\0';
            return texto;
        }
        texto[tamanho++] = (char)caractere;
    }

    free(texto);
    return NULL;
}

// le o registro da posicao atual e deixa o cursor no inicio do proximo.
// retorna 1 se leu e 0 no fim ou em falha; registros removidos sao pulados com fseek.
// nos ativos, aloca os nomes, que precisam ser liberados com liberar_registro.
int ler_registro(FILE *arquivo, RegistroPoP *registro)
{
    int64_t inicio = ftell(arquivo);
    *registro = (RegistroPoP){0};

    if (fread(&registro->removido, sizeof(char), 1, arquivo) != 1)
        return 0;
    if (fread(&registro->tamanhoRegistro, sizeof(int), 1, arquivo) != 1)
        return 0;

    // apenas os dois primeiros campos sao lidos antes de pular um removido
    if (registro->removido == '1')
    {
        fseek(arquivo, registro->tamanhoRegistro - TAM_CONTROLE_INICIAL, SEEK_CUR);
        return 1;
    }

    fread(&registro->encadeamentoLista, sizeof(int64_t), 1, arquivo);
    fread(&registro->idPoPs, sizeof(int), 1, arquivo);
    fread(registro->siglaPais, sizeof(char), 2, arquivo);
    registro->siglaPais[2] = '\0';

    int limite = registro->tamanhoRegistro - TAM_FIXO_REGISTRO;
    if (limite < 2)
        return 0;
    registro->nomePoPs = ler_campo_variavel(arquivo, limite);
    registro->nomePais = ler_campo_variavel(arquivo, limite);
    if (registro->nomePoPs == NULL || registro->nomePais == NULL)
    {
        liberar_registro(registro);
        return 0;
    }

    // pula tambem o lixo que possa existir no fim de um espaco reaproveitado
    fseek(arquivo, inicio + registro->tamanhoRegistro, SEEK_SET);
    return 1;
}

// libera os dois nomes alocados durante a leitura; nao altera o cursor do arquivo.
// nao retorna valor e tambem pode receber um registro removido, com nomes nulos.
void liberar_registro(RegistroPoP *registro)
{
    free(registro->nomePoPs);
    free(registro->nomePais);
    registro->nomePoPs = NULL;
    registro->nomePais = NULL;
}
