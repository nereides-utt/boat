

#test commande :
# mosquitto_pub -h localhost -t battery_temperature -m "{\"temperature\": 25.3}"

import paho.mqtt.client as mqtt
import requests
from dotenv import load_dotenv
import os

# Charger les variables d'environnement depuis un fichier .env
load_dotenv()

#-------------------------------------------------------------------------------
TOPICS = ['battery_temperature', 'voltage']
# Variables d'environnement
BROKER_URL = os.getenv('BROKER_URL', 'localhost')
BROKER_PORT = int(os.getenv('BROKER_PORT', 1883))

API_URL = os.getenv('API_URL', 'http://127.0.0.1:5000/test')
API_TOKEN = os.getenv('API_TOKEN', 'default_token')

# Fonction callback pour la réception des messages
def on_message(client, userdata, message):
    print(f"Message reçu sur {message.topic}: {message.payload.decode('utf-8')}")
    try:
        payload = message.payload.decode('utf-8')
        topic = message.topic
        send_to_api(topic, payload)
    except Exception as e:
        print(f"Erreur de traitement des données: {e}")

# Envoi des données à l'API
def send_to_api(topic, payload):
    try:
        # Convertir la charge utile en entier
        value = float(payload)
        data = {
            'topic': topic,
            'payload': {'value': value}
        }
        headers = {'Authorization': f'Bearer {API_TOKEN}'}
        response = requests.post(API_URL, json=data, headers=headers)
        if response.status_code == 200:
            print(f"API: Données envoyées pour {topic}")
        else:
            print(f"API: Échec de l'envoi pour {topic}, Status: {response.status_code}, Message: {response.text}")
    except ValueError:
        print(f"Erreur: la charge utile '{payload}' n'est pas un entier valide")
    except requests.RequestException as e:
        print(f"API: Erreur de connexion: {e}")

# Fonction callback pour la connexion au broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connecté au broker MQTT")
        # Abonnement aux topics
        for topic in TOPICS:
            client.subscribe(topic)
            print(f"Abonné au topic: {topic}")
    else:
        print(f"Échec de connexion, code de retour: {rc}")

# Fonction callback pour la déconnexion du broker
def on_disconnect(client, userdata, rc):
    print(f"Déconnecté du broker MQTT avec le code {rc}")

# Configuration du client MQTT
client = mqtt.Client(protocol=mqtt.MQTTv311)
client.on_message = on_message
client.on_connect = on_connect
client.on_disconnect = on_disconnect

# Connexion au broker MQTT
try:
    client.connect(BROKER_URL, BROKER_PORT, 60)
    client.loop_forever()
except ValueError as e:
    print(f"Erreur de connexion: {e}")
except Exception as e:
    print(f"Erreur inattendue: {e}")
