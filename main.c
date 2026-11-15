// Alunos:
// João Vitor Amaral Santos - 16812120
// Victor Hugo Albertino e Silva - 16903448

#include <stdio.h>
#include "funcionalidades.h"
// le a opcao e encaminha a execucao para a funcionalidade escolhida.
int main(void)
{
    int funcionalidade;
    scanf("%d", &funcionalidade);

    // escolhe a operacao informada na entrada padrao
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
        case 5:
            remocao_logica();
            break;
        case 6:
            insercao();
            break;
        case 7:
            atualizacao_registros();
            break;
        case 8:
            create_table();
            break;
        case 9:
            create_index();
            break;
        case 10:
            listar_registros();
            break;
        case 11:
            busca();
            break;
        case 12:
            remover_registros();
            break;
        case 13:
            inserir_registros();
            break;
        case 14:
            atualiza_registro();
    }

    return 0;
}
