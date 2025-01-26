// Establish WebSocket connection to the backend
const socket = io("http://localhost:8000", {
  transports: ["websocket"],
});

// DOM Elements
const inputField = document.getElementById("message-input");
const sendButton = document.getElementById("send-button");
const messageList = document.getElementById("messages");

// Event listener for send button
sendButton.addEventListener("click", () => {
  const message = inputField.value.trim();
  if (message) {
    const formattedMessage = `frontend: ${message}`;
    socket.emit("send_message", { message: formattedMessage }); // Formato simplificado
    addMessageToList(`You: ${message}`, "sent");
    inputField.value = "";
  }
});

// Confirm connection to the backend
socket.on("connect", () => {
  console.log("Socket.IO connected successfully.");
});

// Handle incoming messages from the backend
socket.on("chat_message", (data) => {
  const [_sender, message] = data.message.split(": ");
  addMessageToList(`Transcoder: ${message}`, "received");
});

// Utility function to add a message to the chat list
function addMessageToList(text, type) {
  const messageItem = document.createElement("li");
  messageItem.textContent = text;
  messageItem.classList.add("message");
  messageItem.classList.add(type === "sent" ? "sent" : "received");
  messageList.appendChild(messageItem);
  messageList.scrollTop = messageList.scrollHeight; // Auto-scroll to the latest message
}
