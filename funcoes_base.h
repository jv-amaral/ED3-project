#ifndef FUNCOES_BASE_H
#define FUNCOES_BASE_H

//struct para a funcao ler_criterios
typedef struct
{
    int idPoPs;
    int idPoPsConectado;
    int velocidade;
    char unidadeMedida;
} Criterios;

FILE *verificar_arquivo(char *arquivo_binario, char *modo_de_leitura);
RegCabecalho Leitura_Cabecalho(FILE *binario);
int Leitura_Registro(FILE *binario, Registro *Reg);
Criterios ler_criterios(int repeticoes); // arrumar
int escreve_arquivo(FILE *binario, Registro *Reg);
void BinarioNaTela(char *arquivo_binario);
void ScanQuoteString(char *str);


#endif