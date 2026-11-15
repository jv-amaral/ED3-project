#include <string.h>
#include "escrita.h"

// grava um registro campo a campo a partir do cursor atual, sem terminadores nulos.
// preenche a sobra com lixo e termina no proximo registro; retorna 1 ou 0 em falha.
int escrever_registro(FILE *arquivo, const RegistroPoP *registro)
{
    char delimitador = '|';
    int tamanho_nome = strlen(registro->nomePoPs);
    int tamanho_pais = strlen(registro->nomePais);

    fwrite(&registro->removido, sizeof(char), 1, arquivo);
    fwrite(&registro->tamanhoRegistro, sizeof(int), 1, arquivo);
    fwrite(&registro->encadeamentoLista, sizeof(int64_t), 1, arquivo);
    fwrite(&registro->idPoPs, sizeof(int), 1, arquivo);
    fwrite(registro->siglaPais, sizeof(char), 2, arquivo);
    fwrite(registro->nomePoPs, sizeof(char), tamanho_nome, arquivo);
    fwrite(&delimitador, sizeof(char), 1, arquivo);
    fwrite(registro->nomePais, sizeof(char), tamanho_pais, arquivo);
    fwrite(&delimitador, sizeof(char), 1, arquivo);

    int utilizado = TAM_MINIMO_REGISTRO + tamanho_nome + tamanho_pais;
    for (int i = utilizado; i < registro->tamanhoRegistro; i++)
        fputc('$', arquivo);

    return !ferror(arquivo);
}

// imprime os campos do registro na ordem do enunciado, com nulo quando necessario.
// nao retorna valor, nao altera o registro e nao movimenta arquivos.
void imprimir_registro(RegistroPoP *registro)
{
    printf("%d ", registro->idPoPs);
    printf("%s ", registro->nomePoPs[0] == '\0' ? "NULO" : registro->nomePoPs);
    printf("%s ", registro->nomePais[0] == '\0' ? "NULO" : registro->nomePais);
    printf("%s\n", registro->siglaPais[0] == '$' ? "NULO" : registro->siglaPais);
}

// grava os 18 bytes da conexao campo a campo e avanca para o proximo registro.
// nao retorna valor e nao grava o preenchimento interno da estrutura.
void escrever_conexao(FILE *arquivo, RegistroConexao *registro)
{
    fwrite(&registro->removido, sizeof(char), 1, arquivo);
    fwrite(&registro->encadeamento, sizeof(int), 1, arquivo);
    fwrite(&registro->idPoPs, sizeof(int), 1, arquivo);
    fwrite(&registro->idPoPsConectado, sizeof(int), 1, arquivo);
    fwrite(&registro->velocidade, sizeof(int), 1, arquivo);
    fwrite(&registro->unidade_medida, sizeof(char), 1, arquivo);
}

// exibe uma conexao com nulos e unidade entre aspas, no formato das opcoes 2 a 4.
// nao retorna valor, nao modifica o registro e nao movimenta arquivos.
void imprimir_conexao(RegistroConexao *registro)
{
    printf("%d %d ", registro->idPoPs, registro->idPoPsConectado);
    if (registro->velocidade == -1)
        printf("NULO ");
    else
        printf("%d ", registro->velocidade);

    if (registro->unidade_medida == '$')
        printf("NULO\n");
    else
        printf("\"%c\"\n", registro->unidade_medida);
}

