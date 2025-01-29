# CGNR Project

Este projeto implementa o algoritmo CGNR (Conjugate Gradient Normal Residual) para a reconstrução de imagens a partir de dados de ultrassom. O objetivo é fornecer uma solução eficiente para a reconstrução de imagens, utilizando dados de entrada em formato CSV.

## Estrutura do Projeto

O projeto possui a seguinte estrutura de diretórios:

```
cgnr-project
├── src
│   ├── cgnr.cpp
│   └── utils
│       └── readData.cpp
├── data
│   ├── H-1.csv
│   ├── G-1.csv
│   └── G-2.csv
├── CMakeLists.txt
└── README.md
```

### Descrição dos Arquivos

- **src/cgnr.cpp**: Implementação principal do algoritmo CGNR. Responsável por ler os dados da matriz H e do vetor de sinal, além de executar o algoritmo de reconstrução de imagens.

- **src/utils/readData.cpp**: Contém a função `readData`, que lê os dados dos arquivos CSV e os armazena em estruturas de dados apropriadas, como vetores ou matrizes.

- **data/H-1.csv**: Matriz H utilizada como modelo no algoritmo CGNR, com dimensões 50816 x 3600.

- **data/G-1.csv**: Vetor de sinal 1, utilizado como entrada para o algoritmo CGNR.

- **data/G-2.csv**: Vetor de sinal 2, também utilizado como entrada para o algoritmo CGNR.

- **CMakeLists.txt**: Script de configuração para o CMake, especificando as fontes, bibliotecas necessárias e opções de compilação.

## Compilação e Execução

Para compilar o projeto, utilize o CMake. Execute os seguintes comandos no terminal:

```bash
mkdir build
cd build
cmake ..
make
```

Após a compilação, o executável será gerado e poderá ser executado para realizar a reconstrução de imagens.

## Dados Utilizados

Os dados utilizados para o teste do modelo 1 estão localizados na pasta `data`. Certifique-se de que os arquivos CSV estão presentes antes de executar o algoritmo.

## Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues ou pull requests para melhorias e correções.