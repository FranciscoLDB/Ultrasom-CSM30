#### Nosso contexto 
• Um processo distribuído entre um cliente especializado e um servidor; 
• Alto custo computacional; 
• Necessita de gerenciamento de recursos avançado;

Os sistemas de imagem por ultrassom são uteis em diversas áreas da atividade humana. 
Os exemplos mais comuns vão desde a área da saúde, com exames de imagem para diagnóstico médico, até na produção industrial, em inspeção de qualidade de produtos e na busca por falhas e defeitos.
Os métodos de reconstrução de imagens de ultrassom baseados em problemas inversos são inovações recentes para ultrassom e têm demonstrado obter qualidade de imagem superior as reconstruções convencionais baseadas em beamforming e delay-and-sum. 
Contudo, o custo computacional desses algoritmos é alto, não permitindo execução em tempo real, que é uma demanda já estabelecida em ultrassom.

#### Objetivos: 
• Reconstruir uma image a partir do sinal capturado pelo cliente; 
• Gerenciar os métodos de reconstrução conforme escolha do usuário;
• Gerenciar os recursos disponíveis no servidor; 
• Alcançar o máximo possível de eficiência.


#### 1.) Seleção de linguagem e bibliotecas BLAS.

Nesta etapa deverão ser selecionadas linguagens e bibliotecas candidatas para o desenvolvimento. Deverá ser verificado se as escolhas suportam o desenvolvimento dos requisitos funcionais e não funcionais necessários.

#### 2.) Teste das operações básicas.

Utilizar as bibliotecas selecionadas e realizar as operações básicas a seguir:  

_MN = M * N_

_aM = a * M_

_Ma = M * a_

Estas operações são todas matriciais, envolvendo matrizes e vetores e devem seguir as regras da Álgebra Linear.

Dados para teste: Dados.zip

#### 3.) Implementação do algoritmo CGNR.

Codificar um protótipo com o Algoritmo CGNR. Validar os resultados com os dados experimentais. 

Medir o tempo total de execução e o consumo de recursos como memória e ocupação de CPU.

#### 4.) Testes de saturação e implementação de rotinas de controle.  

Realizar testes de saturação do sistema.  

Medir os recursos e projetar um algoritmo para controlar e evitar a saturação.

### Algoritmos e definições.

#### Siglas

**g** - Vetor de sinal

**H** - Matriz de modelo

**f** - Imagem

S - Número de amostras do sinal

N - Número de elementos sensores

#### Cálculo do fator de redução (C)

\(  \large c = || \mathbf{H}^T * \mathbf{H}||_2 \)

#### Cálculo do coeficiente de regularização ( \( \lambda \))  

\( \large \lambda = max( abs(\mathbf{H}^T * \mathbf{g})) * 0.10 \)  

  

#### Cálculo do erro (\( \large \epsilon \))

\(  \large \epsilon = ||\mathbf{r}_{i+1}||_2 - ||\mathbf{r}_i||_2 \)

  

#### Cálculo do ganho de sinal (\( \large \gamma \))

\(for \ c = 1 \ .. \ N \)  

\(for  \ l = 1 \ .. \ S \)  

\( \gamma_l = 100 + 1/20 * l * \sqrt[]{l} \)

\(g_{l,c} = g_{l,c} * \gamma_l \)


##### Requisitos não funcionais e restrições.

Cada imagem deverá conter no mínimo os seguintes dados:

- - Identificação do usuário;
    - Identificação do algoritmo utilizado
    - Data e hora do início da reconstrução;
    - Data e hora do término da reconstrução;
    - Tamanho em pixels;
    - O número de iterações executadas.

##### Requisitos funcionais e restrições.
##### Cliente.

Implementar uma aplicação cliente com as seguintes características:  

