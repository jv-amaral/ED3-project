#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionalidades.h"
#include "auxiliares/arquivos.h"
#include "auxiliares/leitura.h"
#include "auxiliares/escrita.h"
#include "auxiliares/busca.h"
#include "auxiliares/indice.h"

// importa os pops do csv para o arquivo binario.
// a primeira linha indica onde esta cada campo, inclusive quando ha uma coluna extra.
// para cada linha de dados, monta um registro de tamanho variavel e atualiza os contadores.
// ao terminar, grava o cabecalho consistente e exibe o binario ja fechado.
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
    // essa primeira leitura pega os titulos, que nao serao gravados como registro
    int sucesso = ler_linha_csv(arq_csv, &linha_csv, &capacidade) == 1;

    // descobre a ordem pelo cabecalho, inclusive quando ha uma coluna inicial extra.
    if (sucesso)
    {
        qtd_colunas = separar_campos_csv(linha_csv, campos, 5);
        for (int i = 0; i < qtd_colunas; i++)
        {
            // compara o titulo da coluna com os nomes dos campos que precisamos guardar
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
        // sem as colunas necessarias, nao e possivel montar os registros do arquivo
        free(linha_csv);
        fclose(arq_csv);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // o modo wb cria a saida ou substitui seu conteudo, caso ela ja exista
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

    // reserva os 25 bytes iniciais; os dados serao escritos depois desse cabecalho
    escrever_cabecalho(arq_binario, &cabecalho);

    // o primeiro byte disponivel, depois do cabecalho, e o de offset 25.
    cabecalho.proxByteOffset = TAM_CABECALHO;

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

        // converte o texto da coluna do identificador para o inteiro guardado no registro
        registro.idPoPs = atoi(campos[colunas[0]]);

        // os nomes apontam para a linha e sao gravados antes da proxima leitura.
        registro.nomePoPs = campos[colunas[1]];
        registro.nomePais = campos[colunas[2]];
        char *sigla = campos[colunas[3]];
        // a sigla nula ocupa dois bytes de lixo; o terminador nao vai para o disco
        memcpy(registro.siglaPais, sigla[0] == '\0' ? "$$" : sigla, 2);
        registro.siglaPais[2] = '\0'; // terminador somente em memoria.

        // soma os 19 bytes fixos, os dois delimitadores e o comprimento dos nomes
        // os terminadores de string usados em memoria nao entram nesse tamanho
        registro.tamanhoRegistro = TAM_MINIMO_REGISTRO + strlen(registro.nomePoPs) + strlen(registro.nomePais);

        // grava agora porque os nomes apontam para a linha que sera lida novamente
        escrever_registro(arq_binario, &registro);

        // os tamanhos variam, entao o proximo offset cresce pelo tamanho gravado
        cabecalho.proxByteOffset += registro.tamanhoRegistro;
        cabecalho.nroPoPs++;
    }

    // o leitor usa -1 quando nao consegue aumentar o buffer da linha
    if (leitura == -1)
        sucesso = 0;
    fclose(arq_csv);
    // basta liberar a linha: os campos e os nomes apenas apontavam para dentro dela
    free(linha_csv);

    // grava os contadores finais junto com o status consistente
    if (sucesso)
    {
        cabecalho.status = '1';
        escrever_cabecalho(arq_binario, &cabecalho);
    }
    fclose(arq_binario);

    // a exibicao so acontece depois de fechar o arquivo de dados
    if (sucesso)
        BinarioNaTela(nome_arq_saida);
    else
        printf("Falha no processamento do arquivo.\n");
}

// monta o indice com o id e o offset de cada pop ativo.
// as entradas sao ordenadas antes de gravar o arquivo de indice.
// o offset aponta para o inicio do registro de dados, e nao para o campo do identificador.
// apenas le o arquivo de dados; o novo arquivo criado e o de indice.
void create_index(void)
{
    char nome_dados[100];
    char nome_indice[100];
    scanf("%99s %99s", nome_dados, nome_indice);

    FILE *dados = abrir_arquivo(nome_dados, "rb");
    if (dados == NULL)
        return;

    // a quantidade de pops ativos permite reservar espaco para as entradas do indice
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
        // os nomes nao fazem parte do indice e podem ser liberados depois de copiar a chave
        liberar_registro(&registro);
        // acompanha o cursor para guardar o offset correto na proxima iteracao
        posicao = ftell(dados);
    }
    fclose(dados);

    // o arquivo de dados mantem sua ordem; somente o vetor do indice e ordenado
    qsort(indice, quantidade, sizeof(IndicePoPs), comparar_indices);
    // salvar_indice cuida do cabecalho de um byte e da gravacao das entradas ordenadas
    if (salvar_indice(nome_indice, indice, quantidade))
        BinarioNaTela(nome_indice);
    else
        printf("Falha no processamento do arquivo.\n");
    free(indice);
}

