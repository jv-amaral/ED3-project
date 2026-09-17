//Alunos:
//João Vitor Amaral Santos - 16812120
//Victor Hugo Albertino e Silva - 16903448


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "funcionalidades.h"
#include "funcoes_base.h"

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
        case 4:
            busca_RRN();
            break;
    
        case 6: 
            insercao();     
            break;
        case 7:
            atualizacao_registros();
            break;
    }
    
    return 0;
}