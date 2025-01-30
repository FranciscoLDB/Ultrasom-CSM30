import random
import time
import json
import requests
import numpy as np

def generate_signal():
    return np.random.rand(30)

def send_signal(signal):
    data = {
        "user": random.randint(1, 100),
        "gain": random.uniform(1.0, 2.0),
        "model": random.choice(["model1"]),
        "signal": signal.tolist()
    }
    response = requests.post("http://localhost:5000/reconstruct", json=data)
    return response.json()

while True:
    signal = generate_signal()
    result = send_signal(signal)
    # print("Reconstructed Image:", result)
    
    choice = input("Do you want to send another signal? (yes/no): ").strip().lower()
    if choice != 'yes':
        break
    
    time.sleep(random.uniform(1, 5))