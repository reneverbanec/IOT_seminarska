const express = require('express'); 
const WebSocket = require('ws'); 
const http = require('http');
const path = require('path'); 
// Zgoraj so vse potrebne knjižnice

const app = express();  // uporabimo express 
const server = http.createServer(app); 
const wss = new WebSocket.Server({ server }); 



app.get('/', function(req, res) { // če je naslov "/" serviramo:
  res.sendFile(path.join(__dirname + '/naloga2b.html')); // serviramo spodnjo HTML stran
});
app.use(express.static('public')); // serviramo statične datoteke

// Ko se vzpostavi nov Websocket
wss.on('connection', (ws) => {
  console.log('Client connected');
 
   // Ko dobimo sporočilo iz klienta
  ws.on('message', (message) => {
    console.log(`Received: ${message}`);
    wss.clients.forEach((client) => { // Pošljemo sporočilo vsem klientom
      if (client.readyState === WebSocket.OPEN) {
        client.send(message, { binary: false }); // Binary false zato ker smo imeli težave in je vračalo sporočilo tipa Blob.
      }
    });
  });

  // Ob zaprtju povezaved
  ws.on('close', () => {
    console.log('Client disconnected');
  });
});

// Zaženemo HTTP strežnik na portu 3000
const port = process.env.PORT || 3000;
server.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
