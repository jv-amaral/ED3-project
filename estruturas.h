#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#include <stdint.h>

// tamanhos em disco, sem o preenchimento interno das estruturas em memoria
#define TAM_CABECALHO 25
#define TAM_CONTROLE_INICIAL 5
#define TAM_CONTROLE_REGISTRO 13
#define TAM_FIXO_REGISTRO 19
#define TAM_MINIMO_REGISTRO 21
#define TAM_CABECALHO_INDICE 1
#define TAM_REGISTRO_INDICE 12

typedef struct
{
    char status;
    int64_t topoLista;
    int64_t proxByteOffset;
    int nroRegRem;
    int nroPoPs;
} CabecalhoPoPs;

typedef struct
{
    char removido;
    int tamanhoRegistro; // tamanho total do registro em disco
    int64_t encadeamentoLista;
    int idPoPs;
    char siglaPais[3]; // dois bytes no arquivo e terminador apenas em memoria
    char *nomePoPs;
    char *nomePais;
} RegistroPoP;

typedef struct
{
    int idPoPs;
    int64_t byteoffset;
} IndicePoPs;

// conexoes usam registros fixos e uma pilha de rrns removidos
#define TAM_CABECALHO_CONEXOES 17
#define TAM_REGISTRO_CONEXAO 18
#define TAM_CONTROLE_CONEXAO 5

typedef struct
{
    char status;
    int topo_pilha;
    int proxRRN;
    int nroRegRem;
    int nroPares;
} CabecalhoConexoes;

typedef struct
{
    char removido;
    int encadeamento;
    int idPoPs;
    int idPoPsConectado;
    int velocidade;
    char unidade_medida;
} RegistroConexao;

#endif
