#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionalidades.h"
#include "auxiliares/arquivos.h"
#include "auxiliares/leitura.h"
#include "auxiliares/escrita.h"
#include "auxiliares/busca.h"
#include "auxiliares/indice.h"

// func 8
// cria o binario de pops a partir do csv e exibe a soma dos bytes ao terminar.
// reconhece as colunas pelo titulo, ignorando a numeracao inicial.
void create_table(void)
{
    char nome_arq_entrada[100];
    char nome_arq_saida[100];

    scanf("%99s %99s", nome_arq_entrada, nome_arq_saida);

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

    cabecalho.status = '0';
    cabecalho.topoLista = -1;
    cabecalho.proxByteOffset = 0;
    cabecalho.nroRegRem = 0;
    cabecalho.nroPoPs = 0;

    // reserva os 25 bytes iniciais; os dados serao escritos depois desse cabecalho
    escrever_cabecalho(arq_binario, &cabecalho);

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
        registro.removido = '0';
        registro.encadeamentoLista = -1;

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

    if (sucesso)
        BinarioNaTela(nome_arq_saida);
    else
        printf("Falha no processamento do arquivo.\n");
}

// func 9
// cria o indice ordenado dos pops ativos, guardando o offset inicial de cada registro.
// apenas consulta os dados; grava e fecha o indice antes de exibi-lo.
void create_index(void)
{
    char arquivo_binario[50];
    char arquivo_index[50];
    scanf("%49s", arquivo_binario);
    scanf("%49s",arquivo_index);
    RegistroPoP reg;

    FILE *binario = abrir_arquivo(arquivo_binario, "rb");
    if (binario == NULL)
    {
        return;
    }
    CabecalhoPoPs cab = ler_cabecalho(binario);
    int64_t byteOffsetAtual = TAM_CABECALHO;
    // a posicao minima evita malloc de tamanho zero quando nao ha ativos
    int max_registros = cab.nroPoPs > 0 ? cab.nroPoPs : 1;
    IndicePoPs *vetor_indices = (IndicePoPs *) malloc(max_registros * sizeof(IndicePoPs)); // vetor para armazenar idpops e o byteoffset
    if (vetor_indices == NULL)
    {
        fclose(binario);
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    int i = 0;
    while( fread(&reg.removido, sizeof(char), 1, binario)==1 )
    {
        fread(&reg.tamanhoRegistro,sizeof(int),1,binario);
        // so precisamos da chave; os demais campos sao pulados pelo tamanho
        if(reg.removido == '0')
        {
            fread(&reg.encadeamentoLista, sizeof(int64_t),1,binario);
            fread(&reg.idPoPs, sizeof(int),1,binario);
            vetor_indices[i].idPoPs = reg.idPoPs;
            vetor_indices[i].byteoffset = byteOffsetAtual;
            i++;
            fseek(binario, byteOffsetAtual + reg.tamanhoRegistro, SEEK_SET);
        } else
        {
            fseek(binario, reg.tamanhoRegistro - TAM_CONTROLE_INICIAL, SEEK_CUR);
        }

        byteOffsetAtual = byteOffsetAtual + reg.tamanhoRegistro;

    }


    qsort(vetor_indices, i, sizeof(IndicePoPs), comparar_indices);
    fclose(binario);

    int sucesso = salvar_indice(arquivo_index, vetor_indices, i);
    free(vetor_indices);
    if (sucesso)
        BinarioNaTela(arquivo_index);
    else
        printf("Falha no processamento do arquivo.\n");
}

// func 10
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

// func 11
// executa as consultas, usando o indice para idpops e varredura nos demais casos.
// exige todos os criterios e libera os dados de cada consulta antes da seguinte.
void busca(void)
{
    char arquivo_binario[50];
    char arquivo_index[50];

    scanf("%49s", arquivo_binario);
    scanf("%49s", arquivo_index);
    RegistroPoP reg;

    FILE *binario = abrir_arquivo(arquivo_binario, "rb");

    if (binario == NULL)
    {
        return;
    }
    FILE *index = abrir_arquivo(arquivo_index, "rb");
    if (index == NULL)
    {
        fclose(binario);
        return;
    }
    int total_indices = 0;
    IndicePoPs *vetor_indices = carregar_indice(index, &total_indices);
    fclose(index);
    if (vetor_indices == NULL)
    {
        fclose(binario);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int n;
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        int m;
        scanf("%d", &m);
        CriteriosPoPs C = ler_criterios(m);
        int registro_encontrado = 0;

        if (C.buscar_id)
        {
            // o auxiliar confere tambem os criterios alem do identificador
            int posicao_indice = buscar_registro_indexado(binario, vetor_indices, total_indices, &C, &reg);
            if (posicao_indice != -1)
            {
                imprimir_registro(&reg);
                registro_encontrado = 1;
                liberar_registro(&reg);
            }
        }
        else
        {
            fseek(binario, TAM_CABECALHO, SEEK_SET);

            while (ler_registro(binario, &reg) == 1)
            {
                if (reg.removido == '0')
                {
                    if (verificar_criterios(&C, &reg) == 1)
                    {
                        imprimir_registro(&reg);
                        registro_encontrado = 1;
                    }
                }
                liberar_registro(&reg);
            }

        }
        if (registro_encontrado == 0)
            printf("Registro inexistente.\n");

        liberar_criterios(&C);
        // o corretor espera uma linha em branco depois de cada consulta
        printf("\n");
    }

    free(vetor_indices);
    fclose(binario);
}

// func 12
// remove os pops selecionados e suas chaves, mantendo a lista de espacos por tamanho.
// salva dados e indice e exibe os dois arquivos depois de fecha-los.
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
    fclose(dados);
    free(indice);

    // a ordem de saida e dados primeiro e indice depois
    BinarioNaTela(nome_dados);
    BinarioNaTela(nome_indice);
}

