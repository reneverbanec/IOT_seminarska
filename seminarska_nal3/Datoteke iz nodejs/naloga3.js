const express = require('express');
const WebSocket = require('ws');
const http = require('http');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

app.get('/', function(req, res) { // če je naslov "/" serviramo:
  res.sendFile(path.join(__dirname + '/naloga3.html')); // serviramo spodnjo HTML stran
});
app.use(express.static('public')); // serviramo statične datoteke
// Ko se vzpostavi nov Websocket
wss.on('connection', (ws) => {
  console.log('Client connected');

  // Ko dobimo sporočilo iz klienta
  ws.on('message', (message) => { 
    console.log(`Received: ${message}`);
    const data = JSON.parse(message); // razčlenimo sporočilo
    if (data.type === "mappedValue" || data.type === "LEDDisconnected" || data.type === "status") { // če je v sporočilu vrednost pošljemo naprej
      wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(message, { binary: false }); // Binary false zato ker smo imeli težave in je vračalo sporočilo tipa Blob.
        }
      });
    } else if (data.type === "toggleLED") { //Če je tip sporočila toggleLED - za Prižig/ugašanje LED
      wss.clients.forEach((client) => {
        client.send(JSON.stringify({ type: "toggleLED" }));
      });
    }
  });

  // Ob zaprtju povezave
  ws.on('close', () => {
    console.log('Client disconnected');
  });
});

// Zaženemo HTTP strežnik na portu 3000
const port = process.env.PORT || 3000;
server.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
