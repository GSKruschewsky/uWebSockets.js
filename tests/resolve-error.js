// Run inside the tests folder against the freshly built binaries in ../dist.
// Since v1.0.15 uSockets resolves a host name on a resolver thread, so connect() never
// blocks the loop and every outcome arrives through the behavior's handlers:
//   - connect('ws://localhost:<port>/') resolves the name off the loop and opens;
//   - an unresolvable name reports connectError with a code at or below
//     uWS.CONNECT_ERROR_RESOLVE_BASE (uWS.isResolveError, uWS.resolveErrorCode = EAI_*);
//   - a numeric host on a closed port reports the platform errno (ECONNREFUSED);
//   - a localAddress the host does not own fails synchronously inside uSockets (bind):
//     the errno (EADDRNOTAVAIL) is carried through instead of 0;
//   - connect() returns at once in every case and the loop keeps ticking meanwhile.
const uWS = require('../dist/uws.js');
const os = require('os');

const port = 9014;
let failed = false;
const fail = (msg) => { console.error('FAIL:', msg); failed = true; };
const steps = [];
let ticks = 0;
const ticker = setInterval(() => ticks++, 1);

function step (name, url, hostnameOrOpts, check) {
  steps.push({ name, url, opts: hostnameOrOpts || {}, check });
}

step('resolved name opens', `ws://localhost:${port}/`, {}, (r) => {
  if (r.event !== 'open') fail(`localhost: expected open, got ${r.event} ${r.code ?? ''}`);
});
step('unresolvable name', 'ws://no-such-host.invalid:80/', {}, (r) => {
  if (r.event !== 'connectError') return fail(`invalid name: expected connectError, got ${r.event}`);
  if (!uWS.isResolveError(r.code)) return fail(`invalid name: code ${r.code} is not a resolve error (base ${uWS.CONNECT_ERROR_RESOLVE_BASE})`);
  const eai = uWS.resolveErrorCode(r.code);
  if (!(eai > 0)) fail(`invalid name: resolveErrorCode(${r.code}) = ${eai}`);
  console.log(`unresolvable name -> connectError ${r.code} (EAI ${eai}), lookup took ${r.ms.toFixed(0)} ms`);
});
step('refused port', 'ws://127.0.0.1:1/', {}, (r) => {
  if (r.event !== 'connectError') return fail(`refused: expected connectError, got ${r.event}`);
  if (r.code !== os.constants.errno.ECONNREFUSED) fail(`refused: code ${r.code}, expected ECONNREFUSED ${os.constants.errno.ECONNREFUSED}`);
});
step('synchronous bind failure keeps its errno', `ws://127.0.0.1:${port}/`, { localAddress: '192.0.2.1' }, (r) => {
  if (r.event !== 'connectError') return fail(`bad localAddress: expected connectError, got ${r.event}`);
  if (r.code !== os.constants.errno.EADDRNOTAVAIL) fail(`bad localAddress: code ${r.code}, expected EADDRNOTAVAIL ${os.constants.errno.EADDRNOTAVAIL}`);
});

const server = uWS.App().ws('/*', {
  open: (ws) => ws.send('hi'),
  message: () => {},
}).listen(port, (token) => {
  if (!token) {
    console.error('Failed to listen to port', port);
    process.exit(1);
  }
  runStep(0);
});

function runStep (i) {
  const st = steps[i];
  const t0 = process.hrtime.bigint();
  let done = false;
  const ticksBefore = ticks;
  const finish = (event, code) => {
    if (done) return fail(`${st.name}: second outcome ${event}`);
    done = true;
    const ms = Number(process.hrtime.bigint() - t0) / 1e6;
    st.check({ event, code, ms });
    if (ms > 20 && ticks === ticksBefore) fail(`${st.name}: the loop did not tick during the ${ms.toFixed(0)} ms attempt`);
    if (i + 1 < steps.length) return runStep(i + 1);
    clearInterval(ticker);
    console.log(failed ? 'FAILED' : 'OK');
    process.exit(failed ? 1 : 0);
  };
  const client = uWS.CliApp().ws({
    ...st.opts,
    open: (ws) => { finish('open'); ws.end(1000); },
    message: () => {},
    close: () => {},
    rejectedHandshake: (status, statusText, body) => finish('rejectedHandshake', `${status} ${statusText} ${body}`),
    connectError: (code) => finish('connectError', code),
  });
  const c0 = process.hrtime.bigint();
  client.connect(st.url);
  const callMs = Number(process.hrtime.bigint() - c0) / 1e6;
  if (callMs > 20) fail(`${st.name}: connect() blocked for ${callMs.toFixed(1)} ms`);
  console.log(`${st.name}: connect() returned in ${callMs.toFixed(2)} ms`);
}

setTimeout(() => { fail('timed out'); process.exit(1); }, 20000);
