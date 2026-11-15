//Alunos:
//João Vitor Amaral Santos - 16812120
//Victor Hugo Albertino e Silva - 16903448

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "funcionalidades2.h"
#include "funcoes_base2.h"


int main() {
    int funcionalidade;
    scanf("%d", &funcionalidade);

    //switch utilizado para selecionar qual funcionalidade deve ser executada
    switch (funcionalidade)
    {
       
        case 8:
            create_table();
            break;
        case 9:
            create_index();
            break;
        case 10:
                break;
        case 11:
            busca();
            break;
    }
    
    return 0;
}
