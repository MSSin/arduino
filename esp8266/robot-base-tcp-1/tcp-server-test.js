// Load the TCP Library
net = require('net');

// Keep track of the chat clients
var clients = [];

var dir=0;
// Start a TCP Server
net.createServer(function (socket) {

 
    // Identify this client
    socket.name = socket.remoteAddress + ":" + socket.remotePort

    // Put this new client in the list
    clients.push(socket);

    // Send a nice welcome message and announce
    socket.write("Welcome " + socket.name + "\n");
    broadcast(socket.name + " joined the chat\n", socket);

    // Handle incoming messages from clients.
    socket.on('data', function (data) {
        broadcast(socket.name + "> " + data, socket);
    });

    // Remove the client from the list when it leaves
    socket.on('end', function () {
        clients.splice(clients.indexOf(socket), 1);
        broadcast(socket.name + " left the chat.\n");
    });

    socket.on('close', function () {
        clients.splice(clients.indexOf(socket), 1);
        broadcast(socket.name + " closed.\n");
    });

    socket.on('error', function (error) {
        process.stdout.write('Error:' +error.toString());
        //clients.splice(clients.indexOf(socket), 1);
        //broadcast(socket.name + " closed.\n");
    });
    // Send a message to all clients
    function broadcast(message, sender) {
        clients.forEach(function (client) {
            // Don't want to send it to sender
            //if (client === sender) return;
            client.write(dir + "@", function(err){
                //if(err)
                //    process.stdout.write('Error', err)
            });
        });
        // Log it to the server output too
        process.stdout.write(message)
    }
	
}).listen(5000);

// Put a friendly message on the terminal of the server.
console.log("TCP server running at port 5000\n");

var keypress = require('keypress');

// make `process.stdin` begin emitting "keypress" events
keypress(process.stdin);

// listen for the "keypress" event
process.stdin.on('keypress', function (ch, key) {
  console.log('got "keypress"', key);
  if (key && key.ctrl && key.name == 'c') {
    process.stdin.pause();
  }
  else if (key && key.name == "up")
  {
	dir=1;	
  }
  else if (key && key.name == "right")
  {
	dir=2;
  }
  else if (key && key.name == "down")
  {
	dir=3;
  }
  else if (key && key.name == "left")
  {
	dir=4;
  }
  else if (key && key.name == "space")
  {
	dir=0;
  }
  
});

process.stdin.setRawMode(true);
process.stdin.resume();
