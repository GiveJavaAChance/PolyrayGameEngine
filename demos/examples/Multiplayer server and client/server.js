const net = require('net');

const PORT = 3050;

let clients = {};

function getNextClientID() {
    const ids = Object.keys(clients).map(id => parseInt(id));
    if (ids.length === 0) {
        return 0;
    }
    return Math.max(...ids) + 1;
}

const server = net.createServer((socket) => {
    console.log('Client connected');
    const clientID = getNextClientID();

    const clientIDBuffer = Buffer.alloc(4);
    clientIDBuffer.writeInt32BE(clientID, 0);
    socket.write(clientIDBuffer);

    clients[clientID] = socket;

    socket.on('data', (data) => {
        try {
            const header = data.slice(0, 8);
            const clientIDReceived = header.readInt32BE(0);
            const packetID = header.readInt32BE(4);
            broadcastPacket(clientIDReceived, data);
        } catch (err) {
        }
    });

    socket.on('end', () => {
        cleanupClient(clientID, socket);
    });

    socket.on('error', (err) => {
        cleanupClient(clientID, socket);
    });

    socket.on('close', () => {
        cleanupClient(clientID, socket);
    });
});

function cleanupClient(clientID, socket) {
    delete clients[clientID];

    if (socket && socket.writable) {
        socket.end();
    }
}

function broadcastPacket(senderID, packetData) {
    Object.keys(clients).forEach((clientID) => {
        if (parseInt(clientID) !== senderID) {
            const clientSocket = clients[clientID];
            try {
                clientSocket.write(packetData);
            } catch (err) {
            }
        }
    });
}

server.listen(PORT, () => {
    console.log(`Server is listening on port ${PORT}`);
});