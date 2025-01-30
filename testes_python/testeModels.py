import numpy as np
import psutil
import time
import threading
import matplotlib.pyplot as plt
import io
import base64

def calculate_reduction_factor(H):
    """
    Calculate the reduction factor (c) as the 2-norm of the product of the transpose of H and H.
    
    Parameters:
    H (numpy.ndarray): Input matrix H.
    
    Returns:
    float: The reduction factor c.
    """
    # Calculate the product of the transpose of H and H
    H_transpose_H = np.dot(H.T, H)
    
    # Calculate the 2-norm (spectral norm) of the resulting matrix
    c = np.linalg.norm(H_transpose_H, 2)
    
    return c

def calculate_regularization_coefficient(H, g):
    """
    Calculate the regularization coefficient (lambda).
    
    Parameters:
    H (numpy.ndarray): Input matrix H.
    g (numpy.ndarray): Input vector g.
    
    Returns:
    float: The regularization coefficient lambda.
    """
    # Calculate the product of the transpose of H and g
    H_transpose_g = np.dot(H.T, g)
    
    # Calculate the maximum absolute value and multiply by 0.10
    lambda_value = np.max(np.abs(H_transpose_g)) * 0.10
    
    return lambda_value

def calculate_error(r_i, r_i_plus_1):
    """
    Calculate the error (epsilon).
    
    Parameters:
    r_i (numpy.ndarray): Residual vector at iteration i.
    r_i_plus_1 (numpy.ndarray): Residual vector at iteration i+1.
    
    Returns:
    float: The error epsilon.
    """
    # Calculate the 2-norm of the residual vectors
    norm_r_i = np.linalg.norm(r_i, 2)
    norm_r_i_plus_1 = np.linalg.norm(r_i_plus_1, 2)
    
    # Calculate the error
    epsilon = norm_r_i_plus_1 - norm_r_i
    
    return epsilon

def calculate_signal_gain(g, N, S):
    """
    Calculate the signal gain (gamma) and update g.
    
    Parameters:
    g (numpy.ndarray): Input matrix g.
    N (int): Number of columns in g.
    S (int): Number of rows in g.
    
    Returns:
    numpy.ndarray: Updated matrix g with signal gain applied.
    """
    for c in range(N):
        for l in range(1, S + 1):
            gamma_l = 100 + 1/20 * l * np.sqrt(l)
            g[l-1, c] *= gamma_l
    
    return g

def cgnr(H, g, tol=1e-10, max_iter=1000):
    Ht = H.T
    HtH = np.dot(Ht, H)
    Htg = np.dot(Ht, g)
    x = np.zeros(H.shape[1])
    r = Htg - np.dot(HtH, x)
    p = r.copy()
    rsold = np.dot(r.T, r)

    # Calculate reduction factor
    c = calculate_reduction_factor(H)
    print("Reduction factor (c):", c)

    # Calculate regularization coefficient
    lambda_value = calculate_regularization_coefficient(H, g)
    print("Regularization coefficient (lambda):", lambda_value)

    for i in range(max_iter):
        Hp = np.dot(HtH, p)
        alpha = rsold / np.dot(p.T, Hp)
        x += alpha * p
        r_new = r - alpha * Hp

        # Calculate error
        epsilon = calculate_error(r, r_new)
        print(f"Iteration {i}, Error (epsilon):", epsilon)

        r = r_new
        rsnew = np.dot(r.T, r)
        if np.sqrt(rsnew) < tol:
            break
        p = r + (rsnew / rsold) * p
        rsold = rsnew

    return x

def load_data(filepath, delimiter=','):
    try:
        data = []
        with open(filepath, 'r') as file:
            total_lines = sum(1 for line in file)
            file.seek(0)
            for i, line in enumerate(file):
                print(f"Reading line {i + 1} of {total_lines}", end='\r')
                print(f"Row: line, tamanho: {len(line)}")
                row = [float(x.replace('e', 'E')) for x in line.strip().split(delimiter)]
                print(f"Row: {row}")
                data.append(row)
                if (i + 1) % (total_lines // 20) == 0:
                    print(f"Loading data: {((i + 1) / total_lines) * 100:.2f}% complete")
        data = np.array(data)
        print(f"Loaded data from {filepath}")
        return data
    except Exception as e:
        print(f"Erro ao carregar dados de {filepath}: {e}")
        return None

def model1():    
    try:
        print("Loading H")
        H = load_data('Dados de testes para o modelo 1/H-1.csv') # Matriz de modelo 50816 x 3600
        print("Loaded H")
        print("Loading g")
        g = load_data('Dados de testes para o modelo 1/G-1.csv') # Vetor de sinal 50816 x 1
        print("Loaded g")
        if H is None or g is None:
            return

        S = 794 # Número de amostras do sinal
        N = 64 # Número de elementos sensores

        # Executar CGNR
        print("Iniciando CGNR...")
        reconstructed_image = cgnr(H, g)
        print("CGNR concluído.")

        # Gerar imagem 60x60
        side_length = 60
        if side_length * side_length != reconstructed_image.size:
            print("Erro: O tamanho da imagem reconstruída não é um quadrado perfeito")
            return

        f = reconstructed_image.reshape((side_length, side_length))
        plt.imshow(f, cmap='gray')
        plt.axis('off')
        plt.savefig('reconstructed_image_model1.png')
        plt.close()
        return f
    except Exception as e:
        print(f"Erro ao executar o modelo 1: {e}")

def monitor_resources(interval=1, stop_event=None):
    while not stop_event.is_set():
        cpu_usage = psutil.cpu_percent(interval=interval)
        memory_usage = psutil.virtual_memory().percent
        print(f"CPU Usage: {cpu_usage}% | Memory Usage: {memory_usage}%")
        time.sleep(interval)

# Evento para parar o monitoramento
stop_event = threading.Event()

# Executar monitoramento em paralelo com os testes de saturação
monitor_thread = threading.Thread(target=monitor_resources, args=(0.7, stop_event))
monitor_thread.start()

try:
    reconstructed_image = model1()
    if reconstructed_image is not None:
        side_length = 60
        f = reconstructed_image.reshape((side_length, side_length)) # Imagem 60x60
        plt.imshow(f, cmap='gray')
        plt.axis('off')
        plt.savefig('reconstructed_image_model1.png')
        plt.close()
except Exception as e:
    print(f"Erro ao plotar a imagem do modelo 1: {e}")
finally:
    # Parar o monitoramento após a execução do loop
    stop_event.set()
    monitor_thread.join()

