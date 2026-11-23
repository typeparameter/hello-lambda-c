# Hello World in C on AWS Lambda

This is a proof-of-concept for writing AWS Lambda functions in the C programming language.

## Frequently Asked Questions

### Why do this?

Why not?

### No really, why do this?

I believe AWS Lambda is one of Amazon's most significant innovations since the company launched as an online bookstore. Since the original website was written in C, this project serves as an homage to Amazon's history.

### Is the code safe?

Absolutely not. But it works.

### How is this tested?

There is a mock AWS Lambda runtime API implemented in `api.js` using Node.js.

#### Process

1. Build the binary: `make example`
2. Run the mock API (Node.js 24 or newer): `node api.js`
3. Run the Lambda: `AWS_LAMBDA_RUNTIME_API=localhost:8080 ./build/bin/bootstrap`

The console for the API should start flowing with logs.

### How fast is it?

Very fast.

![screenshot of successful invocation of the Lambda function showing a duration of 3.15ms and an init duration of 71.18ms](media/screenshot.png)