1. Enviar uma sequência de sinais em intervalos de tempo aleatórios;
2. O usuário, o ganho de sinal e o modelo da imagem deverão ser definidos aleatoriamente;
3. Gerar um relatório com todas as imagens reconstruídas com as seguintes informações: imagem gerada, usuário, número de iterações e tempo de reconstrução;
4. Gerar um relatório de desempenho do servidor, com as informações de consumo de memória e de ocupação de CPU num determinado intervalo de tempo;

##### Servidor.  

Implementar um servidor para reconstrução de imagens:

1. Receber os dados para reconstrução;      
2. Carregar o modelo de reconstrução de acordo com os parâmetros recebidos;
3. Executar o algoritmo de reconstrução;
4. Executar até que o erro (\( \large \epsilon \)) seja menor do que 1e10-4 .  
5. Salvar o resultado.


### Algoritmo 1: CGNR (Conjugate Gradient Normal Residual) (Saad2003, p. 266)

\( \bf{f}_0 = 0 \)

\( \bf{r}_0 = \bf{g} - \bf{Hf}_0 \)

\( \bf{z}_0 = \bf{H}^T\bf{r}_0 \)

\( \bf{p}_0 = \bf{z}_0 \)

\( for \space i = 0,1,..., until \space convergence \)

\( \bf{w}_i = \bf{H}\bf{p}_i \)

\( \alpha_i = || \bf{z}_i ||^2_2 / || \bf{w}_i ||^2_2 \)

\( \bf{f}_{i+1} = \bf{f}_i + \alpha_i \bf{p}_i \)

\( \bf{r}_{i+1} = \bf{r}_i - \alpha_i \bf{w}_i \)

\( \bf{z}_{i+1} = \bf{H}^T \bf{r}_{i+1} \)

\( \beta_i = || \bf{z}_{i+1} ||^2_2 / || \bf{z}_{i} ||^2_2 \)

\( \bf{p}_{i+1} = \bf{z}_{i+1} + \beta_i \bf{p}_i \)

### Algoritmo 1: CGNE (Conjugate Gradient Method Normal Error)

\( \bf{f}_0 = 0 \)

\( \bf{r}_0 = \bf{g} - \bf{Hf}_0 \)

\( \bf{p}_0 = \bf{H}^T\bf{r}_0 \)

\( for \space i = 0,1,..., until \space convergence \)

\( \alpha_i = \frac{\bf{r}_i^T \bf{r}_i}{\bf{p}_i^T \bf{p}_i} \)

\( \bf{f}_{i+1} = \bf{f}_i + \alpha_i \bf{p}_i \)

\( \bf{r}_{i+1} = \bf{r}_i - \alpha_i \bf{Hp}_i \)

\( \beta_i = \frac{\bf{r}_{i+1}^T \bf{r}_{i+1}}{\bf{r}_i^T \bf{r}_i} \)

\( \bf{p}_{i+1} = \bf{H}^T \bf{r}_{i+1} + \beta_i \bf{p}_i \)

### Arquivos para teste

#### Dados para modelo 1
/Dados de testes para o modelo 1/H-1.csv - matriz H
Matriz H 
Modelo 50816 x 3600, 
60 x 60 pixels, 
S = 794, 
N = 64

/Dados de testes para o modelo 1/G-1.csv - Vetor de sinal 1
Vetor g
Modelo 50816 x 1

/Dados de testes para o modelo 1/G-2.csv - Vetor de sinal 2
Vetor g
Modelo 50816 x 1

#### Dados para modelo 2
/Dados de testes para o modelo 2/H-2.csv - matriz H
Matriz H 
Modelo: 27904 x 900, 
30 x 30 pixels, 
S = 436, 
N = 64

/Dados de testes para o modelo 2/g-30x30-1.csv - Vetor de sinal 1
Vetor g
Modelo 50816 x 1

/Dados de testes para o modelo 2/g-30x30-2.csv - Vetor de sinal 2
Vetor g
Modelo 50816 x 1

# Instruções para o GitHub Copilot

- Oferece as sugestões com base no contexto do problema e com base nos requisitos acima.
- dados importantes estão acima, conforme dados para teste e siglas para entendimento.