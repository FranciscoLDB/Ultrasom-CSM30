import numpy as np
import psutil
import time
import threading

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

def monitor_resources(interval=1, stop_event=None):
    while not stop_event.is_set():
        cpu_usage = psutil.cpu_percent(interval=interval)
        memory_usage = psutil.virtual_memory().percent
        print(f"CPU Usage: {cpu_usage}% | Memory Usage: {memory_usage}%")
        time.sleep(interval)

# Evento para parar o monitoramento
stop_event = threading.Event()

# Executar monitoramento em paralelo com os testes de saturação
monitor_thread = threading.Thread(target=monitor_resources, args=(1, stop_event))
monitor_thread.start()

# Carregando matrizes e vetor dos arquivos
H = np.loadtxt('dados/M.csv', delimiter=';')
g = np.loadtxt('dados/a.csv', delimiter=';')

# Configurações de progresso e quantidade máxima de iterações
max_iterations = 100000
progress_interval = 20  # Porcentagem de progresso para printar

# Executar múltiplas instâncias do algoritmo CGNR
try:
    for i in range(max_iterations):
        x = cgnr(H, g)
        # print("Resultado:", x)
        # print a cada progress_interval% feito
        if ((i % (max_iterations // (100 // progress_interval)) == 0) or (i == max_iterations - 1)):
            print(f"Progresso: {i / (max_iterations // 100):.0f}%")
finally:
    # Parar o monitoramento após a execução do loop
    stop_event.set()
    monitor_thread.join()