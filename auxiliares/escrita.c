#include <string.h>
#include <stdlib.h>
#include "escrita.h"

// auxiliares trab. 1

// grava os seis campos da conexao na posicao atual, avancando 18 bytes.
void escrever_conexao(FILE *arquivo, RegistroConexao *registro)
{
    fwrite(&registro->removido, sizeof(char), 1, arquivo);
    fwrite(&registro->encadeamento, sizeof(int), 1, arquivo);
    fwrite(&registro->idPoPs, sizeof(int), 1, arquivo);
    fwrite(&registro->idPoPsConectado, sizeof(int), 1, arquivo);
    fwrite(&registro->velocidade, sizeof(int), 1, arquivo);
    fwrite(&registro->unidade_medida, sizeof(char), 1, arquivo);
}

// mostra os campos da conexao, usando nulo e aspas na unidade quando necessario.
void imprimir_conexao(RegistroConexao *registro)
{
    printf("%d %d ", registro->idPoPs, registro->idPoPsConectado);
    // os valores usados para representar nulos no disco nao aparecem como dados na saida
    if (registro->velocidade == -1)
        printf("NULO ");
    else
        printf("%d ", registro->velocidade);

    if (registro->unidade_medida == '$')
        printf("NULO\n");
    else
        printf("\"%c\"\n", registro->unidade_medida);
}

// troca os dados por lixo e liga o rrn ao topo da pilha de removidos.
// atualiza o cabecalho em memoria e volta a posicao de leitura que estava em uso.
void remover_conexao(FILE *arquivo, CabecalhoConexoes *cabecalho,
                     RegistroConexao *registro, int rrn)
{
    // salva a posicao seguinte ao registro para retomar a varredura depois da escrita
    long cursor = ftell(arquivo);
    registro->removido = '1';
    // o novo topo aponta para quem estava no topo antes da remocao
    registro->encadeamento = cabecalho->topo_pilha;

    fseek(arquivo, TAM_CABECALHO_CONEXOES + (long)rrn * TAM_REGISTRO_CONEXAO, SEEK_SET);
    fwrite(&registro->removido, sizeof(char), 1, arquivo);
    fwrite(&registro->encadeamento, sizeof(int), 1, arquivo);
    // apenas os dados viram lixo; a marca e o encadeamento precisam continuar validos
    for (int i = TAM_CONTROLE_CONEXAO; i < TAM_REGISTRO_CONEXAO; i++)
        fputc('$', arquivo);

    cabecalho->topo_pilha = rrn;
    cabecalho->nroRegRem++;
    fseek(arquivo, cursor, SEEK_SET);
}

// usa o rrn do topo da pilha, ou o proximo rrn se nao houver espaco livre.
// ajusta os contadores e deixa o cursor depois da conexao gravada.
void inserir_conexao(FILE *arquivo, CabecalhoConexoes *cabecalho, RegistroConexao *registro)
{
    int rrn;
    if (cabecalho->topo_pilha == -1)
    {
        // sem removidos, ocupa a primeira posicao disponivel no final
        rrn = cabecalho->proxRRN;
        cabecalho->proxRRN++;
    }
    else
    {
        rrn = cabecalho->topo_pilha;
        fseek(arquivo, TAM_CABECALHO_CONEXOES + (long)rrn * TAM_REGISTRO_CONEXAO + 1, SEEK_SET);
        // retira o topo atual e guarda o proximo rrn livre como novo topo
        fread(&cabecalho->topo_pilha, sizeof(int), 1, arquivo);
        cabecalho->nroRegRem--;
        cabecalho->nroPares++;
    }

    // a nova conexao fica ativa e deixa de participar do encadeamento de removidos
    registro->removido = '0';
    registro->encadeamento = -1;
    fseek(arquivo, TAM_CABECALHO_CONEXOES + (long)rrn * TAM_REGISTRO_CONEXAO, SEEK_SET);
    escrever_conexao(arquivo, registro);
}

