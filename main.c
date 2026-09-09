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
        case 2: recuperacao_dados();
                break;
        case 6: insercao();     
                break;
    }
    
    
    return 0;
}