FROM python:3 as builder

LABEL maintainer="Benjamin Christau <benjamin.christau@luos.io>" \
  app_name="gate_wscom"

ENV PLATFORMIO_CORE_DIR="/home"

WORKDIR /home/app

COPY . .

RUN pip install --upgrade pip setuptools wheel platformio && \
  rm -rf /root/.cache/pip

RUN platformio run \
  --environment native \
  -d ./examples/projects/native/gate_wscom

CMD [ "./examples/projects/native/gate_wscom/.pio/build/native/program"]
