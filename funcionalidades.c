#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionalidades.h"
#include "auxiliares/arquivos.h"
#include "auxiliares/leitura.h"
#include "auxiliares/escrita.h"
#include "auxiliares/busca.h"

// func 1
// cria o binario de conexoes com as linhas do csv.
// o cabecalho acompanha a quantidade de registros importados.
void leitura_e_gravacao(void)
{
    char nome_entrada[100];
    char nome_saida[100];
    scanf("%99s %99s", nome_entrada, nome_saida);

    FILE *csv = fopen(nome_entrada, "r");
    if (csv == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    FILE *arquivo = fopen(nome_saida, "wb");
    if (arquivo == NULL)
    {
        fclose(csv);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // a criacao comeca sem removidos; o primeiro rrn disponivel e zero
    CabecalhoConexoes cabecalho = {'0', -1, 0, 0, 0};
    escrever_cabecalho_conexoes(arquivo, &cabecalho);
    char *linha = NULL;
    size_t capacidade = 0;
    // le os titulos antes do laco, pois somente as linhas seguintes viram registros
    int sucesso = ler_linha_csv(csv, &linha, &capacidade) == 1;
    int leitura = 0;

    // descarta os titulos e importa as linhas de dados, preservando campos vazios
    while (sucesso && (leitura = ler_linha_csv(csv, &linha, &capacidade)) == 1)
    {
        char *campos[4];
        if (separar_campos_csv(linha, campos, 4) != 4)
        {
            sucesso = 0;
            break;
        }

        RegistroConexao registro;
        registro.removido = '0';
        registro.encadeamento = -1;
        // os dois identificadores sao obrigatorios; velocidade e unidade podem ser nulos
        registro.idPoPs = atoi(campos[0]);
        registro.idPoPsConectado = atoi(campos[1]);
        registro.velocidade = campos[2][0] == '\0' || campos[2][0] == ' ' ? -1 : atoi(campos[2]);
        registro.unidade_medida = campos[3][0] == '\0' || campos[3][0] == ' ' ? '$' : campos[3][0];
        escrever_conexao(arquivo, &registro);
        // cada linha importada ocupa mais um registro de tamanho fixo
        cabecalho.proxRRN++;
        cabecalho.nroPares++;
    }

    free(linha);
    fclose(csv);
    if (leitura == -1)
        sucesso = 0;
    // o cabecalho final ja e escrito com o status consistente
    if (sucesso)
    {
        cabecalho.status = '1';
        escrever_cabecalho_conexoes(arquivo, &cabecalho);
    }
    fclose(arquivo);

    if (sucesso)
        BinarioNaTela(nome_saida);
    else
        printf("Falha no processamento do arquivo.\n");
}

// func 2
// mostra as conexoes ativas, comecando a leitura depois do cabecalho.
void recuperacao_dados(void)
{
    char nome[100];
    scanf("%99s", nome);
    FILE *arquivo = abrir_arquivo(nome, "rb");
    if (arquivo == NULL)
        return;

    // salta o cabecalho para comecar a leitura no primeiro registro de dados
    fseek(arquivo, TAM_CABECALHO_CONEXOES, SEEK_SET);
    RegistroConexao registro;
    int encontrados = 0;
    while (ler_conexao(arquivo, &registro))
    {
        // o leitor pula os dados dos removidos e so os ativos sao impressos
        if (registro.removido == '0')
        {
            imprimir_conexao(&registro);
            encontrados++;
        }
    }

    if (encontrados == 0)
        printf("Registro inexistente.\n");
    fclose(arquivo);
}

// func 3
// repete a busca no arquivo de conexoes para cada conjunto de criterios.
// cada consulta percorre os registros desde o byte 17.
void busca_condicional(void)
{
    char nome[100];
    int quantidade_buscas;
    scanf("%99s %d", nome, &quantidade_buscas);
    FILE *arquivo = abrir_arquivo(nome, "rb");
    if (arquivo == NULL)
        return;

    for (int i = 0; i < quantidade_buscas; i++)
    {
        int quantidade_campos;
        scanf("%d", &quantidade_campos);
        CriteriosConexoes criterios = ler_criterios_conexoes(quantidade_campos);
        RegistroConexao registro;
        int encontrados = 0;
        // reinicia a varredura, porque a consulta anterior pode ter chegado ao fim
        fseek(arquivo, TAM_CABECALHO_CONEXOES, SEEK_SET);

        while (ler_conexao(arquivo, &registro))
        {
            // todos os criterios devem coincidir no mesmo registro ativo
            if (registro.removido == '0' && verificar_conexao(&criterios, &registro))
            {
                imprimir_conexao(&registro);
                encontrados++;
            }
        }
        if (encontrados == 0)
            printf("Registro inexistente.\n");
        // separa a saida de uma busca da seguinte
        printf("\n");
    }
    fclose(arquivo);
}

// func 4
// calcula a posicao pelo rrn e mostra a conexao, se ela estiver ativa.
void busca_RRN(void)
{
    char nome[100];
    int rrn;
    scanf("%99s %d", nome, &rrn);
    FILE *arquivo = abrir_arquivo(nome, "rb");
    if (arquivo == NULL)
        return;

    CabecalhoConexoes cabecalho = ler_cabecalho_conexoes(arquivo);
    RegistroConexao registro;
    // o proximo rrn ainda nao possui registro, portanto ele ja fica fora da consulta
    if (rrn < 0 || rrn >= cabecalho.proxRRN)
        printf("Registro inexistente.\n");
    else
    {
        // a posicao soma o cabecalho com a quantidade de registros que vem antes
        fseek(arquivo, TAM_CABECALHO_CONEXOES + (long)rrn * TAM_REGISTRO_CONEXAO, SEEK_SET);
        if (ler_conexao(arquivo, &registro) && registro.removido == '0')
            imprimir_conexao(&registro);
        else
            printf("Registro inexistente.\n");
    }
    fclose(arquivo);
}

// func 5
// procura as conexoes que devem ser removidas e coloca seus rrns na pilha.
// a saida mostra o arquivo depois das alteracoes.
void remocao_logica(void)
{
    char nome[100];
    int quantidade_remocoes;
    scanf("%99s %d", nome, &quantidade_remocoes);
    FILE *arquivo = abrir_arquivo(nome, "rb+");
    if (arquivo == NULL)
        return;

    CabecalhoConexoes cabecalho = ler_cabecalho_conexoes(arquivo);
    // marca a alteracao antes de percorrer e sobrescrever os registros encontrados
    alterar_status(arquivo, '0');

    for (int i = 0; i < quantidade_remocoes; i++)
    {
        int quantidade_campos;
        scanf("%d", &quantidade_campos);
        CriteriosConexoes criterios = ler_criterios_conexoes(quantidade_campos);
        RegistroConexao registro;
        int rrn = 0;
        fseek(arquivo, TAM_CABECALHO_CONEXOES, SEEK_SET);

        while (ler_conexao(arquivo, &registro))
        {
            if (registro.removido == '0' && verificar_conexao(&criterios, &registro))
                remover_conexao(arquivo, &cabecalho, &registro, rrn);
            // o rrn cresce mesmo quando a leitura pulou um removido
            rrn++;
        }
    }

    // o topo e os contadores acumulam as remocoes de todas as solicitacoes
    cabecalho.status = '1';
    escrever_cabecalho_conexoes(arquivo, &cabecalho);
    fclose(arquivo);
    BinarioNaTela(nome);
}

// func 6
// le as novas conexoes do terminal e usa os espacos livres antes de aumentar o arquivo.
void insercao(void)
{
    char nome[100];
    int quantidade;
    scanf("%99s %d", nome, &quantidade);
    FILE *arquivo = abrir_arquivo(nome, "rb+");
    if (arquivo == NULL)
        return;

    CabecalhoConexoes cabecalho = ler_cabecalho_conexoes(arquivo);
    alterar_status(arquivo, '0');
    for (int i = 0; i < quantidade; i++)
    {
        RegistroConexao registro = ler_conexao_terminal();
        inserir_conexao(arquivo, &cabecalho, &registro);
    }

    cabecalho.status = '1';
    escrever_cabecalho_conexoes(arquivo, &cabecalho);
    fclose(arquivo);
    BinarioNaTela(nome);
}

// func 7
// atualiza os campos pedidos nas conexoes que atendem a busca.
// a escrita e feita sobre o registro, que continua ocupando 18 bytes.
void atualizacao_registros(void)
{
    char nome[100];
    int quantidade_atualizacoes;
    scanf("%99s %d", nome, &quantidade_atualizacoes);
    FILE *arquivo = abrir_arquivo(nome, "rb+");
    if (arquivo == NULL)
        return;

    CabecalhoConexoes cabecalho = ler_cabecalho_conexoes(arquivo);
    alterar_status(arquivo, '0');
    for (int i = 0; i < quantidade_atualizacoes; i++)
    {
        int quantidade_busca;
        int quantidade_campos;
        scanf("%d", &quantidade_busca);
        CriteriosConexoes criterios = ler_criterios_conexoes(quantidade_busca);
        scanf("%d", &quantidade_campos);
        // os novos valores usam o mesmo formato campo e valor dos criterios de busca
        CriteriosConexoes valores = ler_criterios_conexoes(quantidade_campos);

        RegistroConexao registro;
        fseek(arquivo, TAM_CABECALHO_CONEXOES, SEEK_SET);
        while (ler_conexao(arquivo, &registro))
        {
            if (registro.removido == '0' && verificar_conexao(&criterios, &registro))
            {
                // altera apenas a copia em memoria antes de voltar para gravar no arquivo
                aplicar_atualizacoes_conexao(&registro, &valores);
                // a leitura terminou no proximo registro; recua os 18 bytes do registro atual
                fseek(arquivo, -TAM_REGISTRO_CONEXAO, SEEK_CUR);
                escrever_conexao(arquivo, &registro);
                // sincroniza a troca de escrita para leitura no mesmo arquivo
                fseek(arquivo, 0, SEEK_CUR);
            }
        }
    }

    cabecalho.status = '1';
    escrever_cabecalho_conexoes(arquivo, &cabecalho);
    fclose(arquivo);
    BinarioNaTela(nome);
}
