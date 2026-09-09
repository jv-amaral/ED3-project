//arquivo de cabecalho para as funcoes do projeto

//evita que o conteudo do arquivo seja incluido mais de uma vez no programa
#ifndef FUNCOES_H   
#define FUNCOES_H

//definicao do registro do cabecalho
typedef struct {
    char status;
    int topo_pilha;
    int proxRRN;
    int nroRegRem;
    int nroPares;

}RegCabecalho;

//struct de registro de dados
typedef struct 
{
    char removido;
    int encadeamento;
    int idPoPs;
    int idPoPsConectado;
    int velocidade;
    char unidade_medida;

}Registro;


//assinaturas de funcao
void leitura_e_gravacao();
void recuperacao_dados();
void insercao();
void BinarioNaTela(char *arquivo_binario);
void ScanQuoteString(char *str);
#endif
