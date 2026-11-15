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
    int tamanhoRegistro; // Tamanho total em disco, incluindo os campos de controle.
    int64_t encadeamentoLista;
    int idPoPs;
    char siglaPais[3]; // Dois bytes no arquivo; o terceiro e '\0' apenas em memoria.
    char *nomePoPs;
    char *nomePais;
} Registro;



typedef struct
{
    int idPoPs;
    int64_t byteoffset;

}RegIndex;

void create_table(void);
void create_index();
void busca();

#endif
