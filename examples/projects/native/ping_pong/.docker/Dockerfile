FROM python:3 as builder

LABEL maintainer="Benjamin Christau <benjamin.christau@luos.io>" \
  app_name="ping_pong"

ENV PLATFORMIO_CORE_DIR="/home"

WORKDIR /home/app

COPY . .

RUN pip install --upgrade pip setuptools wheel platformio && \
  rm -rf /root/.cache/pip

RUN platformio run \
  --environment native \
  -d ./examples/projects/native/ping_pong

CMD [ "./examples/projects/native/ping_pong/.pio/build/native/program"]
