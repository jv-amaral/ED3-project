#ifndef FUNCIONALIDADES2_H
#define FUNCIONALIDADES2_H
#include <stdint.h>

typedef struct
{
    char status;
    int64_t topoLista;
    int64_t proxByteOffset;
    int nroRegRem;
    int nroPoPs;
} RegCabecalho;

typedef struct
{
    char removido;
    int tamanhoRegistro;
    int64_t encadeamentoLista;
    int idPoPs;
    char siglaPais[3];
    char *nomePoPs;
    char *nomePais;
} Registro;

#endif