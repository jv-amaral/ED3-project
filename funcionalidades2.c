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
// importa os pops do csv para o arquivo binario.
// a primeira linha indica onde esta cada campo, inclusive quando ha uma coluna extra.
// para cada linha de dados, monta um registro de tamanho variavel e atualiza os contadores.
// ao terminar, grava o cabecalho consistente e exibe o binario ja fechado.
void create_table(void)
{
    char nome_arq_entrada[100];
    char nome_arq_saida[100];

    // o csv fornece os dados; o arquivo de saida sera criado em formato binario
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

// func 9
// monta o indice com o id e o offset de cada pop ativo.
// as entradas sao ordenadas antes de gravar o arquivo de indice.
// o offset aponta para o inicio do registro de dados, e nao para o campo do identificador.
// apenas le o arquivo de dados; o novo arquivo criado e o de indice.
void create_index(void)
{
    char arquivo_binario[50]; // variaveis para os arquivos
    char arquivo_index[50];
    scanf("%s", arquivo_binario); // le o nome dos arquivos
    scanf("%s",arquivo_index);
      // declaracao da struct do registro
    RegistroPoP reg;

     FILE *binario = abrir_arquivo(arquivo_binario, "rb");
    if (binario == NULL) // se encontrar algum erro, ele retorna
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fseek(binario, 25, SEEK_SET);
    int64_t byteOffsetAtual = 25; // byteoffset atual depois de ler o registro de cabecalho
     int max_registros = 2000;
    IndicePoPs *vetor_indices = (IndicePoPs *) malloc(max_registros * sizeof(IndicePoPs)); // vetor para armazenar idpops e o byteoffset
     int i = 0;                                                                      // para ordenar depois
    while( fread(&reg.removido, sizeof(char), 1, binario)==1 )
    {
        fread(&reg.tamanhoRegistro,sizeof(int),1,binario);
        if(reg.removido == '0')
        {
            fread(&reg.encadeamentoLista, sizeof(int64_t),1,binario);
            fread(&reg.idPoPs, sizeof(int),1,binario);
            vetor_indices[i].idPoPs = reg.idPoPs;
            vetor_indices[i].byteoffset = byteOffsetAtual;
            i++;
            fseek(binario, reg.tamanhoRegistro - 17,SEEK_CUR);
        } else
        {
            fseek(binario, reg.tamanhoRegistro - 5 ,SEEK_CUR);
        }

        byteOffsetAtual = byteOffsetAtual + reg.tamanhoRegistro;

    }


    qsort(vetor_indices, i, sizeof(IndicePoPs), comparar_indices);
     fclose(binario);

    FILE *index = fopen(arquivo_index,"wb");
    if(index == NULL)
    {
        printf("Falha no processamento do arquivo.\n"); // mensagem padrao de erro
        free(vetor_indices);
        return;
    }
    char status_index = '0';
    fwrite(&status_index,sizeof(char),1,index);
    for ( int j = 0; j<i;j++)
    {
        fwrite(&vetor_indices[j].idPoPs,sizeof(int),1,index);
        fwrite(&vetor_indices[j].byteoffset,sizeof(int64_t),1,index);
    }
    status_index ='1';
    fseek(index,0,SEEK_SET);
    fwrite(&status_index,sizeof(char),1,index);
    fclose(index);
    free(vetor_indices);
    BinarioNaTela(arquivo_index);
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

// func 11
// usa o indice quando a busca tem idpops; nos outros casos faz a varredura.
// todos os campos pedidos precisam coincidir para mostrar o registro.
// cada solicitacao e independente, podendo encontrar nenhum, um ou varios pops.
// o indice fica em memoria durante as consultas e os dados sao abertos so para leitura.
void busca(void)
{
    char arquivo_binario[50]; // variaveis dos nomes dos arquivos
    char arquivo_index[50];

    scanf("%s",arquivo_binario);    //leitura dos nomes dos arquivos
    scanf("%s",arquivo_index);
    CabecalhoPoPs cab;
    RegistroPoP reg;           // criacao das structs de registro e de registro de indice

    FILE *binario = abrir_arquivo(arquivo_binario,"rb");     // verifica se os arquivos nao estao corrompidos
    FILE *index = abrir_arquivo(arquivo_index,"rb");
    if (binario == NULL) // se encontrar algum erro, ele retorna
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    if (index == NULL) // se encontrar algum erro, ele retorna
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fread(&cab.status,sizeof(char),1,binario);

    if (cab.status == '0')                    // verifica se o arquivo esta consistente
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    char status_index;
    fread(&status_index,sizeof(char),1,index);

    if (status_index == '0')            // verifica se o arquivo de indice esta consistente
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }
        // cria o vetor de indices para fazer a busca
       int max_indices = 2000;
         IndicePoPs *vetor_indices = (IndicePoPs *) malloc(max_indices * sizeof(IndicePoPs));
         int total_indices = 0;

         // enquanto conseguir ler um idpops (4 bytes), e porque tem um registro
         while (fread(&vetor_indices[total_indices].idPoPs, sizeof(int), 1, index) == 1)
        {
        // se leu o id, obrigatoriamente tem o byteoffset (8 bytes) logo na frente
        fread(&vetor_indices[total_indices].byteoffset, sizeof(int64_t), 1, index);
        total_indices++;
        }


         fclose(index);
    int n;          // declaracao e leitura da variavel que vai verificar quantas vezes sera feita a busca
    scanf("%d",&n);



    for (int i = 0; i<n;i++)            // loop principal da busca que sera feita n vezes
    {
        int m;
        scanf("%d",&m);
        CriteriosPoPs C = ler_criterios(m);
        int registro_encontrado;
        if (C.buscar_id)
        {



        int inicio = 0;
        int fim = total_indices - 1;
        int64_t offset_encontrado = -1;
            while (inicio <= fim) {
                int meio = (inicio + fim) / 2;

                if (vetor_indices[meio].idPoPs == C.idPoPs) {
                    offset_encontrado = vetor_indices[meio].byteoffset;
                    break; // sai do while na hora
                }
                else if (vetor_indices[meio].idPoPs < C.idPoPs) {
                    inicio = meio + 1;
                }
                else {
                    fim = meio - 1;
                }
            }
            if(offset_encontrado == -1)
            {
                printf("Registro inexistente.\n");
            }
            else
            {
                fseek(binario,offset_encontrado,SEEK_SET);
                ler_registro(binario,&reg);
                if (reg.removido == '0' && verificar_criterios(&C, &reg) == 1)
                {
                    imprimir_registro(&reg);
                    registro_encontrado = 1;
                }

                if (registro_encontrado == 0)
                {
                    printf("Registro inexistente.\n");
                }

            }



        }
        else
        {
            fseek(binario,25,SEEK_SET);

           while (ler_registro(binario, &reg) == 1)
            {
                if (reg.removido == '1') continue;

                if (verificar_criterios(&C, &reg) == 1)
                {
                    imprimir_registro(&reg);
                    registro_encontrado = 1;
                }
            }

            if (registro_encontrado == 0)
            {
                printf("Registro inexistente.\n");
            }
        }
    }




}

// func 12
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