// copia os valores informados para a conexao. o marcador -2 deixa o campo como esta.
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

// auxiliares trab. 2

// escreve o pop na posicao atual, sem gravar os terminadores de string.
// se o espaco reservado for maior, preenche o final com lixo e avanca ate o proximo registro.
void escrever_registro(FILE *arquivo, const RegistroPoP *registro)
{
    char delimitador = '|';
    int tamanho_nome = strlen(registro->nomePoPs);
    int tamanho_pais = strlen(registro->nomePais);

    fwrite(&registro->removido, sizeof(char), 1, arquivo);
    fwrite(&registro->tamanhoRegistro, sizeof(int), 1, arquivo);
    fwrite(&registro->encadeamentoLista, sizeof(int64_t), 1, arquivo);
    fwrite(&registro->idPoPs, sizeof(int), 1, arquivo);
    fwrite(registro->siglaPais, sizeof(char), 2, arquivo);
    // cada nome e seguido pela barra; nenhum terminador de string vai para o arquivo
    fwrite(registro->nomePoPs, sizeof(char), tamanho_nome, arquivo);
    fwrite(&delimitador, sizeof(char), 1, arquivo);
    fwrite(registro->nomePais, sizeof(char), tamanho_pais, arquivo);
    fwrite(&delimitador, sizeof(char), 1, arquivo);

    // a capacidade do espaco pode ser maior que o conteudo, por isso completa a sobra
    int utilizado = TAM_MINIMO_REGISTRO + tamanho_nome + tamanho_pais;
    for (int i = utilizado; i < registro->tamanhoRegistro; i++)
        fputc('$', arquivo);

}

// imprime id, nome do pop, pais e sigla em uma linha, tratando os campos nulos.
void imprimir_registro(RegistroPoP *registro)
{
    // a marca, o tamanho e o encadeamento sao controles e nao aparecem na listagem
    printf("%d ", registro->idPoPs);
    printf("%s ", registro->nomePoPs[0] == '\0' ? "NULO" : registro->nomePoPs);
    printf("%s ", registro->nomePais[0] == '\0' ? "NULO" : registro->nomePais);
    printf("%s\n", registro->siglaPais[0] == '$' ? "NULO" : registro->siglaPais);
}

// coloca o espaco removido no topo da lista e apaga seus dados.
// mantem o tamanho e restaura o cursor para a varredura continuar.
void remover_registro(FILE *arquivo, CabecalhoPoPs *cabecalho,
                      RegistroPoP *registro, int64_t posicao)
{
    int64_t cursor = ftell(arquivo);
    int64_t proximo = cabecalho->topoLista;

    // o novo removido aponta para o antigo topo, sem ordenar por tamanho
    char removido = '1';
    fseek(arquivo, posicao, SEEK_SET);
    fwrite(&removido, sizeof(char), 1, arquivo);
    fwrite(&registro->tamanhoRegistro, sizeof(int), 1, arquivo);
    fwrite(&proximo, sizeof(int64_t), 1, arquivo);
    for (int i = TAM_CONTROLE_REGISTRO; i < registro->tamanhoRegistro; i++)
        fputc('$', arquivo);

    cabecalho->topoLista = posicao;
    cabecalho->nroRegRem++;
    cabecalho->nroPoPs--;
    fseek(arquivo, cursor, SEEK_SET);
}

