FROM docker.io/library/debian:bookworm-20250610-slim AS downloader
RUN apt-get update && apt-get install -y wget && \
    wget -q https://hypersomnia.io/builds/latest/Hypersomnia-Headless.AppImage && \
    chmod +x Hypersomnia-Headless.AppImage

FROM docker.io/library/debian:bookworm-20250610-slim
WORKDIR /home/hypersomniac

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates libpsl5 libssl3 && \
    rm -rf /var/lib/apt/lists/* && \
    groupadd -r hypersomniac && useradd -r -g hypersomniac hypersomniac && \
    mkdir -p /home/hypersomniac/.config/Hypersomnia/user/conf.d && \
    chown -R hypersomniac:hypersomniac /home/hypersomniac

COPY --from=downloader --chown=hypersomniac:hypersomniac \
    Hypersomnia-Headless.AppImage /home/hypersomniac/Hypersomnia-Headless.AppImage

USER hypersomniac
CMD ["./Hypersomnia-Headless.AppImage", "--appimage-extract-and-run"]

# For native clients via POSIX sockets
EXPOSE 8412/udp

# For Web clients via WebRTC
EXPOSE 9000/udp
