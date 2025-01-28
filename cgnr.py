import numpy as np
import time
import tracemalloc
from scipy.sparse.linalg import cg

def cgnr(H, g, tol=1e-10, max_iter=1000):
    Ht = H.T
    HtH = np.dot(Ht, H)
    Htg = np.dot(Ht, g)
    x = np.zeros(H.shape[1])
    r = Htg - np.dot(HtH, x)
    p = r.copy()
    rsold = np.dot(r.T, r)

    for i in range(max_iter):
        Hp = np.dot(HtH, p)
        alpha = rsold / np.dot(p.T, Hp)
        x += alpha * p
        r -= alpha * Hp
        rsnew = np.dot(r.T, r)
        if np.sqrt(rsnew) < tol:
            break
        p = r + (rsnew / rsold) * p
        rsold = rsnew

    return x

# Carregando matrizes e vetor dos arquivos
M = np.loadtxt('dados/M.csv', delimiter=';')
a = np.loadtxt('dados/a.csv', delimiter=';')

# Verificar compatibilidade de dimensões
if M.shape[0] != a.shape[0]:
    raise ValueError("O número de linhas de M deve ser igual ao tamanho de a")

# Normalizar a matriz M para melhorar o condicionamento
M_norm = M / np.linalg.norm(M, axis=0)

# Medir tempo de execução e consumo de recursos
start_time = time.time()
tracemalloc.start()

x = cgnr(M_norm, a)

current, peak = tracemalloc.get_traced_memory()
end_time = time.time()

tracemalloc.stop()

print("Resultado CGNR:", x)
print(f"Tempo de execução: {end_time - start_time:.6f} segundos")
print(f"Memória atual: {current / 10**6:.6f} MB; Memória máxima: {peak / 10**6:.6f} MB")

# Comparação com a função cg do SciPy
start_time_scipy = time.time()
x_scipy, info = cg(M_norm, a, maxiter=1000)
end_time_scipy = time.time()

print("Resultado SciPy CG:", x_scipy)
print(f"Tempo de execução SciPy: {end_time_scipy - start_time_scipy:.6f} segundos")

# Verificação de resíduos
residual_cgnr = np.linalg.norm(np.dot(M_norm, x) - a)
residual_scipy = np.linalg.norm(np.dot(M_norm, x_scipy) - a)

print(f"Resíduo CGNR: {residual_cgnr:.6f}")
print(f"Resíduo SciPy CG: {residual_scipy:.6f}")