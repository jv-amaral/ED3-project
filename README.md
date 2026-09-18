# ED3-project

# conectaPoPs — Gerenciamento de registros em arquivo binário

Primeiro Trabalho Prático da disciplina de Estrutura de Dados III, do Instituto de Ciências Matemáticas e de Computação da Universidade de São Paulo (ICMC-USP).

O projeto implementa, em linguagem C, operações de criação, recuperação, busca, remoção lógica, inserção e atualização de registros de conexões entre pontos de presença de uma rede. Os dados são armazenados em arquivo binário com registros de tamanho fixo, sem índices auxiliares.

## Autores

| Integrante | Número USP |
| --- | --- |
| João Vitor Amaral Santos | 16812120 |
| Victor Hugo Albertino e Silva | 16903448 |

## Organização do projeto

| Arquivo | Responsabilidade |
| --- | --- |
| `main.c` | Lê o identificador da operação e encaminha a execução para uma das sete funcionalidades. |
| `funcionalidades.c` | Implementa o fluxo de cada operação e o processamento das entradas. |
| `funcionalidades.h` | Define as estruturas de cabeçalho e registro e declara as funcionalidades. |
| `funcoes_base.c` | Centraliza leitura, escrita, impressão, critérios de seleção e manipulação da pilha de removidos. |
| `funcoes_base.h` | Define a estrutura de critérios e declara as funções auxiliares. |
| `Makefile` | Oferece os alvos de compilação, execução e limpeza. |
| `README.md` | Documenta a estrutura, o uso e as limitações da implementação. |
| `conectaPoPs.csv` | Conjunto de entrada para a criação do binário, fornecido separadamente pela disciplina. |

Os arquivos devem manter os nomes utilizados pelos `#include`. Sufixos acrescentados ao baixar anexos, como `(2)` e `(3)`, devem ser removidos na organização local do projeto.

## Compilação e execução

Requisitos: compilador GCC e utilitário Make. O formato de arquivo pressupõe `char` de 1 byte e `int` de 4 bytes, conforme o ambiente do trabalho.

Para compilar:

```bash
make
```

Para executar:

```bash
make run
```

Também é possível executar diretamente:

```bash
./programaTrab
```

Para remover o executável:

```bash
make clean
```

O programa recebe os comandos pela entrada padrão e executa uma funcionalidade por processo. A primeira informação é o número da funcionalidade. Operações sucessivas sobre um mesmo arquivo binário devem ser feitas em execuções separadas.

## Funcionalidades

| Código | Operação | Estratégia |
| --- | --- | --- |
| 1 | Criar arquivo binário a partir de CSV | Leitura das linhas e escrita dos campos de cada registro. |
| 2 | Recuperar todos os registros ativos | Percurso sequencial e impressão dos registros não removidos. |
| 3 | Buscar por um ou mais critérios | Varredura sequencial com conjunção dos critérios informados. |
| 4 | Buscar por RRN | Acesso direto ao deslocamento do registro. |
| 5 | Remover registros logicamente | Marcação e inclusão dos espaços em uma pilha de removidos. |
| 6 | Inserir registros | Reaproveitamento do topo da pilha ou acréscimo ao final do arquivo. |
| 7 | Atualizar registros | Busca sequencial e sobrescrita dos registros com os campos alterados. |

### 1. Criação

```text
1 conectaPoPs.csv conectaPoPs.bin
```

O CSV possui uma linha de títulos e quatro colunas: `idPoPs`, `idPoPsConectado`, `velocidade` e `unidadeMedida`. O arquivo binário de saída é criado ou sobrescrito.

### 2. Recuperação de todos os registros

```text
2 conectaPoPs.bin
```

Os registros ativos são apresentados na ordem física do arquivo.

### 3. Busca condicional

```text
3 conectaPoPs.bin 2
1 idPoPs 73
2 velocidade 100 unidadeMedida "G"
```

O número após o nome do arquivo indica a quantidade de buscas. Cada busca começa com a quantidade de pares campo/valor. Todos os critérios de uma mesma busca devem ser atendidos pelo registro.

### 4. Busca por RRN

```text
4 conectaPoPs.bin 10
```

O primeiro registro possui RRN 0. Um registro removido ou um RRN além do final do arquivo resulta em `Registro inexistente.`.

### 5. Remoção lógica

```text
5 conectaPoPs.bin 2
1 velocidade 100
2 idPoPs 10 idPoPsConectado 20
```

Cada solicitação utiliza a mesma forma de seleção da funcionalidade 3. Uma busca sem resultados não impede a execução das solicitações seguintes.

### 6. Inserção

```text
6 conectaPoPs.bin 2
1001 1002 200 "G"
1003 1004 NULO NULO
```

Cada novo registro contém, nesta ordem, `idPoPs`, `idPoPsConectado`, `velocidade` e `unidadeMedida`. Os identificadores de cada par devem respeitar as restrições do trabalho.

### 7. Atualização

```text
7 conectaPoPs.bin 1
1 idPoPs 1001
2 velocidade 300 unidadeMedida "M"
```

Cada solicitação contém uma lista de critérios de busca seguida de uma lista dos campos e valores a atualizar. Campos não informados são preservados. Registros removidos não são atualizados.

## Formato de armazenamento

O arquivo contém um cabeçalho de 17 bytes e registros de dados de 18 bytes. A escrita é realizada campo a campo, sem gravar as estruturas C inteiras; assim, o preenchimento interno das estruturas não é incluído no arquivo.

