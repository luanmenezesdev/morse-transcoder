def handle_special_event(message):
    if "SOS" in message.upper():
        print("Emergency SOS detected!")
        # Perform any special actions here (e.g., notifications)
        return True
    return False
