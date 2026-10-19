// Run inside the tests folder against the freshly built binaries in ../dist.
// Exercises the Host header the client sends and the optional second argument of
// connect(url, hostname), with uWS as both server and client (no external dependencies):
//   - connect('ws://127.0.0.1:<port>/', 'example.test') sends "Host: example.test:<port>"
//     (the name the caller resolved itself, off the loop, while dialing the address;
//     the URL's port is appended because it is not the scheme's default);
//   - connect('ws://127.0.0.1:<port>/', 'example.test:8443') sends the value as given;
//   - connect('ws://127.0.0.1:<port>/') sends the URL's host and port;
//   - a control character in the URL or the hostname makes connect() throw.
const uWS = require('../dist/uws.js');

const port = 9013;
let failed = false;
const fail = (msg) => { console.error('FAIL:', msg); failed = true; };

const expected = [ `example.test:${port}`, 'example.test:8443', `127.0.0.1:${port}` ];
const hostnames = [ 'example.test', 'example.test:8443', undefined ];
const seen = [];
const server = uWS.App().ws('/*', {
  upgrade: (res, req, context) => {
    seen.push(req.getHeader('host'));
    res.upgrade({}, req.getHeader('sec-websocket-key'), req.getHeader('sec-websocket-protocol'), req.getHeader('sec-websocket-extensions'), context);
  },
  open: (ws) => ws.send('hi'),
  message: () => {},
}).listen(port, (token) => {
  if (!token) {
    console.error('Failed to listen to port', port);
    process.exit(1);
  }
  runClient(0);
});

function runClient (i) {
  const client = uWS.CliApp().ws({
    open: (ws) => ws.end(1000),
    message: () => {},
    close: () => {
      if (i + 1 < expected.length) return runClient(i + 1);
      for (let k = 0; k < expected.length; k++) {
        if (seen[k] !== expected[k]) fail(`client ${k} sent Host "${seen[k]}", expected "${expected[k]}"`);
      }
      console.log('Host headers seen by the server:', JSON.stringify(seen));
      console.log(failed ? 'FAILED' : 'OK');
      process.exit(failed ? 1 : 0);
    },
    rejectedHandshake: (status, statusText, body) => fail(`client ${i} handshake rejected: ${status} ${statusText} ${body}`),
    connectError: (code) => fail(`client ${i} connect error ${code}`),
  });
  if (i === 0) {
    for (const [url, hostname] of [[`ws://127.0.0.1:${port}/\r\nX-Injected: 1`, undefined], [`ws://127.0.0.1:${port}/`, 'example.test\r\nX-Injected: 1'], [`ws://127.0.0.1:abc/`, undefined], [`wss://127.0.0.1:${port}/`, undefined]]) {
      let threw = false;
      try { client.connect(url, hostname); } catch (e) { threw = true; }
      if (!threw) fail(`connect(${JSON.stringify(url)}, ${JSON.stringify(hostname)}) did not throw`);
    }
  }
  client.connect(`ws://127.0.0.1:${port}/`, hostnames[i]);
}

setTimeout(() => { fail('timed out'); process.exit(1); }, 10000);
