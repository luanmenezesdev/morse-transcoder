import boto3
import json

AWS_REGION = "us-east-1"
LAMBDA_FUNCTION_NAME = "TriggerAlexaAlarmLuan"

lambda_client = boto3.client("lambda", region_name=AWS_REGION)

def handle_special_event(message):
    if "SOS" in message.upper():
        print("Emergency SOS detected!")
        # Create the payload to send to Lambda
        payload = {
            "request": {
                "type": "CustomEvent",
                "message": "Emergency SOS Triggered",
                "details": message  # Include the full SOS message
            },
            "context": {
                "System": {
                    "user": {
                        "userId": "amzn1.ask.account.ABCDEFGHIJKLMNOPQRST"  # Replace with valid user ID if needed
                    }
                }
            }
        }

        try:
            # Invoke the Lambda function
            response = lambda_client.invoke(
                FunctionName=LAMBDA_FUNCTION_NAME,
                InvocationType="RequestResponse",  # Synchronous invocation to get the response
                Payload=json.dumps(payload)
            )

            # Read the response from Lambda
            response_payload = json.loads(response["Payload"].read())
            print("Lambda Response:", response_payload)
            return True

        except Exception as e:
            print("Error invoking Lambda:", str(e))
            return False

    return False
