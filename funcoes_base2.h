#ifndef FUNCOES_BASE2_H
#define FUNCOES_BASE2_H

#include <stdio.h>
#include "funcionalidades2.h"

typedef struct
{
    int idPoPs;
    char siglaPais[5];
    char nomePoPs[50];
    char nomePais[50];
} Criterios;

FILE *verificar_arquivo(char *arquivo_binario, char *modo_de_leitura);
int Leitura_Registro2(FILE *binario, Registro *Reg);
void escreve_cabecalho2(FILE *binario, RegCabecalho *Cab);
int verificar_encontro(Criterios *C, Registro *Reg);
Criterios ler_criterios(int qtd_criterios);
void print_registro2(Registro *Reg);
int compara_index(const void *a, const void *b);

void BinarioNaTela(char *arquivo_binario);
void ScanQuoteString(char *str);

#endif