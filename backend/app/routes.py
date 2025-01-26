from app import socketio
from app.mqtt_handler import mqtt_client, MQTT_TOPIC
from app.event_handler import handle_special_event

@socketio.on("send_message")
def handle_send_message(data):
    try:
        print("Event triggered: send_message")  # Log when the event is triggered
        print("Data received:", data)          # Log the incoming data

        message = data.get("message")
        if message:
            print("Handling message:", message)  # Log the message to handle
            handle_special_event(message)
            mqtt_client.publish(MQTT_TOPIC, message)
            print(f"Message sent to MQTT: {message}")
        else:
            print("No message in data.")  # Log if data is missing the 'message' field
    except Exception as e:
        print("Error in handle_send_message:", e)
