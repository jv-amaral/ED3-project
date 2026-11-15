#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#include <stdint.h>

// tamanhos em disco, sem o padding das estruturas em memoria
// cabecalho dos pops: status(1) + topo(8) + proximo offset(8) + dois contadores(4 + 4)
#define TAM_CABECALHO 25
// marca(1) e tamanho(4); o encadeamento de oito bytes completa os controles
#define TAM_CONTROLE_INICIAL 5
#define TAM_CONTROLE_REGISTRO 13
// controles(13) + identificador(4) + sigla(2)
#define TAM_FIXO_REGISTRO 19
// parte fixa(19) + dois delimitadores(1 + 1), mesmo quando os nomes sao nulos
#define TAM_MINIMO_REGISTRO 21
// no indice, o cabecalho e apenas o status; cada entrada tem id(4) e offset(8)
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

// conexoes: cabecalho de status(1) e quatro inteiros(16)
#define TAM_CABECALHO_CONEXOES 17
// registro: marca(1), quatro inteiros(16) e unidade(1); controles ocupam 1 + 4
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
