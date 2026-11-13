#ifndef FUNCOES_BASE_H
#define FUNCOES_BASE_H

// struct para a funcao ler_criterios


// funcoes a serem utilizadas no funcionalidades.c
FILE *verificar_arquivo(char *arquivo_binario, char *modo_de_leitura);
RegCabecalho Leitura_Cabecalho2(FILE *binario);
int Leitura_Registro2(FILE *binario, Registro *Reg);
void escreve_registro2(FILE *binario, Registro *Reg);
void escreve_cabecalho2(FILE *binario, RegCabecalho *Cab);
void BinarioNaTela(char *arquivo_binario);
void ScanQuoteString(char *str);

#endif