// percorre os pops e imprime os que nao foram removidos.
// se nao houver nenhum ativo, mostra a mensagem de registro inexistente.
// a leitura segue a ordem fisica do arquivo, sem consultar o indice.
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
            // a impressao trata os nulos; o contador so considera registros exibidos
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

// usa o indice quando a busca tem idpops; nos outros casos faz a varredura.
// todos os campos pedidos precisam coincidir para mostrar o registro.
// cada solicitacao e independente, podendo encontrar nenhum, um ou varios pops.
// o indice fica em memoria durante as consultas e os dados sao abertos so para leitura.
void busca(void)
{
    char nome_dados[100];
    char nome_indice[100];
    int quantidade_buscas;
    scanf("%99s %99s %d", nome_dados, nome_indice, &quantidade_buscas);

    FILE *dados = abrir_arquivo(nome_dados, "rb");
    if (dados == NULL)
        return;
    // os dois arquivos precisam estar consistentes para relacionar as chaves aos dados
    FILE *arquivo_indice = abrir_arquivo(nome_indice, "rb");
    if (arquivo_indice == NULL)
    {
        fclose(dados);
        return;
    }

    int quantidade_indices;
    // carrega o indice uma vez e reutiliza o vetor em todas as buscas desta execucao
    IndicePoPs *indice = carregar_indice(arquivo_indice, &quantidade_indices);
    // as consultas seguintes usam o vetor, entao o arquivo de indice pode ser fechado
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
        // o numero lido informa quantos pares de campo e valor pertencem a esta consulta
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
                // so chega aqui se o pop encontrado pelo id tambem atender aos outros campos
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
            // nao encerra a funcao, pois ainda pode haver outras consultas na entrada
            printf("Registro inexistente.\n");
        // os criterios seguintes podem conter outros textos, entao libera os atuais
        liberar_criterios(&criterios);
    }

    free(indice);
    fclose(dados);
}

// remove os pops selecionados e retira suas chaves do indice.
// os espacos liberados entram na lista de removidos, do maior para o menor.
// nas buscas por id usa acesso direto; nas demais, percorre todos os registros ativos.
// o cabecalho e o indice acompanham as remocoes e sao gravados ao final da operacao.
void remover_registros(void)
{
    char nome_dados[100];
    char nome_indice[100];
    int quantidade_remocoes;
    scanf("%s %s %d", nome_dados, nome_indice, &quantidade_remocoes);

    // rb+ permite consultar e modificar o mesmo arquivo, sem apagar os registros existentes
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
    // o arquivo e fechado, mas seu vetor permanece disponivel para as exclusoes
    fclose(arquivo_indice);

    for (int i = 0; i < quantidade_remocoes; i++)
    {
        int quantidade_campos;
        scanf("%d", &quantidade_campos);
        // os criterios valem so para esta solicitacao, que pode remover mais de um pop
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
                // primeiro libera o espaco nos dados; depois elimina a chave do vetor
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
                // removidos anteriores ja foram pulados pelo leitor e nao devem entrar na lista outra vez
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

    // salva o indice sem as chaves retiradas durante as remocoes
    // recriar o indice tambem retira os bytes das entradas que nao existem mais
    if (!salvar_indice(nome_indice, indice, quantidade_indices))
    {
        fclose(dados);
        free(indice);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // o cabecalho final ja leva o status 1, sem outra escrita so para esse byte
    cabecalho.status = '1';
    escrever_cabecalho(dados, &cabecalho);
    // fecha os dados e libera o vetor antes da exibicao dos dois arquivos
    fclose(dados);
    free(indice);

    // a ordem de saida e dados primeiro e indice depois
    BinarioNaTela(nome_dados);
    BinarioNaTela(nome_indice);
}
