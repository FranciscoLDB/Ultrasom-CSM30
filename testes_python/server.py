from flask import Flask, request, jsonify
import numpy as np
import time
import matplotlib.pyplot as plt
import io
import base64

app = Flask(__name__)

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

def model1():    
    H = np.loadtxt('dados/M.csv', delimiter=';') # Matriz de modelo
    g = np.loadtxt('dados/a.csv', delimiter=';') # Vetor de ganho
    S = 794 # Número de amostras do sinal
    N = 64 # Número de elementos sensores
    f = any # imagem

@app.route('/reconstruct', methods=['POST'])
def reconstruct():
    data = request.json
    if not all(k in data for k in ('signal', 'user', 'gain', 'model')):
        return jsonify({"error": "Missing required parameters"}), 400

    signal = np.array(data['signal'])
    user = data['user']
    gain = data['gain']
    model = data['model']

    tam = 0
    if (model == "model1"):
        tam = 30
    elif (model == "model2"):
        tam = 60
    else:
        return jsonify({"error": "Invalid model"})

    # Verifica tamanho de g
    if len(signal) != tam:
        return jsonify({"error": "Signal size does not match the model"})

    H = np.random.rand(tam, tam)
    g = signal * gain
    
    start_time = time.time()
    reconstructed_image = cgnr(H, g)
    end_time = time.time()
    
    # Verificar se o tamanho da imagem é um quadrado perfeito
    # side_length = int(np.sqrt(reconstructed_image.size))
    # if side_length * side_length != reconstructed_image.size:
    #     return jsonify({"error": "Reconstructed image size is not a perfect square"})

    # # Plotar a imagem reconstruída em tons de preto e branco
    # plt.imshow(reconstructed_image.reshape((side_length, side_length)), cmap='gray')
    # plt.axis('off')
    # buf = io.BytesIO()
    # plt.savefig(buf, format='png')
    # buf.seek(0)
    # image_base64 = base64.b64encode(buf.read()).decode('utf-8')
    # plt.close()

    result = {
        "user": user,
        "algorithm": "CGNR",
        "start_time": start_time,
        "end_time": end_time,
        "size": reconstructed_image.size,
        "iterations": 100,  # Exemplo
        "reconstructed_image": reconstructed_image.tolist(),
        #"image_base64": image_base64
    }
    return jsonify(result)

if __name__ == '__main__':
    app.run(debug=True)