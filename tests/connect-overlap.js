// Run inside the tests folder against the freshly built binaries in ../dist.
// Two connect() calls on one client app before either attempt has opened: each socket
// sends and verifies its own WebSocket handshake (the flag and key are per socket, not
// per app), so both attempts open and the server sees two distinct keys. Before this
// fix the second connect() re-armed the app-wide flag, the first socket to open sent the
// handshake and the second never did, idling until the HTTP timeout closed it.
const uWS = require('../dist/uws.js');

const port = 9016;
let failed = false;
const fail = (msg) => { console.error('FAIL:', msg); failed = true; };

const keys = [];
const server = uWS.App().ws('/*', {
  upgrade: (res, req, context) => {
    keys.push(req.getHeader('sec-websocket-key'));
    res.upgrade({}, req.getHeader('sec-websocket-key'), req.getHeader('sec-websocket-protocol'), req.getHeader('sec-websocket-extensions'), context);
  },
  open: (ws) => ws.send('hi'),
  message: () => {},
}).listen(port, (token) => {
  if (!token) {
    console.error('Failed to listen to port', port);
    process.exit(1);
  }
  runClient();
});

function runClient () {
  let opens = 0, closes = 0;
  const client = uWS.CliApp().ws({
    open: (ws) => { opens++; ws.end(1000); },
    message: () => {},
    close: () => {
      closes++;
      if (closes < 2) return;
      if (opens !== 2) fail(`expected 2 opens, got ${opens}`);
      if (keys.length !== 2) fail(`server saw ${keys.length} handshakes, expected 2`);
      if (keys.length === 2 && keys[0] === keys[1]) fail('both handshakes carried the same key');
      console.log(`opens ${opens}, server handshake keys ${JSON.stringify(keys)}`);
      console.log(failed ? 'FAILED' : 'OK');
      process.exit(failed ? 1 : 0);
    },
    rejectedHandshake: (status, statusText, body) => fail(`handshake rejected: ${status} ${statusText} ${body}`),
    connectError: (code) => fail(`connect error ${code}`),
  });
  client.connect(`ws://127.0.0.1:${port}/`);
  client.connect(`ws://127.0.0.1:${port}/`);
}

setTimeout(() => { fail('timed out (the second attempt never opened)'); process.exit(1); }, 15000);
