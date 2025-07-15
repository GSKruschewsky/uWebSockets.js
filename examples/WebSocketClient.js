/* A simple WebSocket client example */

const uWS = require('../dist/uws.js');
const port = 9001;

const app = uWS.CliSSLApp();

app
  .ws({
    /* Options */
    /* You can insert any options here... */

    /* Handlers */
    open: (ws) => {
      console.log('[!] WebSocket client connection is open.\n ');
    },
    message: (ws, message, isBinary) => {
      /* In case of a JSON message: 
       * console.log('[!] WebSocket client received a message:', JSON.parse(Buffer.from(message)), '\n ');
       */
      console.log('[!] WebSocket client received a message:', Buffer.from(message).toString('utf8'), '\n ');
    },
    drain: (ws) => {
      console.log('WebSocket backpressure: ' + ws.getBufferedAmount());
    },
    close: (ws, code, message) => {
      console.log('[!] WebSocket client connection is closed.\n ');
    }
  })
  .connect("wss://localhost");