### Cabeçalho

| Campo | Deslocamento em bytes | Tamanho |
| --- | ---: | ---: |
| `status` | 0 | 1 byte |
| `topoPilha` | 1 | 4 bytes |
| `proxRRN` | 5 | 4 bytes |
| `nroRegRem` | 9 | 4 bytes |
| `nroPares` | 13 | 4 bytes |

`status` indica consistência; `topoPilha` contém o RRN do topo da pilha, ou -1 quando vazia; `proxRRN` identifica a próxima posição ao final; `nroRegRem` conta os registros removidos.

O tratamento de `nroPares` desta versão preserva a adaptação informada pelos autores a partir da orientação da docente para o corretor. Não se deve reinterpretar automaticamente esse campo como contador de registros ativos sem consultar essa orientação. Na implementação revisada, ele é incrementado na importação do CSV e na reutilização de um espaço removido, mas não na remoção lógica nem na inserção ao final.

### Registro de dados

Os deslocamentos abaixo são relativos ao início de cada registro.

| Campo | Deslocamento em bytes | Tamanho |
| --- | ---: | ---: |
| `removido` | 0 | 1 byte |
| `encadeamentoPilha` | 1 | 4 bytes |
| `idPoPs` | 5 | 4 bytes |
| `idPoPsConectado` | 9 | 4 bytes |
| `velocidade` | 13 | 4 bytes |
| `unidadeMedida` | 17 | 1 byte |

A posição de um registro é calculada por `17 + RRN * 18`. Os nomes internos `topo_pilha`, `encadeamento` e `unidade_medida` correspondem aos campos de controle e de unidade descritos no enunciado.

## Nulos, remoção e consistência

- Velocidade nula é armazenada como -1; unidade nula, como `$`.
- Na entrada dos comandos, valores nulos são representados por `NULO`.
- Na saída textual, nulos aparecem como `NULO`, sem aspas. Unidades preenchidas aparecem entre aspas, como `"G"`.
- Registros ativos possuem `removido = '0'`; registros removidos possuem `removido = '1'`.
- Na remoção, os 13 bytes de dados são preenchidos com `$`, preservando os campos de controle.
- A pilha de removidos segue a ordem LIFO: o último espaço removido é o primeiro reaproveitado.
- Operações de escrita marcam o arquivo com `status = '0'` e restauram `status = '1'` ao concluir.
- As funcionalidades 1, 5, 6 e 7 fecham o binário antes de chamar `BinarioNaTela()`.

As funções `BinarioNaTela()` e `ScanQuoteString()` estão identificadas no código como rotinas fornecidas pela plataforma. A primeira produz o somatório dos bytes dividido por 100, utilizado na comparação das saídas.

## Modularização

`ler_criterios()` e `verificar_encontro()` compartilham a seleção entre as funcionalidades 3, 5 e 7. `print_registro()` centraliza a apresentação nas funcionalidades 2, 3 e 4. `Leitura_Registro()`, `escreve_registro()`, `Leitura_Cabecalho()` e `escreve_cabecalho()` concentram a serialização dos campos.

A escrita isolada do status e a leitura do encadeamento da pilha acessam somente os campos necessários, sem exigir a leitura ou regravação de estruturas completas.

## Validação e limites da versão revisada

A versão recebida para revisão compilou pelo Makefile e com `-std=c11 -Wall -Wextra -Wpedantic`, sem avisos. Foram verificados importação do CSV, tamanho físico, consultas, nulos, remoção, pilha, reaproveitamento, inserção ao final, atualizações sucessivas e rejeição de arquivo ausente ou com status inconsistente.

Existe uma pendência reproduzida na funcionalidade 6: ao ler uma velocidade de um único algarismo por `ScanQuoteString()`, a rotina pode consumir o token da unidade. A entrada `10 20 5 "G"` foi gravada como `10 20 5 NULO`. Uma cópia de teste que leu a velocidade com `scanf("%19s", temp_velocidade)` e manteve `ScanQuoteString()` para a unidade passou pelos 31 controles da revisão; os arquivos-fonte enviados não foram modificados nessa revisão.

A implementação pressupõe entradas no formato esperado. Não oferece validação completa de CSV malformado, leituras parciais de arquivos, falhas de escrita ou valores fora do domínio. A estrutura de critérios mantém um valor por campo, e a sentinela -2 de `unidadeMedida` pressupõe um `char` com sinal. A funcionalidade 3 acrescenta uma linha vazia após cada busca.

Essas verificações locais não substituem os testes reservados nem os demais critérios de avaliação da disciplina.

## Preparação para entrega

A seção “Material para Entregar” do enunciado solicita código-fonte documentado e Makefile no ZIP. A composição mínima é:

- `main.c`
- `funcionalidades.c`
- `funcionalidades.h`
- `funcoes_base.c`
- `funcoes_base.h`
- `Makefile`

Mantenha esses arquivos na raiz do ZIP, com os nomes esperados pelos includes. O README e o CSV não são exigidos nessa relação; o CSV é necessário apenas para executar localmente a funcionalidade 1. Executáveis, binários de dados, resultados de testes e versões antigas dos fontes não precisam integrar a entrega.

O vídeo é uma entrega separada no e-disciplinas: deve ter até sete minutos, participação equilibrada de todos os integrantes, webcam e link acessível. Os critérios acadêmicos e as orientações posteriores da docente também devem ser observados.