import numpy as np
import psutil

def test_matrix_size():
    available_memory = psutil.virtual_memory().available
    print(f"Memória disponível: {available_memory / (1024 ** 3):.2f} GB")

    size = 30000  # Tamanho inicial da matriz
    size2 = 3600
    while True:
        try:
            print(f"Tentando carregar matriz de tamanho {size}x{size2}...")
            matrix = np.ones((size, size2), dtype=np.float64)  # Usando np.float64 para maior precisão
            print(f"Matriz de tamanho {size}x{size2} carregada com sucesso.")
            size = int(size * 1.1)  # Dobrar o tamanho da matriz e pegar apenas o inteiro
        except MemoryError:
            print(f"Erro de memória ao carregar matriz de tamanho {size}x{size2}.")
            break

def print_min_values():
    print(f"Menor valor para np.float64: {np.finfo(np.float64).min}")
    print(f"Menor valor para np.float32: {np.finfo(np.float32).min}")
    print(f"Menor valor para np.int64: {np.iinfo(np.int64).min}")
    print(f"Menor valor para np.int32: {np.iinfo(np.int32).min}")

if __name__ == "__main__":
    print_min_values()
    test_matrix_size()