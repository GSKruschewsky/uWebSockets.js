// Run inside the tests folder against the freshly built binaries in ../dist.
// connectTimeout bounds the name lookup plus the TCP connect: an attempt whose SYN is
// never answered (TEST-NET-1 address, blackholed on every ordinary network) fails with
// connectError(ETIMEDOUT) after the option's seconds (4-second timer granularity)
// instead of hanging for the kernel's own connect timeout (minutes). The loop keeps
// ticking meanwhile and a connect started from inside the callback works. The callback
// names the attempt the timeout belongs to.
// If the host answers the SYN at once with "unreachable" there is no blackhole to time
// out on; the test then reports that and passes, since the code path cannot be reached.
const uWS = require('../dist/uws.js');
const os = require('os');

let failed = false;
const fail = (msg) => { console.error('FAIL:', msg); failed = true; };
let ticks = 0;
const ticker = setInterval(() => ticks++, 1);
const started = Date.now();
let attempts = 0;

const client = uWS.CliApp().ws({
  connectTimeout: 1,
  open: (ws) => fail('opened a blackholed address'),
  message: () => {},
  close: () => {},
  rejectedHandshake: (status, statusText, body) => fail(`handshake rejected: ${JSON.stringify(status)} ${JSON.stringify(body)}`),
  connectError: (code, attempt) => {
    const elapsed = Date.now() - started;
    attempts++;
    if (attempt !== attempts) fail(`connectError reported attempt ${attempt}, expected ${attempts}`);
    if (code === os.constants.errno.ENETUNREACH || code === os.constants.errno.EHOSTUNREACH) {
      console.log(`no blackhole route on this host (errno ${code}); the timeout path cannot be exercised here`);
      console.log('OK');
      process.exit(0);
    }
    if (code !== os.constants.errno.ETIMEDOUT) fail(`attempt ${attempts}: expected ETIMEDOUT (${os.constants.errno.ETIMEDOUT}), got ${code}`);
    if (elapsed > 12000) fail(`attempt ${attempts}: timed out after ${elapsed} ms, expected within ~8 s`);
    if (ticks < 50) fail(`the loop ticked only ${ticks} times while the attempt was pending`);
    console.log(`attempt ${attempts}: connectError ${code} after ${elapsed} ms (loop ticks ${ticks})`);
    if (attempts === 1) {
      /* A re-dial from inside the callback must be timed out the same way */
      const n = client.connect('ws://192.0.2.1:9/');
      if (n !== 2) fail(`second connect() returned ${n}, expected 2`);
      return;
    }
    clearInterval(ticker);
    console.log(failed ? 'FAILED' : 'OK');
    process.exit(failed ? 1 : 0);
  },
});
const first = client.connect('ws://192.0.2.1:9/');
if (first !== 1) fail(`first connect() returned ${first}, expected 1`);

setTimeout(() => { fail('no connectError within 30 s'); process.exit(1); }, 30000);