// procura o maior espaco livre, grava o pop e devolve o offset usado.
// ajusta o cabecalho em memoria e termina depois do registro gravado.
int64_t inserir_pop(FILE *arquivo, CabecalhoPoPs *cabecalho, RegistroPoP *registro)
{
    int64_t posicao = -1;
    int64_t atual = cabecalho->topoLista;
    int64_t anterior = -1;
    int64_t anterior_maior = -1;
    int64_t proximo_maior = -1;
    int tamanho_maior = 0;

    // percorre todos os elos; em empate, conserva o primeiro maior encontrado
    while (atual != -1)
    {
        int tamanho;
        int64_t proximo;
        fseek(arquivo, atual + 1, SEEK_SET);
        fread(&tamanho, sizeof(int), 1, arquivo);
        fread(&proximo, sizeof(int64_t), 1, arquivo);
        if (tamanho > tamanho_maior)
        {
            tamanho_maior = tamanho;
            posicao = atual;
            anterior_maior = anterior;
            proximo_maior = proximo;
        }
        anterior = atual;
        atual = proximo;
    }

    if (posicao != -1 && tamanho_maior >= registro->tamanhoRegistro)
    {
        // desencaixa somente o escolhido, preservando a ordem dos outros blocos
        if (anterior_maior == -1)
            cabecalho->topoLista = proximo_maior;
        else
        {
            fseek(arquivo, anterior_maior + TAM_CONTROLE_INICIAL, SEEK_SET);
            fwrite(&proximo_maior, sizeof(int64_t), 1, arquivo);
        }
        cabecalho->nroRegRem--;
        // conserva a capacidade; o escritor preenche a sobra com lixo
        registro->tamanhoRegistro = tamanho_maior;
    }
    else
    {
        posicao = cabecalho->proxByteOffset;
        cabecalho->proxByteOffset += registro->tamanhoRegistro;
    }

    fseek(arquivo, posicao, SEEK_SET);
    escrever_registro(arquivo, registro);
    cabecalho->nroPoPs++;
    return posicao;
}
// substitui em memoria apenas os campos informados nos novos valores.
// os nomes recebem copias alocadas, liberadas com liberar_registro; nao acessa o arquivo.
void modifica_registro(RegistroPoP *reg, CriteriosPoPs *NovosValores)
{
    // o indicador de busca tambem informa se um novo identificador foi fornecido
    if (NovosValores->buscar_id)
    {
        reg->idPoPs = NovosValores->idPoPs;
    }
    
    // um ponteiro nulo indica que o nome nao foi pedido na atualizacao
    if (NovosValores->nomePoPs != NULL)
    {
        free(reg->nomePoPs); // libera o nome que veio da leitura do registro
        if (strcmp(NovosValores->nomePoPs, "") == 0) // texto vazio representa o valor nulo
        {
            // mesmo um nome vazio precisa de espaco para o terminador na memoria
            reg->nomePoPs = malloc(1);
            reg->nomePoPs[0] = '\0';
        }
        else
        {
            // copia o texto para que o registro nao dependa da memoria dos criterios
            reg->nomePoPs = malloc(strlen(NovosValores->nomePoPs) + 1);
            strcpy(reg->nomePoPs, NovosValores->nomePoPs);
        }
    }

    // o pais segue o mesmo tratamento: mantem o anterior se nao foi informado
    if (NovosValores->nomePais != NULL)
    {
        free(reg->nomePais); // descarta o pais anterior antes de guardar o novo
        if (strcmp(NovosValores->nomePais, "") == 0) // pais nulo fica como string vazia
        {
            reg->nomePais = malloc(1);
            reg->nomePais[0] = '\0';
        }
        else
        {
            // reserva os caracteres do novo pais mais o terminador de string
            reg->nomePais = malloc(strlen(NovosValores->nomePais) + 1);
            strcpy(reg->nomePais, NovosValores->nomePais);
        }
    }
    
    // a sigla e um vetor fixo: nao precisa de alocacao, e o nulo e representado por lixo
    if (NovosValores->siglaPais != NULL)
    {
        if (strcmp(NovosValores->siglaPais, "") == 0)
        {
            reg->siglaPais[0] = '$';
            reg->siglaPais[1] = '$';
            reg->siglaPais[2] = '\0';
        } 
        else 
        {
            // o terminador fica na memoria; a gravacao usa apenas os dois bytes da sigla
            strcpy(reg->siglaPais, NovosValores->siglaPais);
        }
    }
}



