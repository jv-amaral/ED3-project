#include <stdlib.h>
#include <string.h>
#include "busca.h"
#include "leitura.h"
#include "indice.h"

// le os pares campo e valor da entrada padrao e retorna os criterios preenchidos.
// aloca os valores de texto; eles devem ser liberados com liberar_criterios.
CriteriosPoPs ler_criterios(int quantidade)
{
    CriteriosPoPs criterios = {0};

    for (int i = 0; i < quantidade; i++)
    {
        char campo[30];
        scanf("%29s", campo);
        char *valor = ler_valor();
        if (valor == NULL)
            break;

        if (strcmp(campo, "idPoPs") == 0)
        {
            criterios.buscar_id = 1;
            criterios.idPoPs = valor[0] == '\0' ? -1 : atoi(valor);
            free(valor);
        }
        else if (strcmp(campo, "nomePoPs") == 0)
        {
            free(criterios.nomePoPs);
            criterios.nomePoPs = valor;
        }
        else if (strcmp(campo, "nomePais") == 0)
        {
            free(criterios.nomePais);
            criterios.nomePais = valor;
        }
        else if (strcmp(campo, "siglaPais") == 0)
        {
            free(criterios.siglaPais);
            criterios.siglaPais = valor;
        }
        else
            free(valor);
    }
    return criterios;
}

// retorna 1 quando o registro atende a todos os criterios, ou 0 caso contrario.
// nao altera o registro nem o cursor; diferencia campo ausente de busca por nulo.
int verificar_criterios(CriteriosPoPs *criterios, RegistroPoP *registro)
{
    if (criterios->buscar_id && criterios->idPoPs != registro->idPoPs)
        return 0;
    if (criterios->nomePoPs != NULL && strcmp(criterios->nomePoPs, registro->nomePoPs) != 0)
        return 0;
    if (criterios->nomePais != NULL && strcmp(criterios->nomePais, registro->nomePais) != 0)
        return 0;
    if (criterios->siglaPais != NULL)
    {
        char *sigla = registro->siglaPais[0] == '$' ? "" : registro->siglaPais;
        if (strcmp(criterios->siglaPais, sigla) != 0)
            return 0;
    }
    return 1;
}

// busca o id no vetor e le seu registro por acesso direto com fseek.
// retorna a posicao no indice ou -1 se nao atender a todos os criterios.
// em sucesso, deixa o cursor apos o registro e os nomes devem ser liberados.
int buscar_registro_indexado(FILE *arquivo, IndicePoPs *indice, int quantidade,
                            CriteriosPoPs *criterios, RegistroPoP *registro)
{
    int posicao = buscar_no_indice(indice, quantidade, criterios->idPoPs);
    if (posicao == -1)
        return -1;

    fseek(arquivo, indice[posicao].byteoffset, SEEK_SET);
    if (!ler_registro(arquivo, registro))
        return -1;
    if (registro->removido == '0' && verificar_criterios(criterios, registro))
        return posicao;

    liberar_registro(registro);
    return -1;
}

// libera os textos dos criterios; nao retorna valor e nao acessa arquivos.
void liberar_criterios(CriteriosPoPs *criterios)
{
    free(criterios->nomePoPs);
    free(criterios->nomePais);
    free(criterios->siglaPais);
}

// le os pares de campos das conexoes e retorna criterios ou valores de atualizacao.
// consome a entrada padrao, libera os textos lidos e nao movimenta arquivos.
CriteriosConexoes ler_criterios_conexoes(int quantidade)
{
    CriteriosConexoes criterios = {-2, -2, -2, -2};

    for (int i = 0; i < quantidade; i++)
    {
        char campo[30];
        scanf("%29s", campo);
        char *valor = ler_valor();
        int numero = valor[0] == '\0' ? -1 : atoi(valor);

        if (strcmp(campo, "idPoPs") == 0)
            criterios.idPoPs = numero;
        else if (strcmp(campo, "idPoPsConectado") == 0)
            criterios.idPoPsConectado = numero;
        else if (strcmp(campo, "velocidade") == 0)
            criterios.velocidade = numero;
        else if (strcmp(campo, "unidadeMedida") == 0)
            criterios.unidadeMedida = valor[0] == '\0' ? '$' : valor[0];
        free(valor);
    }
    return criterios;
}

// retorna 1 se a conexao satisfaz todos os campos informados, ou 0 caso contrario.
// nao altera os parametros e nao movimenta arquivos.
int verificar_conexao(CriteriosConexoes *criterios, RegistroConexao *registro)
{
    if (criterios->idPoPs != -2 && criterios->idPoPs != registro->idPoPs)
        return 0;
    if (criterios->idPoPsConectado != -2 && criterios->idPoPsConectado != registro->idPoPsConectado)
        return 0;
    if (criterios->velocidade != -2 && criterios->velocidade != registro->velocidade)
        return 0;
    if (criterios->unidadeMedida != -2 && criterios->unidadeMedida != registro->unidade_medida)
        return 0;
    return 1;
}