// marca o registro, preenche seus dados com lixo e o inclui na lista worst fit.
// atualiza os contadores e o topo em memoria; nao retorna valor.
// usa fseek para percorrer a lista e volta ao cursor original antes de terminar.
void remover_registro(FILE *arquivo, CabecalhoPoPs *cabecalho,
                      RegistroPoP *registro, int64_t posicao)
{
    int64_t cursor = ftell(arquivo);
    int64_t anterior = -1;
    int64_t atual = cabecalho->topoLista;

    // a lista fica do maior para o menor; em empate, o novo entra antes
    while (atual != -1)
    {
        int tamanho;
        int64_t proximo;
        fseek(arquivo, atual + 1, SEEK_SET);
        fread(&tamanho, sizeof(int), 1, arquivo);
        fread(&proximo, sizeof(int64_t), 1, arquivo);

        if (tamanho <= registro->tamanhoRegistro)
            break;
        anterior = atual;
        atual = proximo;
    }

    // preserva tamanho e encadeamento; todo o restante vira lixo
    char removido = '1';
    fseek(arquivo, posicao, SEEK_SET);
    fwrite(&removido, sizeof(char), 1, arquivo);
    fwrite(&registro->tamanhoRegistro, sizeof(int), 1, arquivo);
    fwrite(&atual, sizeof(int64_t), 1, arquivo);
    for (int i = TAM_CONTROLE_REGISTRO; i < registro->tamanhoRegistro; i++)
        fputc('$', arquivo);

    // liga o anterior ao novo espaco, ou altera o topo quando ele for o maior
    if (anterior == -1)
        cabecalho->topoLista = posicao;
    else
    {
        fseek(arquivo, anterior + TAM_CONTROLE_INICIAL, SEEK_SET);
        fwrite(&posicao, sizeof(int64_t), 1, arquivo);
    }

    cabecalho->nroRegRem++;
    cabecalho->nroPoPs--;
    fseek(arquivo, cursor, SEEK_SET);
}

// marca a conexao como removida, apaga os dados com lixo e empilha seu rrn.
// atualiza topo e quantidade de removidos; preserva nropares como na base original.
// nao retorna valor; grava com fseek e restaura o cursor para continuar a leitura.
void remover_conexao(FILE *arquivo, CabecalhoConexoes *cabecalho,
                     RegistroConexao *registro, int rrn)
{
    long cursor = ftell(arquivo);
    registro->removido = '1';
    registro->encadeamento = cabecalho->topo_pilha;

    fseek(arquivo, TAM_CABECALHO_CONEXOES + (long)rrn * TAM_REGISTRO_CONEXAO, SEEK_SET);
    fwrite(&registro->removido, sizeof(char), 1, arquivo);
    fwrite(&registro->encadeamento, sizeof(int), 1, arquivo);
    for (int i = TAM_CONTROLE_CONEXAO; i < TAM_REGISTRO_CONEXAO; i++)
        fputc('$', arquivo);

    cabecalho->topo_pilha = rrn;
    cabecalho->nroRegRem++;
    fseek(arquivo, cursor, SEEK_SET);
}

// insere uma conexao no topo livre da pilha ou no fim do arquivo.
// atualiza o cabecalho em memoria e deixa o cursor apos o registro gravado.
// nao retorna valor; mantem a regra original de nropares ao reaproveitar espaco.
void inserir_conexao(FILE *arquivo, CabecalhoConexoes *cabecalho, RegistroConexao *registro)
{
    int rrn;
    if (cabecalho->topo_pilha == -1)
    {
        rrn = cabecalho->proxRRN;
        cabecalho->proxRRN++;
    }
    else
    {
        rrn = cabecalho->topo_pilha;
        fseek(arquivo, TAM_CABECALHO_CONEXOES + (long)rrn * TAM_REGISTRO_CONEXAO + 1, SEEK_SET);
        fread(&cabecalho->topo_pilha, sizeof(int), 1, arquivo);
        cabecalho->nroRegRem--;
        cabecalho->nroPares++;
    }

    registro->removido = '0';
    registro->encadeamento = -1;
    fseek(arquivo, TAM_CABECALHO_CONEXOES + (long)rrn * TAM_REGISTRO_CONEXAO, SEEK_SET);
    escrever_conexao(arquivo, registro);
}

// altera apenas os campos informados da conexao recebida, preservando os demais.
// nao retorna valor e modifica somente a memoria, sem movimentar arquivos.
void aplicar_atualizacoes_conexao(RegistroConexao *registro, CriteriosConexoes *valores)
{
    if (valores->idPoPs != -2)
        registro->idPoPs = valores->idPoPs;
    if (valores->idPoPsConectado != -2)
        registro->idPoPsConectado = valores->idPoPsConectado;
    if (valores->velocidade != -2)
        registro->velocidade = valores->velocidade;
    if (valores->unidadeMedida != -2)
        registro->unidade_medida = valores->unidadeMedida;
}
