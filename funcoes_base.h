#ifndef FUNCOES_BASE_H   
#define FUNCOES_BASE_H

void BinarioNaTela(char *arquivo_binario);
void ScanQuoteString(char *str);
FILE *verificar_arquivo(char *arquivo_binario,char *modo_de_leitura);
RegCabecalho Leitura_Cabecalho(FILE *binario);
int Leitura_Registro(FILE *binario, Registro *Reg);

#endif