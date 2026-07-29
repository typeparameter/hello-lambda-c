FROM amazonlinux:2023 AS builder

RUN yum install gcc make -y

WORKDIR /workspace
COPY . .
RUN make bootstrap


FROM scratch AS artifact

COPY --from=builder /workspace/build /
