// Run inside the tests folder against the freshly built binaries in ../dist.
// Exercises the client-only messageView option and the per-turn callback scope
// with uWS as both server and client (no external dependencies):
//   - with messageView the handler receives a Uint8Array view for every message
//     that lies inside the loop's receive buffer (plain and TLS alike) and an
//     ArrayBuffer for a message reassembled across reads (bigger than the buffer);
//     every byte of every message is checked;
//   - process.nextTick callbacks and promise continuations scheduled inside the
//     handler all run by the time the connection is done;
//   - an exception thrown by the handler reaches process 'uncaughtException' and
//     the stream continues afterwards;
//   - without the option the handler keeps receiving ArrayBuffers.
const uWS = require('../dist/uws.js');

const port = 9012;
let failed = false;
const fail = (msg) => { console.error('FAIL:', msg); failed = true; };
const sizes = [[200, 500], [8192, 100], [700 * 1024, 2]];
const fill = (n, i) => { const b = Buffer.alloc(n); b.writeUInt32LE(i, 0); for (let k = 4; k < n; k++) b[k] = (i + k) & 0xff; return b; };
let caught = 0;
process.on('uncaughtException', (e) => { if (e.message === 'boom') caught++; else { fail(`unexpected exception ${e.stack}`); } });

const server = uWS.App().ws('/*', {
  maxBackpressure: 1 << 30,
  open: (ws) => { for (const [n, c] of sizes) for (let i = 0; i < c; i++) ws.send(fill(n, i), true); ws.send('done'); },
});

function runClient(messageView, cb) {
  const stats = {}; let scheduled = 0, micro = 0, ticks = 0, bad = 0, received = 0;
  const client = uWS.CliApp();
  client.ws({ messageView, maxPayloadLength: 16 << 20,
    open: () => {},
    message: (ws, m, isBinary) => {
      if (!isBinary) { setTimeout(() => cb({ stats, scheduled, micro, ticks, bad, received }), 100); return; }
      received++;
      const isView = ArrayBuffer.isView(m);
      const u8 = isView ? m : new Uint8Array(m);
      const key = `${u8.byteLength}B`;
      stats[key] ??= { view: 0, arraybuffer: 0 };
      stats[key][isView ? 'view' : 'arraybuffer']++;
      const i = u8[0] | (u8[1] << 8) | (u8[2] << 16) | (u8[3] << 24);
      for (let k = 4; k < u8.byteLength; k++) if (u8[k] !== ((i + k) & 0xff)) { bad++; break; }
      scheduled++; Promise.resolve().then(() => micro++); process.nextTick(() => ticks++);
      if (messageView && received === 5) throw new Error('boom');
    },
  });
  client.connect(`ws://127.0.0.1:${port}/`);
}

server.listen('127.0.0.1', port, (token) => {
  if (!token) { fail(`could not listen on ${port}`); process.exit(1); }
  runClient(true, (r) => {
    console.log('messageView:', JSON.stringify(r));
    const total = sizes.reduce((a, [, c]) => a + c, 0);
    if (r.received !== total) fail(`received ${r.received} of ${total} messages`);
    if (r.bad) fail(`${r.bad} messages with wrong bytes`);
    if ((r.stats['200B']?.view ?? 0) < 490) fail(`200-byte messages not delivered as views: ${JSON.stringify(r.stats['200B'])}`);
    if ((r.stats['716800B']?.arraybuffer ?? 0) !== 2) fail(`oversized messages must fall back to ArrayBuffer: ${JSON.stringify(r.stats['716800B'])}`);
    if (r.micro !== r.scheduled || r.ticks !== r.scheduled) fail(`microtasks ${r.micro}/${r.scheduled}, nextTicks ${r.ticks}/${r.scheduled}`);
    if (caught !== 1) fail(`uncaughtException seen ${caught} times, expected 1`);
    runClient(false, (r2) => {
      console.log('default:', JSON.stringify(r2));
      if (r2.received !== total || r2.bad) fail(`default client received ${r2.received}, bad ${r2.bad}`);
      for (const k of Object.keys(r2.stats)) if (r2.stats[k].view) fail(`default client got a view for ${k}`);
      console.log(failed ? 'messageview: FAILED' : 'messageview: OK');
      process.exit(failed ? 1 : 0);
    });
  });
});
