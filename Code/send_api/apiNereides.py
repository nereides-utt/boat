import time
import paho.mqtt.client as mqtt
import requests
from dotenv import load_dotenv
import os

PARAMS  = {
  "nereides_temp1": -1,
  "nereides_temp2": -1,
  "nereides_temp3": -1,
  "nereides_voltage": -1,
  "nereides_current": -1,
  "team": "d2mKdfuCjv2AB9NQYXMFyGe9DEDL8Pmb"
}

load_dotenv()

#-------------------------------------------------------------------------------
#mettre les bons topics + est ce que batterie températrue
#'nereides/pac/current_a','nereides/pac/voltage_v'
TOPICS = ['nereides/pac/temperature_c',
            'nereides/battery/temp_pack1',
            'nereides/batterySE/temp', 
            'nereides/battery/current_a',
            'nereides/battery/voltage_v'
            ]
# Variables d'environnement
BROKER_URL = os.getenv('BROKER_URL', 'test.mosquitto.org')
BROKER_PORT = int(os.getenv('BROKER_PORT', 1883))

#API_URL = 
API_TOKEN = "R5LQ8KsalkC6qPenc9s8JjEKIbvXDYoebDbMUDgBOq0-jjDI=ne==p2ZNMjKcwCqJGbg3eeI59dWq-DoBz3f6arAXSQ=="

# Fonction callback pour la réception des messages
def on_message(client, userdata, message):
    print(f"Message reçu sur {message.topic}: {message.payload.decode('utf-8')}")
    try:
        payload = message.payload.decode('utf-8')
        topic = message.topic
        value = float(payload)
        mettre_a_jour_params(topic,value)
    except Exception as e:
        print(f"Erreur de traitement des données: {e}")

def mettre_a_jour_params(topic, value):
    if topic == TOPICS[0]:
        PARAMS["nereides_temp1"] = value
    elif topic == TOPICS[1]:
        PARAMS["nereides_temp2"] = value
    elif topic == TOPICS[2]:
        PARAMS["nereides_temp3"] = value
    elif topic == TOPICS[3]:
        PARAMS["nereides_current"] = value
    elif topic == TOPICS[4]:
        PARAMS["nereides_voltage"] = value
    print(PARAMS)


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

def envoyer_donnees_api():
    try:
        response = requests.post(url = "http://changerlipenfonctiondeleurAPI:3001/monitoringdata/", json= PARAMS)
        if response.status_code == 200:
            print("API : données envoyées périodiquement")
        else: 
            print(f"API echec") 
    except requests.RequestException as e: 
        print(f"API: erreur connexion")

# Configuration du client MQTT
client = mqtt.Client(protocol=mqtt.MQTTv311)
client.on_message = on_message
client.on_connect = on_connect
client.on_disconnect = on_disconnect

# Connexion au broker MQTT
try:
    client.connect(BROKER_URL, BROKER_PORT, 60)
    last_sent_time = time.time()
    interval = 0.75
    while True:
        client.loop(timeout=1.0)
        current_time = time.time()
        if current_time - last_sent_time >= interval:
            envoyer_donnees_api()
            last_send_time = current_time
except ValueError as e:
    print(f"Erreur de connexion: {e}")
except Exception as e:
    print(f"Erreur inattendue: {e}")
