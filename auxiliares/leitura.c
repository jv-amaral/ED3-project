#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "leitura.h"

// auxiliares compartilhadas pelos trabalhos 1 e 2

// le strings entre aspas ou nulo para um buffer reservado pelo chamador.
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

// le ate a quebra de linha, aumentando o espaco do buffer quando precisa.
// devolve 1 para uma linha lida, 0 no fim do csv ou -1 se faltar memoria.
// atualiza linha e capacidade; quem chama libera o buffer ao terminar as leituras.
int ler_linha_csv(FILE *csv, char **linha, size_t *capacidade)
{
    size_t tamanho = 0;
    // quando da linha atual eu ja li
    // capacidade vai ser o espaco do buffer
    int caractere;

    // primeira chamada vai alocar espaco
    while ((caractere = fgetc(csv)) != EOF)
    {
        // reserva espaco tambem para o terminador '\0' usado em memoria.
        if (tamanho + 1 >= *capacidade)
        {
            size_t nova_capacidade = *capacidade == 0 ? 128 : *capacidade * 2;
            char *nova_linha = realloc(*linha, nova_capacidade);
            if (nova_linha == NULL)
                return -1;
            // realloc pode mudar o endereco, entao atualiza o ponteiro usado pelo chamador
            *linha = nova_linha;
            *capacidade = nova_capacidade;
        }
        if (caractere == '\n')
            break;
        (*linha)[tamanho++] = (char)caractere;
    }

    if (caractere == EOF && tamanho == 0)
        return 0;
    // aceita tanto a quebra de linha do linux (lf) quanto a do windows (crlf).
    if (tamanho > 0 && (*linha)[tamanho - 1] == '\r')
        tamanho--;
    (*linha)[tamanho] = '\0';
    return 1;
}

// troca as virgulas por terminadores e guarda o inicio de cada campo.
// devolve a quantidade de colunas, ou -1 se passar do limite; os ponteiros usam a propria linha.
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
        // mesmo se o proximo trecho estiver vazio, ele ainda conta como um campo
        inicio = virgula + 1;
    }
    return -1; // mais colunas do que o esperado.
}

// le um valor do terminal, com ou sem aspas. nulo sem aspas vira string vazia.
// quem chama libera a string devolvida; retorna null se faltar memoria.
// usado na funcao 12 e 13
char *ler_valor(void)
{
    int caractere;
    do
    {
        caractere = getchar();
    } while (caractere != EOF && isspace((unsigned char)caractere));

    // dentro de aspas, os espacos fazem parte do valor e nao encerram a leitura
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

        // mantem espaco para o terminador enquanto o texto cresce
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

    // nulo entre aspas e texto; sem aspas representa um campo vazio
    valor[tamanho] = '\0';
    if (!entre_aspas && strcmp(valor, "NULO") == 0)
        valor[0] = '\0';
    return valor;
}

// auxiliares trab. 1

// le a proxima conexao e devolve 0 quando nao encontra outro registro.
// a marca de removido faz pular os 17 bytes restantes com fseek.
int ler_conexao(FILE *arquivo, RegistroConexao *registro)
{
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

// monta uma conexao com os quatro valores da entrada.
// velocidade nula fica -1 e unidade nula fica com o caractere de lixo.
RegistroConexao ler_conexao_terminal(void)
{
    RegistroConexao registro;
    registro.removido = '0';
    registro.encadeamento = -1;
    scanf("%d %d", &registro.idPoPs, &registro.idPoPsConectado);
    // a velocidade e lida como texto para aceitar o numero ou nulo
    char *velocidade = ler_valor();
    char unidade[2];
    ScanQuoteString(unidade);

    registro.velocidade = velocidade[0] == '\0' ? -1 : atoi(velocidade);
    registro.unidade_medida = unidade[0] == '\0' ? '$' : unidade[0];
    // os valores ja foram copiados para os campos numericos e para a unidade
    free(velocidade);
    return registro;
}

// auxiliares trab. 2

// junta os caracteres ate encontrar a barra vertical e devolve o texto.
// o cursor passa pelo delimitador; quem chama libera o texto. retorna null se a leitura falhar.
static char *ler_campo_variavel(FILE *arquivo, int limite)
{
    char *texto = malloc(limite + 1);
    if (texto == NULL)
        return NULL;

    int tamanho = 0;
    int caractere;
    while (tamanho < limite && (caractere = fgetc(arquivo)) != EOF)
    {
        // a barra termina o campo no disco; na memoria, ela e substituida por zero
        if (caractere == '|')
        {
            texto[tamanho] = '\0';
            return texto;
        }
        texto[tamanho++] = (char)caractere;
    }

    // sem encontrar o delimitador, descarta a leitura incompleta
    free(texto);
    return NULL;
}

// le um pop da posicao atual; devolve 1 se leu ou pulou um registro e 0 no fim ou em falha.
// pula removidos pelo tamanho e termina no proximo registro; liberar_registro descarta os nomes lidos.
int ler_registro(FILE *arquivo, RegistroPoP *registro)
{
    // vai comecar na posicao atual do cursor

    // removidos nao recebem nomes alocados, mas podem passar pela rotina de liberacao
    registro->nomePoPs = NULL;
    registro->nomePais = NULL;

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

    // desconta a parte fixa para reservar espaco suficiente para ler os nomes
    int limite = registro->tamanhoRegistro - TAM_FIXO_REGISTRO;
    registro->nomePoPs = ler_campo_variavel(arquivo, limite);
    registro->nomePais = ler_campo_variavel(arquivo, limite);
    if (registro->nomePoPs == NULL || registro->nomePais == NULL)
    {
        liberar_registro(registro);
        return 0;
    }

    // pula tambem o lixo que possa existir no fim de um espaco reaproveitado
    int tamanho_lido = TAM_MINIMO_REGISTRO + strlen(registro->nomePoPs) + strlen(registro->nomePais);
    fseek(arquivo, registro->tamanhoRegistro - tamanho_lido, SEEK_CUR);
    return 1;
}

// libera os nomes guardados no registro e deixa os ponteiros vazios.
void liberar_registro(RegistroPoP *registro)
{
    free(registro->nomePoPs);
    free(registro->nomePais);
    // depois da liberacao, o registro nao deve continuar apontando para esses textos
    registro->nomePoPs = NULL;
    registro->nomePais = NULL;
}

// le id, nome do pop, pais e sigla do terminal e devolve o registro pronto para inserir.
// os nomes usam memoria alocada por ler_valor e devem ser liberados depois da gravacao.
RegistroPoP ler_pop_terminal(void)
{
    RegistroPoP registro;
    scanf("%d", &registro.idPoPs);
    registro.nomePoPs = ler_valor();
    registro.nomePais = ler_valor();
    // a sigla tem dois caracteres; o terceiro byte fica para o terminador
    char sigla[3];
    ScanQuoteString(sigla);

    // a sigla ocupa dois bytes; os nomes vazios serao gravados apenas com o delimitador
    memcpy(registro.siglaPais, sigla[0] == '\0' ? "$$" : sigla, 2);
    registro.siglaPais[2] = '\0';

    registro.removido = '0';
    registro.encadeamentoLista = -1;
    registro.tamanhoRegistro = TAM_MINIMO_REGISTRO + strlen(registro.nomePoPs) + strlen(registro.nomePais);
    return registro;
}
