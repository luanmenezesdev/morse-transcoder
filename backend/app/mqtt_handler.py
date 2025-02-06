# mqtt_handler.py
import paho.mqtt.client as mqtt
from app import socketio

MQTT_BROKER = "broker.hivemq.com"
MQTT_PORT = 1883
MQTT_TOPIC = "morse-transcoder/chat"

mqtt_client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code", rc)
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    decoded_message = msg.payload.decode()
    print(f"Received MQTT message: {decoded_message}")

    # Parse the sender and message
    try:
        sender, message = decoded_message.split(": ", 1)  # Split into sender and message
        sender = sender.strip()
        message = message.strip()

        # Only emit to frontend if the sender is not "frontend"
        if sender != "frontend":
            socketio.emit("chat_message", {"message": decoded_message})
            print(f"Message propagated to frontend via WebSocket: {decoded_message}")
        else:
            print(f"Message from frontend ignored: {decoded_message}")
    except ValueError:
        print("Invalid message format. Expected 'sender: message'")


def init_mqtt():
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_start()
