FROM ubuntu

WORKDIR /app
COPY . .
ENV DEBIAN_FRONTEND = noninteractive

RUN apt-get clean \
    && apt-get update \
    && apt-get install -qy git \
    && apt-get install -qy build-essential cmake \
    && apt-get install libssl-dev \

    && mkdir build && cd build; \
        cmake -DCMAKE_BUILD_TYPE=Release ..; \
        make; \
        make install; \
        cd ..; \
        mv docker-entrypoint.sh /entrypoint.sh; \
        chmod -R 755 /entrypoint.sh; \
        cd ..; \
        rm -r app

ENTRYPOINT ["/entrypoint.sh"]
STOPSIGNAL SIGINT
EXPOSE 5600

