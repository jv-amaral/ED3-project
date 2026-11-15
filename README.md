# projeto de estrutura de dados iii

Autores: João Vitor Amaral Santos (16812120) e Victor Hugo Albertino e Silva (16903448).

O programa reúne as funcionalidades de conexões entre pontos de presença (`conectaPoPs`, Trabalho 1) e de pontos de presença (`PoPs`, Trabalho 2). O código está em C e grava os arquivos binários campo a campo.

## estado atual do trabalho

As funcionalidades **1 a 12 estão implementadas**. As funcionalidades **13 e 14 ainda estão pendentes**; suas regras, exemplos e verificações finais estão documentados abaixo para orientar a conclusão. Esta atualização acrescenta comentários e documentação, sem implementar novas operações.

| Opção | Operação | Dados | Situação |
| --- | --- | --- | --- |
| 1 | Criar binário a partir de CSV | Conexões | Implementada |
| 2 | Listar registros ativos | Conexões | Implementada |
| 3 | Buscar por critérios | Conexões | Implementada |
| 4 | Buscar por RRN | Conexões | Implementada |
| 5 | Remover logicamente | Conexões | Implementada |
| 6 | Inserir reaproveitando a pilha | Conexões | Implementada |
| 7 | Atualizar campos | Conexões | Implementada |
| 8 | Criar binário a partir de CSV | PoPs | Implementada |
| 9 | Criar índice primário | PoPs | Implementada |
| 10 | Listar registros ativos | PoPs | Implementada |
| 11 | Buscar por critérios e índice | PoPs | Implementada |
| 12 | Remover e atualizar o índice | PoPs | Implementada |
| 13 | Inserir com Worst Fit e atualizar o índice | PoPs | Pendente |
| 14 | Atualizar com Worst Fit e atualizar o índice | PoPs | Pendente |

## organização dos arquivos

```text
main.c                   escolha da funcionalidade
estruturas.h             estruturas e tamanhos em disco
funcionalidades.h        declaracoes das funcionalidades
funcionalidades.c        funcionalidades 1 a 7
funcionalidades2.c       funcionalidades 8 a 12; futuras 13 e 14
auxiliares/
    arquivos.c / .h      abertura, cabecalhos, status e exibicao binaria
    leitura.c / .h       csv, terminal, leitura de registros e liberacao dos nomes
    escrita.c / .h       gravacao, impressao, remocao, insercao e atualizacao
    busca.c / .h         criterios das conexoes e dos pops
    indice.c / .h        vetor de indice, busca binaria e persistencia
```

Os dois formatos compartilham os cinco módulos auxiliares. As rotinas específicas de cada formato têm tipos diferentes; não há uma pasta separada para o Trabalho 1. As futuras funções 13 e 14 devem ser acrescentadas a `funcionalidades2.c`, com os auxiliares necessários nos módulos existentes e suas chamadas no `main.c`.

Os comentários do código usam letras minúsculas, sem acentos ou cedilha. Acima das funções, explicam finalidade, retorno e efeitos sobre o cursor ou a memória; dentro delas, explicam os principais blocos de processamento.

## compilação e execução

Requisitos: GCC e Make. O formato pressupõe inteiros de 4 bytes e inteiros de 64 bits com 8 bytes, conforme o ambiente do trabalho.

```sh
make
./programaTrab
```

Também é possível iniciar com `make run`. O comando `make clean` remove apenas o executável. O Makefile compila os fontes da raiz e de `auxiliares/`.

Cada execução recebe **uma funcionalidade** pela entrada padrão. Para usar outro comando, execute o programa novamente. Operações de escrita podem modificar o mesmo binário em execuções sucessivas.

## entradas e saídas comuns

- Strings dos comandos são informadas entre aspas duplas, como `"CAMPINA GRANDE"`.
- Valores nulos nos comandos são informados com `NULO`, sem aspas.
- No CSV, campos vazios ou compostos apenas de espaços representam nulos.
- Na mesma busca, todos os pares campo/valor precisam ser satisfeitos.
- Uma busca ou remoção sem resultado não impede o processamento das solicitações seguintes.
- A mensagem de falha é `Falha no processamento do arquivo.`.
- As consultas sem resultado exibem `Registro inexistente.`. A remoção 12 exibe os binários finais mesmo se não houver registros correspondentes.
- As operações que exigem saída binária fecham os arquivos antes de chamar `BinarioNaTela`.

Nos exemplos abaixo, cada bloco representa a entrada de uma execução separada. Os nomes de arquivos são exemplos e podem ser ajustados.

## funcionalidades 1 a 7: conexões

O CSV de conexões possui os campos `idPoPs`, `idPoPsConectado`, `velocidade` e `unidadeMedida`. O cabeçalho ocupa 17 bytes e cada registro ocupa 18 bytes.

### 1. criar o binário

```text
1 conectaPoPs.csv conectaPoPs.bin
```

Cria ou sobrescreve o arquivo de saída e exibe sua soma binária.

### 2. listar os ativos

```text
2 conectaPoPs.bin
```

