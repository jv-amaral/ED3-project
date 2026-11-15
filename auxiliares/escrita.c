#include <string.h>
#include "escrita.h"

// auxiliares trab. 1

// grava os seis campos da conexao na posicao atual, avancando 18 bytes.
void escrever_conexao(FILE *arquivo, RegistroConexao *registro)
{
    // os controles ficam antes dos dados e fazem parte dos 18 bytes gravados
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
    // essa etapa muda somente a memoria; quem chamou decide onde gravar a conexao
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

    // primeiro vao os controles e os campos fixos, sempre na ordem do formato binario
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

// encaixa o espaco removido na lista ordenada por tamanho e apaga seus dados.
// mantem o tamanho do registro e restaura o cursor para a varredura continuar.
void remover_registro(FILE *arquivo, CabecalhoPoPs *cabecalho,
                      RegistroPoP *registro, int64_t posicao)
{
    // guarda onde a leitura parou antes de percorrer e alterar a lista de espacos livres
    int64_t cursor = ftell(arquivo);
    int64_t anterior = -1;
    int64_t atual = cabecalho->topoLista;

    // a lista fica do maior para o menor; em empate, o novo entra antes
    while (atual != -1)
    {
        int tamanho;
        int64_t proximo;
        // pula a marca e le apenas o tamanho e o proximo elo da lista
        fseek(arquivo, atual + 1, SEEK_SET);
        fread(&tamanho, sizeof(int), 1, arquivo);
        fread(&proximo, sizeof(int64_t), 1, arquivo);

        if (tamanho <= registro->tamanhoRegistro)
            break;
        // continua enquanto os espacos encontrados forem maiores que o novo removido
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

    // o registro deixa de ser ativo, mas o arquivo nao diminui de tamanho
    cabecalho->nroRegRem++;
    cabecalho->nroPoPs--;
    fseek(arquivo, cursor, SEEK_SET);
}

// escolhe um espaco por worst fit, grava o pop e devolve o offset utilizado.
// modifica o cabecalho em memoria e deixa o cursor depois do registro gravado.
int64_t inserir_pop(FILE *arquivo, CabecalhoPoPs *cabecalho, RegistroPoP *registro)
{
    int64_t posicao = -1;

    // o topo guarda o maior espaco: se ele nao servir, nenhum outro da lista serve
    if (cabecalho->topoLista != -1)
    {
        int tamanho_livre;
        fseek(arquivo, cabecalho->topoLista + 1, SEEK_SET);
        fread(&tamanho_livre, sizeof(int), 1, arquivo);

        if (tamanho_livre >= registro->tamanhoRegistro)
        {
            posicao = cabecalho->topoLista;
            fread(&cabecalho->topoLista, sizeof(int64_t), 1, arquivo);
            cabecalho->nroRegRem--;
            // conserva a capacidade do espaco; escrever_registro completa a sobra com lixo
            registro->tamanhoRegistro = tamanho_livre;
        }
    }

    if (posicao == -1)
    {
        posicao = cabecalho->proxByteOffset;
        cabecalho->proxByteOffset += registro->tamanhoRegistro;
    }

    fseek(arquivo, posicao, SEEK_SET);
    escrever_registro(arquivo, registro);
    cabecalho->nroPoPs++;
    return posicao;
}
void modifica_registro(RegistroPoP *reg, CriteriosPoPs *NovosValores)
{
    if (NovosValores->buscar_id)
    {
        reg->idPoPs = NovosValores->idPoPs;
    }
    
    // Tratamento dinâmico para nomePoPs
    if (NovosValores->nomePoPs != NULL)
    {
        free(reg->nomePoPs); // Liberta a memória antiga
        if (strcmp(NovosValores->nomePoPs, "") == 0) // Tratamento para nulo
        {
            reg->nomePoPs = malloc(1);
            reg->nomePoPs[0] = '\0';
        }
        else
        {
            // Aloca nova memória e copia
            reg->nomePoPs = malloc(strlen(NovosValores->nomePoPs) + 1);
            strcpy(reg->nomePoPs, NovosValores->nomePoPs);
        }
    }

    // Tratamento dinâmico para nomePais
    if (NovosValores->nomePais != NULL)
    {
        free(reg->nomePais); // Liberta a memória antiga
        if (strcmp(NovosValores->nomePais, "") == 0) // Tratamento para nulo
        {
            reg->nomePais = malloc(1);
            reg->nomePais[0] = '\0';
        }
        else
        {
            // Aloca nova memória e copia
            reg->nomePais = malloc(strlen(NovosValores->nomePais) + 1);
            strcpy(reg->nomePais, NovosValores->nomePais);
        }
    }
    
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
            strcpy(reg->siglaPais, NovosValores->siglaPais);
        }
    }
}



