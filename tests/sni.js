// Run inside the tests folder against the freshly built binaries in ../dist.
// The TLS SNI is per attempt: two overlapping connects on one CliSSLApp, started before
// either opened, each present the name of their own connect() call (alpha.test / beta.test),
// observed by the server's missingServerName callback (neither name is registered, so the
// default certificate serves them). A third attempt that dials an address literal without a
// hostname sends no SNI at all (RFC 6066 allows no literal), so missingServerName sees nothing
// for it. The self-signed test certificate lives next to this file.
const uWS = require('../dist/uws.js');

const port = 9017;
let failed = false;
const fail = (msg) => { console.error('FAIL:', msg); failed = true; };

const names = [];
const server = uWS.SSLApp({ key_file_name: 'test-key.pem', cert_file_name: 'test-cert.pem' })
  .missingServerName((hostname) => { names.push(hostname); })
  .ws('/*', {
    open: (ws) => ws.send('hi'),
    message: () => {},
  }).listen(port, (token) => {
    if (!token) {
      console.error('Failed to listen to port', port);
      process.exit(1);
    }
    runClients();
  });

function runClients () {
  let closes = 0;
  const opened = [];
  const client = uWS.CliSSLApp().ws({
    open: (ws, attempt) => { opened.push(attempt); ws.end(1000); },
    message: () => {},
    close: () => {
      if (++closes < 3) return;
      const seen = names.slice().sort();
      if (JSON.stringify(seen) !== JSON.stringify([ 'alpha.test', 'beta.test' ])) fail(`server saw SNI ${JSON.stringify(names)}, expected alpha.test and beta.test once each and none for the literal`);
      if (JSON.stringify(opened.sort()) !== JSON.stringify([1, 2, 3])) fail(`open reported attempts ${JSON.stringify(opened)}, expected [1,2,3]`);
      console.log('SNI names seen by the server:', JSON.stringify(names), 'opened attempts', JSON.stringify(opened));
      console.log(failed ? 'FAILED' : 'OK');
      process.exit(failed ? 1 : 0);
    },
    rejectedHandshake: (status, statusText, body, req, attempt) => fail(`attempt ${attempt} handshake rejected: ${status} ${statusText} ${body}`),
    connectError: (code, attempt) => fail(`attempt ${attempt} connect error ${code}`),
  });
  client.connect(`wss://127.0.0.1:${port}/`, 'alpha.test');
  client.connect(`wss://127.0.0.1:${port}/`, 'beta.test');
  client.connect(`wss://127.0.0.1:${port}/`);
}

setTimeout(() => { fail('timed out'); process.exit(1); }, 15000);
