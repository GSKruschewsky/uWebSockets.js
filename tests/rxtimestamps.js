// Run inside the tests folder against the freshly built binaries in ../dist.
// Exercises the client-only rxTimestamps option end to end with uWS as both
// server and client (no external dependencies):
//   - with rxTimestamps the message handler receives (ws, message, isBinary,
//     rxTimestampNs: bigint, rxTimestampFromKernel: boolean), the stamp is on
//     the CLOCK_REALTIME clock (uWS.nowNs()) and lies shortly before "now";
//   - on Linux the kernel stamp is present once the connection has settled (the
//     kernel flips its packet-stamping switch from a workqueue the first time a
//     process enables SO_TIMESTAMPING, so only the very first packets may fall
//     back to the userspace stamp, reported as rxTimestampFromKernel=false);
//   - without the option the handler keeps the plain 3-argument signature.
const uWS = require('../dist/uws.js');

const port = 9011;
let failed = false;
const fail = (msg) => { console.error('FAIL:', msg); failed = true; };

const nowNs = uWS.nowNs();
if (typeof nowNs !== 'bigint') fail(`uWS.nowNs() returned ${typeof nowNs}`);
const skewMs = Number(nowNs / 1000000n) - Date.now();
if (Math.abs(skewMs) > 1000) fail(`uWS.nowNs() is ${skewMs} ms away from Date.now()`);
console.log(`uWS.nowNs() = ${nowNs} (skew vs Date.now(): ${skewMs} ms)`);

let phase = 0; // 0: rxTimestamps client, 1: default client
const server = uWS.App().ws('/*', {
  open: (ws) => {
    // Spread the sends out so the kernel switch has settled by the last one
    ws.send('m1');
    setTimeout(() => ws.send('m2'), 60);
    setTimeout(() => ws.send('m3'), 120);
  },
  message: () => {},
}).listen(port, (token) => {
  if (!token) {
    console.error('Failed to listen to port', port);
    process.exit(1);
  }
  runTimestampedClient();
});

function runTimestampedClient () {
  let received = 0;
  uWS.CliApp().ws({
    rxTimestamps: true,
    open: () => console.log('rxTimestamps client connected'),
    message: function (ws, message, isBinary, rxNs, fromKernel) {
      const now = uWS.nowNs();
      const text = Buffer.from(message).toString();
      console.log(`  ${text}: rxTimestampNs=${rxNs} fromKernel=${fromKernel} handler delay=${now - rxNs} ns (args=${arguments.length})`);
      if (arguments.length !== 5) fail(`expected 5 handler arguments, got ${arguments.length}`);
      if (typeof rxNs !== 'bigint' || rxNs === 0n) fail(`rxTimestampNs is ${typeof rxNs} ${rxNs}`);
      if (typeof fromKernel !== 'boolean') fail(`rxTimestampFromKernel is ${typeof fromKernel}`);
      if (now < rxNs) fail('receive timestamp is in the future');
      if (now - rxNs > 1000000000n) fail('receive timestamp is more than 1 s old');
      if (++received === 3) {
        if (process.platform === 'linux' && !fromKernel) fail('third message still carries a userspace fallback stamp on Linux');
        ws.close();
      }
    },
    close: () => {
      if (received !== 3) fail(`rxTimestamps client received ${received} messages`);
      runDefaultClient();
    },
    connectError: (code) => { fail(`connect error ${code}`); process.exit(1); },
  }).connect(`ws://127.0.0.1:${port}`);
}

function runDefaultClient () {
  let received = 0;
  uWS.CliApp().ws({
    open: () => console.log('default client connected'),
    message: function (ws, message, isBinary) {
      if (arguments.length !== 3) fail(`default client: expected 3 handler arguments, got ${arguments.length}`);
      if (++received === 3) ws.close();
    },
    close: () => {
      if (received !== 3) fail(`default client received ${received} messages`);
      finish();
    },
    connectError: (code) => { fail(`connect error ${code}`); process.exit(1); },
  }).connect(`ws://127.0.0.1:${port}`);
}

function finish () {
  console.log(failed ? 'rxTimestamps test FAILED' : 'rxTimestamps test passed');
  process.exit(failed ? 1 : 0);
}

setTimeout(() => { fail('timed out'); finish(); }, 10000);
