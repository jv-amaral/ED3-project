#ifndef BUSCA_H
#define BUSCA_H

#include <stdio.h>
#include "../estruturas.h"

// ponteiro nulo significa campo ausente; string vazia significa busca por nulo
typedef struct
{
    int buscar_id;
    int idPoPs;
    char *nomePoPs;
    char *nomePais;
    char *siglaPais;
} CriteriosPoPs;

CriteriosPoPs ler_criterios(int quantidade);
int verificar_criterios(CriteriosPoPs *criterios, RegistroPoP *registro);
int buscar_registro_indexado(FILE *arquivo, IndicePoPs *indice, int quantidade, CriteriosPoPs *criterios, RegistroPoP *registro);
void liberar_criterios(CriteriosPoPs *criterios);

// nas conexoes, -2 indica campo ausente e -1 indica inteiro nulo
typedef struct
{
    int idPoPs;
    int idPoPsConectado;
    int velocidade;
    int unidadeMedida;
} CriteriosConexoes;

CriteriosConexoes ler_criterios_conexoes(int quantidade);
int verificar_conexao(CriteriosConexoes *criterios, RegistroConexao *registro);

#endif
