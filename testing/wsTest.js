const exampleSocket = new WebSocket("ws://127.0.0.1:8081/websocket");
const jsonObject = JSON.stringify({
  action: "CREATE_USER",
  data: "fauzan tahir",
});

exampleSocket.addEventListener("open", (event) => {
  console.log(
    "websocket connection successfully established now sending message",
  );

  exampleSocket.send(jsonObject);
});
exampleSocket.onmessage = (event) => {
  console.log(event.data);
};