Exibe `idPoPs idPoPsConectado velocidade unidadeMedida`. Unidades preenchidas aparecem entre aspas; campos nulos aparecem como `NULO`.

### 3. buscar por critérios

```text
3 conectaPoPs.bin 2
1 idPoPs 73
2 velocidade 100 unidadeMedida "G"
```

O número após o arquivo indica a quantidade de buscas. Cada busca começa com a quantidade de pares campo/valor. A versão atual preserva uma linha em branco após cada busca, como na base original.

### 4. buscar por RRN

```text
4 conectaPoPs.bin 0
```

O primeiro registro tem RRN 0. RRN negativo, removido ou fora do intervalo disponível não é exibido.

### 5. remover logicamente

```text
5 conectaPoPs.bin 2
1 velocidade 100
2 idPoPs 10 idPoPsConectado 20
```

Marca os registros e empilha seus RRNs. Os dados removidos são preenchidos com `$`. Ao final, exibe o binário atualizado.

### 6. inserir conexões

```text
6 conectaPoPs.bin 2
1001 1002 200 "G"
1003 1004 NULO NULO
```

A ordem de entrada é `idPoPs idPoPsConectado velocidade unidadeMedida`. Reaproveita o topo da pilha antes de acrescentar registros ao final e exibe o binário atualizado.

### 7. atualizar conexões

```text
7 conectaPoPs.bin 1
1 idPoPs 1001
2 velocidade 300 unidadeMedida "M"
```

Cada atualização contém uma lista de critérios de busca seguida de uma lista dos novos valores. Campos não informados são preservados. Ao final, exibe o binário atualizado.

## funcionalidades 8 a 12: PoPs

A criação identifica as colunas do CSV pelo cabeçalho. O arquivo fornecido possui uma coluna inicial de numeração, que é ignorada. Os quatro campos de dados são `idPoPs`, `nomePoPs`, `nomePais` e `siglaPais`.

### 8. criar o binário de PoPs

```text
8 PoPs.csv PoPs.bin
```

Cria ou sobrescreve o binário, grava os registros de tamanho variável e exibe sua soma binária.

### 9. criar o índice primário

```text
9 PoPs.bin indexaPoPs.bin
```

Indexa somente registros ativos. O índice contém pares de identificador e byte offset, ordenados por identificador. A saída é a soma binária do índice.

### 10. listar os PoPs ativos

```text
10 PoPs.bin
```

A ordem de saída é `idPoPs nomePoPs nomePais siglaPais`, um registro por linha. Nulos aparecem como `NULO` e os textos não recebem aspas na saída.

### 11. buscar PoPs

```text
11 PoPs.bin indexaPoPs.bin 2
1 idPoPs 23
2 nomePais "GERMANY" siglaPais "DE"
```

Se houver critério de `idPoPs`, usa busca binária no vetor do índice e acesso direto ao registro; os demais critérios também são conferidos. Sem identificador, percorre o arquivo sequencialmente.

### 12. remover PoPs

```text
12 PoPs.bin indexaPoPs.bin 2
1 nomePoPs "RECIFE"
3 nomePoPs "FRANKFURT" nomePais "GERMANY" siglaPais "DE"
```

Preenche os dados com `$`, inclui o espaço na lista de removidos e retira a chave do índice. A lista permanece ordenada do maior espaço para o menor, para Worst Fit. Em empate de tamanho, a implementação atual coloca o removido mais recente antes. A saída exibe primeiro o binário de dados e depois o índice.

## funcionalidades pendentes para concluir o Trabalho 2

**Os comandos desta seção são a sintaxe prevista pelo enunciado; ainda não estão disponíveis no programa.**

### 13. inserção com Worst Fit — pendente

```text
13 PoPs.bin indexaPoPs.bin 2
3 "CAMPINA GRANDE" "BRAZIL" "BR"
22 "TERESINA" NULO NULO
```

A entrada contém `idPoPs nomePoPs nomePais siglaPais`. A implementação deverá:

- Calcular o tamanho necessário e procurar o maior espaço removido, no topo da lista ordenada.
- Reaproveitar esse espaço se couber, retirando-o da lista e preenchendo a sobra com `$`; se não couber, acrescentar ao final.
- Atualizar o topo, os contadores e o próximo byte offset quando necessário.
- Inserir o identificador e seu offset no índice, mantendo a ordem crescente.
- Manter os status de dados e índice consistentes ao terminar e exibir os dois binários, dados primeiro.

### 14. atualização com Worst Fit — pendente

```text
14 PoPs.bin indexaPoPs.bin 1
1 idPoPs 3
2 nomePoPs "PARIS" nomePais "FRANCE"
```

Cada solicitação contém os critérios de busca e, na linha seguinte, os campos a atualizar. A implementação deverá:

- Usar a mesma seleção da função 11, inclusive o índice quando houver `idPoPs`.
- Preservar campos não informados e aceitar `NULO` para os campos que permitem nulos.
- Atualizar no mesmo espaço se o registro resultante couber, com `$` no final da sobra.
- Se crescer além do espaço disponível, remover logicamente o registro antigo e reinseri-lo por Worst Fit.
- Atualizar identificador e offset no índice quando mudarem e processar cada registro selecionado uma vez por solicitação.
- Continuar após uma solicitação sem resultados e exibir os dois binários ao terminar.

