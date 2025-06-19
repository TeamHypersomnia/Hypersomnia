FROM ubuntu:22.04

WORKDIR /app

RUN apt update
RUN apt install -y wget
RUN wget https://hypersomnia.xyz/builds/latest/Hypersomnia-Headless.AppImage
RUN chmod +x Hypersomnia-Headless.AppImage

RUN mkdir /root/.config
RUN mkdir /root/.config/Hypersomnia
RUN mkdir /root/.config/Hypersomnia/user
COPY cmake/dockerfile_server_config.json /root/.config/Hypersomnia/user/conf.d/server.json

CMD ["./Hypersomnia-Headless.AppImage", "--appimage-extract-and-run"]

# For native clients via POSIX sockets
EXPOSE 8412/udp

# For Web clients via WebRTC
EXPOSE 9000/udp

