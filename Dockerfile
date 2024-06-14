# run with flags --cap-add SYS_ADMIN --device /dev/fuse --security-opt apparmor:unconfined

FROM ubuntu:latest

WORKDIR /app

RUN apt update
RUN apt install -y fuse
RUN apt install -y wget
RUN wget https://hypersomnia.xyz/builds/latest/Hypersomnia-Headless.AppImage
RUN chmod +x Hypersomnia-Headless.AppImage

RUN mkdir /root/.config
RUN mkdir /root/.config/Hypersomnia
RUN mkdir /root/.config/Hypersomnia/user
COPY cmake/dockerfile_server_config.lua /root/.config/Hypersomnia/user/config.lua

CMD ["./Hypersomnia-Headless.AppImage"]

EXPOSE 8412/tcp
EXPOSE 8412/udp
EXPOSE 9000-9100/tcp
EXPOSE 9000-9100/udp

