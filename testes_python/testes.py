import numpy as np

# Carregando matrizes e vetor dos arquivos
M = np.loadtxt('dados/M.csv', delimiter=';')
N = np.loadtxt('dados/N.csv', delimiter=';')
a = np.loadtxt('dados/a.csv', delimiter=';')
MN_expected = np.loadtxt('dados/MN.csv', delimiter=';')  # Carregar MN esperado
aM_expected = np.loadtxt('dados/aM.csv', delimiter=';')  # Carregar aM esperado
print("M:\n", M)
print("N:\n", N)
print("a:\n", a)

# Operações matriciais
MN = np.dot(M, N)
aM = np.dot(a, M)  # Corrigir para multiplicação de vetor por matriz
aM = np.round(aM, 2)  # Arredondar aM para 2 casas decimais

print("MN:\n", MN)
print("aM:\n", aM)

# Comparação com MN esperado
if np.allclose(MN, MN_expected):
    print("MN é igual ao esperado.")
else:
    print("MN não é igual ao esperado.")

# Comparação com aM esperado
if np.allclose(aM, aM_expected):
    print("aM é igual ao esperado.")
else:
    print("aM não é igual ao esperado.")