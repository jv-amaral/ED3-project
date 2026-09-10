//Alunos:
//João Vitor Amaral Santos - 16812120
//Victor Hugo Albertino e Silva - 16903448


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "funcoes.h"

int main() {
    int funcionalidade;
    scanf("%d", &funcionalidade);

    switch (funcionalidade)
    {
        case 1: 
            leitura_e_gravacao();
            break;
        case 2: 
            recuperacao_dados();
            break;
        case 3:
            busca_condicional();
            break;
        case 6: 
            insercao();     
            break;
    }
    
    return 0;
}