// func 13
// insere os pops recebidos pela entrada, reaproveitando os maiores espacos removidos.
// atualiza os contadores e o indice; fecha os arquivos antes de exibir os dois binarios.
void inserir_registros(void)
{
    char nome_dados[100];
    char nome_indice[100];
    int quantidade_insercoes;
    scanf("%99s %99s %d", nome_dados, nome_indice, &quantidade_insercoes);

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
    IndicePoPs *indice = carregar_indice(arquivo_indice, &quantidade_indices);
    if (indice == NULL)
    {
        fclose(dados);
        fclose(arquivo_indice);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // reserva de uma vez o espaco para as novas entradas, sem realocar a cada insercao
    IndicePoPs *novo_indice = realloc(indice, (quantidade_indices + quantidade_insercoes + 1) * sizeof(IndicePoPs));
    if (novo_indice == NULL)
    {
        free(indice);
        fclose(dados);
        fclose(arquivo_indice);
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    indice = novo_indice;

    alterar_status(dados, '0');
    alterar_status(arquivo_indice, '0');
    fclose(arquivo_indice);

    for (int i = 0; i < quantidade_insercoes; i++)
    {
        RegistroPoP registro = ler_pop_terminal();
        int64_t posicao = inserir_pop(dados, &cabecalho, &registro);
        // a chave precisa apontar para o espaco realmente usado, inclusive quando reaproveitado
        inserir_no_indice(indice, &quantidade_indices, registro.idPoPs, posicao);
        liberar_registro(&registro);
    }

    if (!salvar_indice(nome_indice, indice, quantidade_indices))
    {
        fclose(dados);
        free(indice);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // grava o cabecalho uma vez no final, ja com o status e os contadores atualizados
    cabecalho.status = '1';
    escrever_cabecalho(dados, &cabecalho);
    fclose(dados);
    free(indice);

    BinarioNaTela(nome_dados);
    BinarioNaTela(nome_indice);
}

// func 14
// atualiza os pops selecionados no proprio bloco ou reinsere por worst fit se crescerem.
// rejeita chave nula ou repetida; salva o indice antes de finalizar o cabecalho dos dados.

void atualiza_registro()
{
    char arquivo_binario[50];
    char arquivo_index[50];

    scanf("%49s", arquivo_binario);
    scanf("%49s", arquivo_index);

    FILE *binario = abrir_arquivo(arquivo_binario, "rb+");
    if (binario == NULL)
        return;
    FILE *index = abrir_arquivo(arquivo_index, "rb+");
    if (index == NULL)
    {
        fclose(binario);
        return;
    }

    CabecalhoPoPs cab = ler_cabecalho(binario);
    RegistroPoP reg;

    int total_indices = 0;
    IndicePoPs *vetor_indices = carregar_indice(index, &total_indices);
    if (vetor_indices == NULL)
    {
        fclose(binario);
        fclose(index);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    alterar_status(binario, '0');
    alterar_status(index, '0');
    fclose(index);

    int n;
    scanf("%d", &n);
    int erro_atualizacao = 0;
    for (int i = 0; i < n && !erro_atualizacao; i++)
    {
        int m;
        scanf("%d", &m);
        CriteriosPoPs C = ler_criterios(m);

        int p;
        scanf("%d", &p);
        CriteriosPoPs NovosValores = ler_criterios(p);

        if (C.buscar_id)
        {
            int inicio = 0;
            int fim = total_indices - 1;
            int64_t offset_encontrado = -1;
            int posicao_indice = -1;

            while (inicio <= fim) {
                int meio = (inicio + fim) / 2;

                if (vetor_indices[meio].idPoPs == C.idPoPs)
                {
                    offset_encontrado = vetor_indices[meio].byteoffset;
                    posicao_indice = meio;
                    break;
                }
                else if (vetor_indices[meio].idPoPs < C.idPoPs)
                {
                    inicio = meio + 1;
                }
                else
                {
                    fim = meio - 1;
                }
            }

            if (offset_encontrado != -1)
            {
                fseek(binario, offset_encontrado, SEEK_SET);
                ler_registro(binario, &reg);

                // encontrar o id nao basta: os outros campos da busca tambem precisam coincidir
                if (reg.removido == '0' && verificar_criterios(&C, &reg))
                {
                    // manter a propria chave e permitido, mas ocupar a de outro pop nao
                    if (NovosValores.buscar_id &&
                        (NovosValores.idPoPs == -1 ||
                         (NovosValores.idPoPs != reg.idPoPs &&
                          buscar_no_indice(vetor_indices, total_indices, NovosValores.idPoPs) != -1)))
                    {
                        erro_atualizacao = 1;
                    }
                    if (!erro_atualizacao)
                    {
                        int id_antigo = reg.idPoPs;
                        modifica_registro(&reg, &NovosValores);

                        int tamanho_nome = strlen(reg.nomePoPs);
                        int tamanho_pais = strlen(reg.nomePais);
                        int tamanho_novo = TAM_MINIMO_REGISTRO + tamanho_nome + tamanho_pais;

                        if (tamanho_novo <= reg.tamanhoRegistro)
                        {
                            // conserva o tamanho do espaco; o escritor completa a sobra com lixo
                            fseek(binario, offset_encontrado, SEEK_SET);
                            escrever_registro(binario, &reg);

                            if (id_antigo != reg.idPoPs) {
                                retirar_do_indice(vetor_indices, &total_indices, posicao_indice);
                                inserir_no_indice(vetor_indices, &total_indices, reg.idPoPs, offset_encontrado);
                            }
                        }
                        else
                        {
                            // libera o espaco antigo e retira sua entrada antes da reinsercao
                            remover_registro(binario, &cab, &reg, offset_encontrado);
                            retirar_do_indice(vetor_indices, &total_indices, posicao_indice);

                            // a insercao escolhe outro espaco e devolve o offset para o indice
                            reg.tamanhoRegistro = tamanho_novo;
                            int64_t nova_pos = inserir_pop(binario, &cab, &reg);
                            inserir_no_indice(vetor_indices, &total_indices, reg.idPoPs, nova_pos);
                        }
                    }
                }
                liberar_registro(&reg);

            }
        }
        else
        {
            // fixa os registros selecionados antes de mover qualquer um deles
            int64_t *posicoes = malloc((cab.nroPoPs + 1) * sizeof(int64_t));
            int quantidade_posicoes = 0;
            int64_t offset_atual = TAM_CABECALHO;
            fseek(binario, offset_atual, SEEK_SET);
            if (posicoes == NULL)
                erro_atualizacao = 1;
            while (!erro_atualizacao && ler_registro(binario, &reg))
            {
                if (reg.removido == '0' && verificar_criterios(&C, &reg))
                    posicoes[quantidade_posicoes++] = offset_atual;
                liberar_registro(&reg);
                offset_atual = ftell(binario);
            }

            // uma unica chave nova nao pode ser atribuida a varios registros
            if (!erro_atualizacao && quantidade_posicoes > 0 && NovosValores.buscar_id)
            {
                int pos_novo_id = buscar_no_indice(vetor_indices, total_indices, NovosValores.idPoPs);
                if (NovosValores.idPoPs == -1 || quantidade_posicoes > 1 ||
                    (pos_novo_id != -1 && vetor_indices[pos_novo_id].byteoffset != posicoes[0]))
                    erro_atualizacao = 1;
            }

            // offsets fixados impedem que uma copia movida adiante seja atualizada outra vez
            for (int j = 0; j < quantidade_posicoes && !erro_atualizacao; j++)
            {
                offset_atual = posicoes[j];
                fseek(binario, offset_atual, SEEK_SET);
                ler_registro(binario, &reg);
                if (reg.removido == '0' && verificar_criterios(&C, &reg))
                {

                    int id_antigo = reg.idPoPs;
                    modifica_registro(&reg, &NovosValores);

                    int tamanho_nome = strlen(reg.nomePoPs);
                    int tamanho_pais = strlen(reg.nomePais);
                    int tamanho_novo = TAM_MINIMO_REGISTRO + tamanho_nome + tamanho_pais;

                    if (tamanho_novo <= reg.tamanhoRegistro)
                    {
                        fseek(binario, offset_atual, SEEK_SET);
                        escrever_registro(binario, &reg);

                        if (id_antigo != reg.idPoPs) {
                            int pos_idx = buscar_no_indice(vetor_indices, total_indices, id_antigo);
                            retirar_do_indice(vetor_indices, &total_indices, pos_idx);
                            inserir_no_indice(vetor_indices, &total_indices, reg.idPoPs, offset_atual);
                        }
                    }
                    else
                    {
                        // o registro maior passa pela lista de removidos e pela insercao worst fit
                        remover_registro(binario, &cab, &reg, offset_atual);

                        int pos_idx = buscar_no_indice(vetor_indices, total_indices, id_antigo);
                        retirar_do_indice(vetor_indices, &total_indices, pos_idx);

                        reg.tamanhoRegistro = tamanho_novo;
                        int64_t nova_pos = inserir_pop(binario, &cab, &reg);
                        inserir_no_indice(vetor_indices, &total_indices, reg.idPoPs, nova_pos);
                    }
                }
                liberar_registro(&reg);

            }
            free(posicoes);
        }
        // descarta tanto os textos da busca quanto os novos valores desta solicitacao
        liberar_criterios(&C);
        liberar_criterios(&NovosValores);
    }

    if (!salvar_indice(arquivo_index, vetor_indices, total_indices))
    {
        fclose(binario);
        free(vetor_indices);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // o cabecalho so fica consistente depois da gravacao bem-sucedida do indice
    cab.status = '1';
    escrever_cabecalho(binario, &cab);

    fclose(binario);
    free(vetor_indices);

    if (erro_atualizacao)
        printf("Falha no processamento do arquivo.\n");
    else
    {
        BinarioNaTela(arquivo_binario);
        BinarioNaTela(arquivo_index);
    }
}
