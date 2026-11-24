import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import { randomUUID } from "node:crypto";
import { once } from "node:events";
import { createServer } from "node:http";
import { text } from "node:stream/consumers";
import { test } from "node:test";

const bootstrap = process.env.BOOTSTRAP ?? "./build/bin/bootstrap";

function route(method, pathname, handler) {
  return { method, pattern: new URLPattern({ pathname }), handler };
}

test("responds to each invocation", { timeout: 10_000 }, async (t) => {
  const events = ["{}", "{}"];
  const sent = [];
  const results = [];
  const { promise: finished, resolve: finish } = Promise.withResolvers();

  const routes = [
    route("GET", "/2018-06-01/runtime/invocation/next", () => {
      // Like the real runtime API, hold the request open until there is an event.
      if (events.length === 0) return null;

      const requestId = randomUUID();
      sent.push(requestId);
      return { status: 200, headers: { "Lambda-Runtime-Aws-Request-Id": requestId }, body: events.shift() };
    }),

    route("POST", "/2018-06-01/runtime/invocation/:requestId/:kind(response|error)", async (req, { requestId, kind }) => {
      results.push({ requestId, kind, body: await text(req) });
      if (results.length === sent.length && events.length === 0) finish();
      return { status: 202 };
    }),
  ];

  const server = createServer(async (req, res) => {
    const pathname = new URL(req.url, "http://localhost").pathname;
    const match = routes.find((r) => r.method === req.method && r.pattern.test({ pathname }));
    const params = match?.pattern.exec({ pathname }).pathname.groups;
    const reply = match ? await match.handler(req, params) : { status: 404 };
    if (reply === null) return;

    const { status, headers = {}, body = "" } = reply;
    res.writeHead(status, { ...headers, "Content-Length": Buffer.byteLength(body) });
    res.end(body);
  });

  server.listen(0, "127.0.0.1");
  await once(server, "listening");
  const { port } = server.address();

  const runtime = spawn(bootstrap, {
    env: { ...process.env, AWS_LAMBDA_RUNTIME_API: `127.0.0.1:${port}` },
    stdio: ["ignore", "inherit", "inherit"],
  });

  t.after(() => {
    runtime.kill();
    server.closeAllConnections();
    server.close();
  });

  const exited = once(runtime, "exit").then(([code, signal]) => {
    throw new Error(`runtime exited early (code ${code}, signal ${signal})`);
  });
  await Promise.race([finished, exited]);

  assert.deepEqual(results, [
    { requestId: sent[0], kind: "response", body: "Hello, Lambda!" },
    { requestId: sent[1], kind: "response", body: "Hello, Lambda!" },
  ]);
});
