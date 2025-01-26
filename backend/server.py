# Monkey-patch for compatibility
import eventlet
eventlet.monkey_patch()

from app import app, socketio, routes
from app.mqtt_handler import init_mqtt, mqtt_client, MQTT_TOPIC

if __name__ == "__main__":
    init_mqtt()  # Initialize MQTT client
    print("MQTT initialized successfully.")

    # Test MQTT publish
    mqtt_client.publish(MQTT_TOPIC, "Test message from server")
    print("Test message published to MQTT.")

    # Run the server
    socketio.run(app, host="0.0.0.0", port=8000, debug=True, log_output=True)
