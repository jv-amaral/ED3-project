#ifndef LEITURA_H
#define LEITURA_H

#include <stdio.h>
#include "../estruturas.h"

int ler_linha_csv(FILE *csv, char **linha, size_t *capacidade);
int separar_campos_csv(char *linha, char *campos[], int max_campos);
int ler_registro(FILE *arquivo, RegistroPoP *registro);
void liberar_registro(RegistroPoP *registro);
char *ler_valor(void);
void ScanQuoteString(char *str);
int ler_conexao(FILE *arquivo, RegistroConexao *registro);
RegistroConexao ler_conexao_terminal(void);
RegistroPoP ler_pop_terminal(void);

#endif
