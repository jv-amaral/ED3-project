#ifndef FUNCOES_BASE_H
#define FUNCOES_BASE_H

// struct para a funcao ler_criterios
typedef struct
{
    int idPoPs;
    int idPoPsConectado;
    int velocidade;
    char unidadeMedida;
} Criterios;

// funcoes a serem utilizadas no funcionalidades.c
FILE *verificar_arquivo(char *arquivo_binario, char *modo_de_leitura);
RegCabecalho Leitura_Cabecalho(FILE *binario);
int Leitura_Registro(FILE *binario, Registro *Reg);
Criterios ler_criterios(int repeticoes); // arrumar
void escreve_registro(FILE *binario, Registro *Reg);
void escreve_cabecalho(FILE *binario, RegCabecalho *Cab);
void print_registro();
void remove_registro(Registro *Reg, RegCabecalho *Cabecalho, int RRN_registro);
int verificar_encontro(Criterios *C, Registro *Reg);
void BinarioNaTela(char *arquivo_binario);
void ScanQuoteString(char *str);

#endif