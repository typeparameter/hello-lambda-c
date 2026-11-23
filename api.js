import { randomUUID } from "node:crypto";
import { createServer } from "node:http";
import { text } from "node:stream/consumers";

function route(method, pathname, handler) {
  return { method, pattern: new URLPattern({ pathname }), handler };
}

const routes = [
  route("GET", "/2018-06-01/runtime/invocation/next", () => {
    const requestId = randomUUID();
    console.info(`Sending request ${requestId} to Lambda function`);

    return {
      status: 200,
      headers: { "Lambda-Runtime-Aws-Request-Id": requestId },
      body: "{}",
    };
  }),

  route("POST", "/2018-06-01/runtime/invocation/:requestId/response", async (req, { requestId }) => {
    const data = await text(req);
    console.info(`Received response from Lambda function for request ${requestId}: ${data}`);
    return { status: 202 };
  }),

  route("POST", "/2018-06-01/runtime/invocation/:requestId/error", (req, { requestId }) => {
    console.info(`Received error from Lambda function for request ${requestId}`);
    return { status: 202 };
  }),
];

const server = createServer(async (req, res) => {
  const pathname = new URL(req.url, "http://localhost").pathname;
  const match = routes.find((r) => r.method === req.method && r.pattern.test({ pathname }));
  const params = match?.pattern.exec({ pathname }).pathname.groups;
  const reply = match ? await match.handler(req, params) : { status: 404 };
  const { status, headers = {}, body = "" } = reply;

  res.writeHead(status, { ...headers, "Content-Length": Buffer.byteLength(body) });
  res.end(body);
});

server.listen(8080);
