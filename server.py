from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/reconstruct', methods=['POST'])
def reconstruct():
    data = request.json
    signal = np.array(data['signal'])
    # Carregar modelo e executar algoritmo de reconstrução
    # ...
    result = {
        "user": data['user'],
        "algorithm": "CGNR",
        "start_time": time.time(),
        "end_time": time.time() + 1,  # Exemplo
        "size": signal.size,
        "iterations": 100  # Exemplo
    }
    return jsonify(result)

if __name__ == '__main__':
    app.run(debug=True)