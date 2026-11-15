#ifndef INDICE_H
#define INDICE_H

#include <stdio.h>
#include "../estruturas.h"

IndicePoPs *carregar_indice(FILE *arquivo, int *quantidade);
int buscar_no_indice(IndicePoPs *indice, int quantidade, int id);
int comparar_indices(const void *primeiro, const void *segundo);
void retirar_do_indice(IndicePoPs *indice, int *quantidade, int posicao);
int salvar_indice(char *nome, IndicePoPs *indice, int quantidade);
void inserir_no_indice(IndicePoPs *indice, int *quantidade, int id, int64_t posicao);

#endif