As novas rotinas devem reutilizar a leitura, os critérios, a escrita de registros, a lista de removidos e o índice já existentes. Não é necessário criar mais categorias de auxiliares.

## formatos binários e consistência

| Característica | Conexões | PoPs |
| --- | --- | --- |
| Cabeçalho | 17 bytes | 25 bytes |
| Registro de dados | 18 bytes fixos | 21 bytes mais os nomes, sem contar eventual sobra |
| Endereço | RRN | Byte offset |
| Espaços removidos | Pilha LIFO | Lista ordenada por tamanho para Worst Fit |
| Índice auxiliar | Não utiliza | Cabeçalho de 1 byte e entradas de 12 bytes |

Nas conexões, o início de um registro é `17 + RRN * 18`. Nos PoPs, `tamanhoRegistro` inclui o registro inteiro: 19 bytes fixos, dois delimitadores e os nomes. Os campos variáveis terminam em `|`, sem gravar `\0`; a sigla ocupa exatamente dois bytes. Sigla nula é `$$`; nome nulo é representado apenas por seu delimitador.

Os registros removidos são pulados com `fseek`, sem carregar seus dados campo a campo. O leitor também pula o lixo ao final de um registro ativo reaproveitado.

No início de uma alteração, grava-se somente o status `'0'`. Na finalização dos dados, o status é alterado para `'1'` em memória **antes de gravar o cabeçalho completo uma única vez**. Não se grava o cabeçalho final para depois voltar e modificar apenas esse byte. Na criação, o cabeçalho inicial reserva o início do arquivo. No índice, o cabeçalho inteiro é apenas o próprio byte de status.

A regra de `nroPares` das conexões foi preservada da base original: incrementa na criação e na reutilização de um espaço removido; não muda na remoção nem na inserção ao final. Não foi reinterpretado como contador de registros ativos.

Nos PoPs, a remoção diminui `nroPoPs`, aumenta `nroRegRem` e mantém `proxByteOffset`. A quantidade de removidos e o encadeamento precisam permanecer coerentes também após as futuras inserções e atualizações.

## testes e revisão até a conclusão

A última bateria de testes de implementação registrou:

- 47 operações das funções 1 a 7 comparadas à base original, conferindo saída e bytes dos arquivos; mais cinco consultas de RRN negativo e 12 casos de arquivo ausente ou inconsistente.
- 28 cenários das funções 8 a 12, cobrindo CSV real, consultas, nulos, nomes longos, índice, remoção, lista Worst Fit, empates e espaços com sobra.
- Execução normal e com sanitizadores de endereço e comportamento indefinido; não foi executada a detecção de vazamentos.

Nesta revisão de documentação, foi verificado que o conteúdo executável de `funcionalidades2.c` permaneceu igual e foi feita verificação sintática com `-Wall -Wextra -Wpedantic`, sem avisos. Não houve nova implementação de funcionalidades, nem submissão ao run.codes.

Depois de implementar 13 e 14, verificar:

- Inserção ao final, em espaço maior, em espaço exato e quando o maior removido ainda é pequeno demais.
- Inserções e atualizações seguidas, com lista vazia, um elemento, vários elementos e empate de tamanho.
- Atualização menor, do mesmo tamanho e maior; alteração do próprio identificador ou do campo usado na busca.
- Preservação do índice ordenado, dos offsets, dos contadores e dos status em todas as operações.
- Campos nulos, nenhum resultado, vários resultados, todos removidos e registros ativos com lixo ao final.
- O fluxo completo `8 → 9 → 13 → 14 → 12 → 10/11`, em execuções separadas, e os testes disponíveis da disciplina.

## preparação da entrega final

O trabalho ainda precisa da implementação e validação das funções 13 e 14 para ser considerado concluído.

Para fechar a entrega, seguir esta ordem:

- Concluir 13 e 14, declarar suas funções em `funcionalidades.h` e acrescentar os casos no `main.c`.
- Atualizar o estado das funcionalidades, os exemplos e os testes deste README e do relatório somente depois de validar o código.
- Conferir nomes e números USP dos dois integrantes no comentário inicial de `main.c` e os comentários das funções.
- Compilar a partir dos fontes e verificar as saídas e os formatos exigidos pelo enunciado.
- Preparar o ZIP com os fontes `.c`, os cabeçalhos `.h` e o `Makefile`, preservando a pasta `auxiliares/`. CSVs, binários de teste e o executável não são necessários para compilar.
- Extrair o ZIP em uma pasta limpa e conferir `make` e `make run` antes da submissão ao run.codes.
- Gravar o vídeo de até sete minutos com os dois integrantes, webcam e participação equilibrada, explicando cada funcionalidade e sua implementação.
- Entregar o link do vídeo no e-disciplinas com os dados pedidos e conferir se o link pode ser acessado.

O relatório atualizado está em [RELATORIO_MUDANCAS.pdf](RELATORIO_MUDANCAS.pdf).
