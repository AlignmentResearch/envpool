FROM nvidia/cuda:12.1.1-cudnn8-devel-ubuntu22.04

ARG HOME=/root

ENV DEBIAN_FRONTEND=noninteractive
ENV PATH=$PATH:$HOME/go/bin

RUN apt-get update \
    && apt-get install -y python3-pip python3.11-dev golang-1.18 git wget curl tmux vim ssh \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
RUN ln -s /usr/bin/python3.11 /usr/bin/python
RUN ln -sf /usr/lib/go-1.18/bin/go /usr/bin/go

# Install Bazel
RUN go install github.com/bazelbuild/bazelisk@latest && ln -sf $HOME/go/bin/bazelisk $HOME/go/bin/bazel
RUN go install github.com/bazelbuild/buildtools/buildifier@latest

ARG USE_BAZEL_VERSION=6.4.0
RUN $HOME/go/bin/bazel version

WORKDIR /app
COPY . .
