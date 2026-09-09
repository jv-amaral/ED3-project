//arquivo de cabecalho para as funcoes do projeto
#ifndef FUNCOES_H
#define FUNCOES_H

typedef struct {
    char status;
    int topo_pilha;
    int proxRRN;
    int nroRegRem;
    int nroPares;

}RegCabecalho;

typedef struct 
{
    char removido;
    int encadeamento;
    int idPoPs;
    int idPoPsConectado;
    int velocidade;
    char unidade_medida;

}Registro;



void leitura_e_gravacao();
void recuperacao_dados();
void insercao();
void BinarioNaTela(char *arquivo_binario);
void ScanQuoteString(char *str);
#endif
