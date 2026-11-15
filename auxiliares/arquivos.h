#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include <stdio.h>
#include "../estruturas.h"

FILE *abrir_arquivo(char *nome, char *modo);
CabecalhoPoPs ler_cabecalho(FILE *arquivo);
void escrever_cabecalho(FILE *arquivo, CabecalhoPoPs *cabecalho);
void alterar_status(FILE *arquivo, char status);
CabecalhoConexoes ler_cabecalho_conexoes(FILE *arquivo);
void escrever_cabecalho_conexoes(FILE *arquivo, CabecalhoConexoes *cabecalho);
void BinarioNaTela(char *nome);

#endif
