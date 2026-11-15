#include <stdlib.h>
#include <string.h>
#include "busca.h"
#include "leitura.h"
#include "indice.h"

// auxiliares trab. 1

// le os campos usados na busca ou na atualizacao de conexoes.
// os valores devolvidos ficam com -2 nos campos que nao foram informados.
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

// confere os criterios da conexao; basta um campo diferente para devolver 0.
// quando todos coincidem, o resultado e 1.
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

// auxiliares trab. 2

// guarda os pares campo e valor de uma busca de pops.
// os textos ficam alocados nos criterios devolvidos e sao liberados apos a busca.
CriteriosPoPs ler_criterios(int quantidade)
{
    CriteriosPoPs criterios;
    criterios.buscar_id = 0;
    criterios.idPoPs = -1;
    criterios.nomePoPs = NULL;
    criterios.nomePais = NULL;
    criterios.siglaPais = NULL;

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

// confere os campos informados e devolve 1 se todos coincidirem com o pop.
// ponteiro nulo indica campo nao pedido; texto vazio indica busca por valor nulo.
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

// localiza o id no indice, vai ate seu offset e confere os outros criterios.
// devolve a posicao no vetor, ou -1 quando nao encontra; na leitura, o cursor passa pelo registro.
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

// descarta os textos que foram lidos para a consulta dos pops.
void liberar_criterios(CriteriosPoPs *criterios)
{
    free(criterios->nomePoPs);
    free(criterios->nomePais);
    free(criterios->siglaPais);
}
