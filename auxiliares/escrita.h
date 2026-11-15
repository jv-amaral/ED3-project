#ifndef ESCRITA_H
#define ESCRITA_H

#include <stdio.h>
#include "../estruturas.h"
#include "busca.h"

void escrever_registro(FILE *arquivo, const RegistroPoP *registro);
void imprimir_registro(RegistroPoP *registro);
void escrever_conexao(FILE *arquivo, RegistroConexao *registro);
void imprimir_conexao(RegistroConexao *registro);
void remover_registro(FILE *arquivo, CabecalhoPoPs *cabecalho, RegistroPoP *registro, int64_t posicao);
void remover_conexao(FILE *arquivo, CabecalhoConexoes *cabecalho, RegistroConexao *registro, int rrn);
void inserir_conexao(FILE *arquivo, CabecalhoConexoes *cabecalho, RegistroConexao *registro);
void aplicar_atualizacoes_conexao(RegistroConexao *registro, CriteriosConexoes *valores);

#endif
