const http = require("http"); // knjižnica za delo s HTTP

const fs = require("fs"); // knjižnice za delo z datotekami, ki jih rabimo pri posredovanju HTML strani

const hostname = "192.168.1.13"; // IP naslov VirtualBox Node.js strežnika
const port = 80; // port preko katerega pošiljamo HTTP zahtevek

const server = http.createServer((req, res) => { // inicializacija strežnika
    if (req.url === "/") {   // če bo zahtevan url z "/" potem: 
        fs.readFile(__dirname + "/naloga1.html", (err, data) => { // preberemo podatek o spletni strani 
            if (err) { // če pri branju datoteke nal1_htmlstran.html pride do napake
                res.writeHead(500, {"Content-Type": "text/plain; charset=utf-8"}); // vrnemo response s kodo 500
                return res.end("Napaka pri nalaganju HTML strani. Branje datoteke naloga1.html z diska ni bilo mogoče."); // klientu kot odgovor ("response") posredujemo obvestilo o napaki
            }
    
            res.writeHead(200); // če ni napake vrnemo nazaj kodo 200
            res.end(data); // vrnemo nazaj spletno stran
        });
    }
});

server.listen(port, hostname, () => {
    console.log("Strežnik teče na http://" + hostname + ":" + port);
});
