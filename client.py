import random
import time
import json
import requests

def generate_signal():
    return np.random.rand(100)

def send_signal(signal):
    data = {
        "user": random.randint(1, 100),
        "gain": random.uniform(1.0, 2.0),
        "model": random.choice(["model1", "model2"]),
        "signal": signal.tolist()
    }
    response = requests.post("http://localhost:5000/reconstruct", json=data)
    return response.json()

while True:
    signal = generate_signal()
    result = send_signal(signal)
    print("Reconstructed Image:", result)
    time.sleep(random.uniform(1, 5))