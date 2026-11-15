#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionalidades.h"
#include "auxiliares/arquivos.h"
#include "auxiliares/leitura.h"
#include "auxiliares/escrita.h"
#include "auxiliares/busca.h"

// funcionalidade 1: cria o binario de conexoes a partir das quatro colunas do csv.
// le os nomes pela entrada, grava cabecalho e registros, fecha e exibe o binario.
// nao retorna valor; usa os mesmos auxiliares csv da funcionalidade 8.
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

    CabecalhoConexoes cabecalho = {'0', -1, 0, 0, 0};
    escrever_cabecalho_conexoes(arquivo, &cabecalho);
    char *linha = NULL;
    size_t capacidade = 0;
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
        registro.idPoPs = atoi(campos[0]);
        registro.idPoPsConectado = atoi(campos[1]);
        registro.velocidade = campos[2][0] == '\0' || campos[2][0] == ' ' ? -1 : atoi(campos[2]);
        registro.unidade_medida = campos[3][0] == '\0' || campos[3][0] == ' ' ? '$' : campos[3][0];
        escrever_conexao(arquivo, &registro);
        cabecalho.proxRRN++;
        cabecalho.nroPares++;
    }

    free(linha);
    fclose(csv);
    sucesso = sucesso && leitura != -1 && !ferror(arquivo);
    if (sucesso)
    {
        cabecalho.status = '1';
        escrever_cabecalho_conexoes(arquivo, &cabecalho);
    }
    if (fclose(arquivo) != 0)
        sucesso = 0;

    if (sucesso)
        BinarioNaTela(nome_saida);
    else
        printf("Falha no processamento do arquivo.\n");
}

// funcionalidade 2: percorre as conexoes e mostra apenas os registros ativos.
// le o nome pela entrada, avanca do byte 17 ate o fim e fecha o arquivo; nao retorna valor.
void recuperacao_dados(void)
{
    char nome[100];
    scanf("%99s", nome);
    FILE *arquivo = abrir_arquivo(nome, "rb");
    if (arquivo == NULL)
        return;

    fseek(arquivo, TAM_CABECALHO_CONEXOES, SEEK_SET);
    RegistroConexao registro;
    int encontrados = 0;
    while (ler_conexao(arquivo, &registro))
    {
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

// funcionalidade 3: realiza as buscas sequenciais nos registros de conexoes.
// consome os criterios da entrada e reinicia o cursor no byte 17 a cada busca.
// imprime os resultados, fecha o arquivo e nao retorna valor.
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
        fseek(arquivo, TAM_CABECALHO_CONEXOES, SEEK_SET);

        while (ler_conexao(arquivo, &registro))
        {
            if (registro.removido == '0' && verificar_conexao(&criterios, &registro))
            {
                imprimir_conexao(&registro);
                encontrados++;
            }
        }
        if (encontrados == 0)
            printf("Registro inexistente.\n");
        // preserva a linha em branco entre as buscas da implementacao original
        printf("\n");
    }
    fclose(arquivo);
}

// funcionalidade 4: acessa uma conexao diretamente pelo rrn usando fseek.
// le o nome e o rrn pela entrada, imprime o resultado e fecha o arquivo.
// nao retorna valor; rrn negativo, fora do arquivo ou removido nao e exibido.
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
    if (rrn < 0 || rrn >= cabecalho.proxRRN)
        printf("Registro inexistente.\n");
    else
    {
        fseek(arquivo, TAM_CABECALHO_CONEXOES + (long)rrn * TAM_REGISTRO_CONEXAO, SEEK_SET);
        if (ler_conexao(arquivo, &registro) && registro.removido == '0')
            imprimir_conexao(&registro);
        else
            printf("Registro inexistente.\n");
    }
    fclose(arquivo);
}

// funcionalidade 5: remove conexoes que atendem aos criterios e empilha os rrns.
// le as solicitacoes pela entrada, altera o arquivo e grava o cabecalho final.
// fecha o binario antes de exibi-lo e nao retorna valor.
void remocao_logica(void)
{
    char nome[100];
    int quantidade_remocoes;
    scanf("%99s %d", nome, &quantidade_remocoes);
    FILE *arquivo = abrir_arquivo(nome, "rb+");
    if (arquivo == NULL)
        return;

    CabecalhoConexoes cabecalho = ler_cabecalho_conexoes(arquivo);
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

    cabecalho.status = '1';
    escrever_cabecalho_conexoes(arquivo, &cabecalho);
    fclose(arquivo);
    BinarioNaTela(nome);
}

// funcionalidade 6: le novas conexoes e reaproveita a pilha antes de crescer o arquivo.
// altera o binario e seus contadores, fecha antes da exibicao e nao retorna valor.
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

// funcionalidade 7: busca conexoes e sobrescreve apenas os campos solicitados.
// le criterios e valores pela entrada, percorre o arquivo e grava o cabecalho final.
// fecha antes da exibicao e nao retorna valor; os registros continuam com 18 bytes.
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
        CriteriosConexoes valores = ler_criterios_conexoes(quantidade_campos);

        RegistroConexao registro;
        fseek(arquivo, TAM_CABECALHO_CONEXOES, SEEK_SET);
        while (ler_conexao(arquivo, &registro))
        {
            if (registro.removido == '0' && verificar_conexao(&criterios, &registro))
            {
                aplicar_atualizacoes_conexao(&registro, &valores);
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
