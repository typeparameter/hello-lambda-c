import * as crypto from "node:crypto";
import express from "express";

const app = express();
app.use(express.text({ type: "*/*" }));

app.get("/2018-06-01/runtime/invocation/next", (_, res) => {
	const requestId = crypto.randomUUID();
	console.info(`Sending request ${requestId} to Lambda function`);

	res.set("Lambda-Runtime-Aws-Request-Id", requestId);
	res.set("Lambda-Runtime-Trace-Id", crypto.randomUUID());
	res.send("{}");
});

app.post("/2018-06-01/runtime/invocation/:requestId/error", (req, res) => {
	const requestId = req.params["requestId"];
	console.info(`Received error from Lambda function for request ${requestId}`);
	res.sendStatus(202);
});

app.post("/2018-06-01/runtime/invocation/:requestId/response", (req, res) => {
	const requestId = req.params["requestId"];
	const data = req.body;
	console.info(`Received response from Lambda function for request ${requestId}: ${data}`);
	res.sendStatus(202);
});

app.listen(8080);
