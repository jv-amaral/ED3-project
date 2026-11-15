#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionalidades.h"
#include "auxiliares/arquivos.h"
#include "auxiliares/leitura.h"
#include "auxiliares/escrita.h"
#include "auxiliares/busca.h"
#include "auxiliares/indice.h"

// funcionalidade 8: le os nomes pela entrada padrao e cria o binario a partir do csv.
// grava cabecalho e registros, fecha os arquivos e exibe o binario; nao retorna valor.
void create_table(void)
{
    char nome_arq_entrada[100];
    char nome_arq_saida[100];

    // o csv fornece os dados; o arquivo de saida sera criado em formato binario
    scanf("%s %s", nome_arq_entrada, nome_arq_saida);

    FILE *arq_csv = fopen(nome_arq_entrada, "r");

    if (arq_csv == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // a linha cresce conforme necessario, sem truncar nomes de tamanho variavel.
    char *linha_csv = NULL;
    size_t capacidade = 0;
    char *campos[5]; // quatro campos de dados e uma possivel coluna de numeracao.

    const char *nomes_campos[] = {"idPoPs", "nomePoPs", "nomePais", "siglaPais"};
    // cada posicao guarda em qual coluna do csv esta um dos quatro campos
    int colunas[4] = {-1, -1, -1, -1};
    int qtd_colunas = 0;
    int sucesso = ler_linha_csv(arq_csv, &linha_csv, &capacidade) == 1;

    // descobre a ordem pelo cabecalho, inclusive quando ha uma coluna inicial extra.
    if (sucesso)
    {
        qtd_colunas = separar_campos_csv(linha_csv, campos, 5);
        for (int i = 0; i < qtd_colunas; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (strcmp(campos[i], nomes_campos[j]) == 0)
                    colunas[j] = i;
            }
        }
        // a coluna extra de numeracao e ignorada, mas os quatro campos devem existir
        for (int i = 0; i < 4; i++)
        {
            if (colunas[i] == -1)
                sucesso = 0;
        }
    }

    if (!sucesso)
    {
        free(linha_csv);
        fclose(arq_csv);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *arq_binario = fopen(nome_arq_saida, "wb");
    if (arq_binario == NULL)
    {
        free(linha_csv);
        fclose(arq_csv);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    CabecalhoPoPs cabecalho;

    // durante a criacao o arquivo fica inconsistente e ainda nao possui removidos
    cabecalho.status = '0';
    cabecalho.topoLista = -1;
    cabecalho.proxByteOffset = 0;
    cabecalho.nroRegRem = 0;
    cabecalho.nroPoPs = 0;

    escrever_cabecalho(arq_binario, &cabecalho);

    // o primeiro byte disponivel, depois do cabecalho, e o de offset 25.
    cabecalho.proxByteOffset = TAM_CABECALHO;

    sucesso = !ferror(arq_binario) && fflush(arq_binario) == 0;

    int leitura = 0;
    while (sucesso && (leitura = ler_linha_csv(arq_csv, &linha_csv, &capacidade)) == 1)
    {
        // separar os campos preserva os vazios, que representam valores nulos
        if (separar_campos_csv(linha_csv, campos, 5) != qtd_colunas)
        {
            sucesso = 0;
            break;
        }
        RegistroPoP registro;
        // o registro importado esta ativo e nao participa da lista de removidos
        registro.removido = '0';
        registro.encadeamentoLista = -1;

        // strtol permite verificar se o id realmente representa um inteiro de 4 bytes.
        char *fim_id;
        errno = 0;
        long id = strtol(campos[colunas[0]], &fim_id, 10);
        if (fim_id == campos[colunas[0]] || *fim_id != '\0' ||
            errno == ERANGE || id < INT_MIN || id > INT_MAX || id == -1)
        {
            sucesso = 0;
            break;
        }
        registro.idPoPs = (int)id;

        // os nomes apontam para a linha e sao gravados antes da proxima leitura.
        registro.nomePoPs = campos[colunas[1]];
        registro.nomePais = campos[colunas[2]];
        char *sigla = campos[colunas[3]];
        if (sigla[0] != '\0' && strlen(sigla) != 2)
        {
            sucesso = 0;
            break;
        }
        // a sigla nula ocupa dois bytes de lixo; o terminador nao vai para o disco
        memcpy(registro.siglaPais, sigla[0] == '\0' ? "$$" : sigla, 2);
        registro.siglaPais[2] = '\0'; // terminador somente em memoria.

        // 19 bytes fixos + dois delimitadores + os bytes dos dois nomes.
        size_t tamanho = TAM_MINIMO_REGISTRO + strlen(registro.nomePoPs) + strlen(registro.nomePais);
        if (tamanho > INT_MAX || cabecalho.nroPoPs == INT_MAX)
        {
            sucesso = 0;
            break;
        }
        registro.tamanhoRegistro = (int)tamanho;

        // grava agora porque os nomes apontam para a linha que sera lida novamente
        if (!escrever_registro(arq_binario, &registro))
        {
            sucesso = 0;
            break;
        }

        // os tamanhos variam, entao o proximo offset cresce pelo tamanho gravado
        cabecalho.proxByteOffset += registro.tamanhoRegistro;
        cabecalho.nroPoPs++;
    }

    if (leitura == -1)
        sucesso = 0;
    if (fclose(arq_csv) != 0)
        sucesso = 0;
    // basta liberar a linha: os campos e os nomes apenas apontavam para dentro dela
    free(linha_csv);

    // termina os dados antes de gravar o cabecalho final ja consistente
    if (sucesso)
    {
        sucesso = fflush(arq_binario) == 0;
        if (sucesso)
        {
            // o status e alterado em memoria antes da unica gravacao final do cabecalho
            cabecalho.status = '1';
            escrever_cabecalho(arq_binario, &cabecalho);
            sucesso = !ferror(arq_binario);
        }
    }
    if (fclose(arq_binario) != 0)
        sucesso = 0;

    // a exibicao so acontece depois de fechar o arquivo de dados
    if (sucesso)
        BinarioNaTela(nome_arq_saida);
    else
        printf("Falha no processamento do arquivo.\n");
}

// funcionalidade 9: monta e ordena o indice com os ids e offsets dos registros ativos.
// le os nomes pela entrada, percorre e fecha os arquivos e exibe o indice; nao retorna valor.
void create_index(void)
{
    char nome_dados[100];
    char nome_indice[100];
    scanf("%99s %99s", nome_dados, nome_indice);

    FILE *dados = abrir_arquivo(nome_dados, "rb");
    if (dados == NULL)
        return;

    CabecalhoPoPs cabecalho = ler_cabecalho(dados);
    // o vetor guarda apenas as chaves e os offsets; os nomes ficam no arquivo de dados
    IndicePoPs *indice = malloc((cabecalho.nroPoPs + 1) * sizeof(IndicePoPs));
    if (indice == NULL)
    {
        fclose(dados);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int quantidade = 0;
    // a leitura do cabecalho ja deixou o cursor no inicio do primeiro registro
    int64_t posicao = TAM_CABECALHO;
    RegistroPoP registro;
    while (ler_registro(dados, &registro))
    {
        if (registro.removido == '0')
        {
            // usa o inicio do registro, pois a leitura ja avancou ate o seguinte
            indice[quantidade].idPoPs = registro.idPoPs;
            indice[quantidade].byteoffset = posicao;
            quantidade++;
        }
        liberar_registro(&registro);
        // acompanha o cursor para guardar o offset correto na proxima iteracao
        posicao = ftell(dados);
    }
    fclose(dados);

    // o arquivo de dados mantem sua ordem; somente o vetor do indice e ordenado
    qsort(indice, quantidade, sizeof(IndicePoPs), comparar_indices);
    if (salvar_indice(nome_indice, indice, quantidade))
        BinarioNaTela(nome_indice);
    else
        printf("Falha no processamento do arquivo.\n");
    free(indice);
}

// funcionalidade 10: percorre o arquivo e imprime somente os registros ativos.
// le o nome pela entrada padrao, fecha o arquivo ao terminar e nao retorna valor.
void listar_registros(void)
{
    char nome_arquivo[100];
    scanf("%99s", nome_arquivo);

    FILE *arquivo = abrir_arquivo(nome_arquivo, "rb");
    if (arquivo == NULL)
        return;

    int encontrados = 0;
    RegistroPoP registro;
    // o cabecalho nao faz parte da listagem, por isso a leitura comeca no byte 25
    fseek(arquivo, TAM_CABECALHO, SEEK_SET);

    while (ler_registro(arquivo, &registro))
    {
        // a leitura ja pula os removidos sem carregar seus campos de dados
        if (registro.removido == '0')
        {
            imprimir_registro(&registro);
            encontrados++;
        }
        // os nomes foram alocados pelo leitor e nao serao usados na proxima iteracao
        liberar_registro(&registro);
    }

    // tambem vale para um arquivo que possui apenas registros removidos
    if (encontrados == 0)
        printf("Registro inexistente.\n");
    fclose(arquivo);
}

// funcionalidade 11: usa o indice para id e busca sequencial para os outros campos.
// le as buscas da entrada, imprime os resultados e fecha os arquivos; nao retorna valor.
void busca(void)
{
    char nome_dados[100];
    char nome_indice[100];
    int quantidade_buscas;
    scanf("%99s %99s %d", nome_dados, nome_indice, &quantidade_buscas);

    FILE *dados = abrir_arquivo(nome_dados, "rb");
    if (dados == NULL)
        return;
    FILE *arquivo_indice = abrir_arquivo(nome_indice, "rb");
    if (arquivo_indice == NULL)
    {
        fclose(dados);
        return;
    }

    int quantidade_indices;
    // carrega o indice uma vez e reutiliza o vetor em todas as buscas desta execucao
    IndicePoPs *indice = carregar_indice(arquivo_indice, &quantidade_indices);
    fclose(arquivo_indice);
    if (indice == NULL)
    {
        fclose(dados);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    for (int i = 0; i < quantidade_buscas; i++)
    {
        int quantidade_campos;
        scanf("%d", &quantidade_campos);
        CriteriosPoPs criterios = ler_criterios(quantidade_campos);
        RegistroPoP registro;
        // cada solicitacao tem sua propria contagem de resultados
        int encontrados = 0;

        if (criterios.buscar_id)
        {
            // o auxiliar busca o id no vetor, acessa o offset e confere os demais campos
            int posicao = buscar_registro_indexado(dados, indice, quantidade_indices,
                                                   &criterios, &registro);
            if (posicao != -1)
            {
                imprimir_registro(&registro);
                liberar_registro(&registro);
                encontrados++;
            }
        }
        else
        {
            // cada busca sequencial comeca no primeiro registro de dados
            fseek(dados, TAM_CABECALHO, SEEK_SET);
            while (ler_registro(dados, &registro))
            {
                // um registro so e exibido quando satisfaz todos os criterios
                if (registro.removido == '0' && verificar_criterios(&criterios, &registro))
                {
                    imprimir_registro(&registro);
                    encontrados++;
                }
                liberar_registro(&registro);
            }
        }

        if (encontrados == 0)
            printf("Registro inexistente.\n");
        // os criterios seguintes podem conter outros textos, entao libera os atuais
        liberar_criterios(&criterios);
    }

    free(indice);
    fclose(dados);
}

// funcionalidade 12: remove os registros encontrados e atualiza o indice primario.
// mantem a lista de removidos por tamanho decrescente para worst fit.
// le as solicitacoes da entrada, altera e fecha os dois arquivos e nao retorna valor.
void remover_registros(void)
{
    char nome_dados[100];
    char nome_indice[100];
    int quantidade_remocoes;
    scanf("%s %s %d", nome_dados, nome_indice, &quantidade_remocoes);

    FILE *dados = abrir_arquivo(nome_dados, "rb+");
    if (dados == NULL)
        return;
    FILE *arquivo_indice = abrir_arquivo(nome_indice, "rb+");
    if (arquivo_indice == NULL)
    {
        fclose(dados);
        return;
    }

    CabecalhoPoPs cabecalho = ler_cabecalho(dados);
    int quantidade_indices;
    // as exclusoes mudam o vetor em memoria; o indice em disco sera salvo no final
    IndicePoPs *indice = carregar_indice(arquivo_indice, &quantidade_indices);
    if (indice == NULL)
    {
        fclose(dados);
        fclose(arquivo_indice);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // apenas o status muda no inicio; o cabecalho completo fica para o final
    alterar_status(dados, '0');
    alterar_status(arquivo_indice, '0');
    fclose(arquivo_indice);

    for (int i = 0; i < quantidade_remocoes; i++)
    {
        int quantidade_campos;
        scanf("%d", &quantidade_campos);
        CriteriosPoPs criterios = ler_criterios(quantidade_campos);
        RegistroPoP registro;

        if (criterios.buscar_id)
        {
            // com id unico, no maximo um registro pode satisfazer esta solicitacao
            int posicao_indice = buscar_registro_indexado(dados, indice, quantidade_indices, &criterios, &registro);
            if (posicao_indice != -1)
            {
                // guarda o offset antes de retirar a entrada e deslocar o vetor
                int64_t posicao = indice[posicao_indice].byteoffset;
                remover_registro(dados, &cabecalho, &registro, posicao);
                retirar_do_indice(indice, &quantidade_indices, posicao_indice);
                liberar_registro(&registro);
            }
        }
        else
        {
            // sem id, percorre os registros na ordem fisica e pode remover varios
            int64_t posicao = TAM_CABECALHO;
            fseek(dados, posicao, SEEK_SET);

            while (ler_registro(dados, &registro))
            {
                if (registro.removido == '0' && verificar_criterios(&criterios, &registro))
                {
                    // o auxiliar preenche com lixo, encadeia por tamanho e atualiza os contadores
                    remover_registro(dados, &cabecalho, &registro, posicao);
                    // busca a chave para retirar a entrada correspondente do indice
                    int posicao_indice = buscar_no_indice(indice, quantidade_indices, registro.idPoPs);
                    retirar_do_indice(indice, &quantidade_indices, posicao_indice);
                }

                liberar_registro(&registro);
                // a remocao restaura o cursor, permitindo continuar a varredura
                posicao = ftell(dados);
            }
        }
        // nenhuma mensagem e necessaria se nada foi removido; segue para a proxima busca
        liberar_criterios(&criterios);
    }

    // salva o indice sem entradas antigas e grava o cabecalho final de uma vez
    int sucesso = !ferror(dados) && fflush(dados) == 0;
    if (sucesso)
        sucesso = salvar_indice(nome_indice, indice, quantidade_indices);
    if (sucesso)
    {
        // grava o cabecalho completo ja consistente, sem voltar depois apenas pelo status
        cabecalho.status = '1';
        escrever_cabecalho(dados, &cabecalho);
        sucesso = !ferror(dados);
    }

    if (fclose(dados) != 0)
        sucesso = 0;
    free(indice);

    if (sucesso)
    {
        // respeita a ordem de saida do enunciado: dados primeiro, indice depois
        BinarioNaTela(nome_dados);
        BinarioNaTela(nome_indice);
    }
    else
        printf("Falha no processamento do arquivo.\n");
}
