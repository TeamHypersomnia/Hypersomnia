# Hypersomnia dedicated server guide (Ubuntu 22.04)

First you'll have to buy a VPS, e.g. you can buy a cheap one here:

https://contabo.com/

This guide is written for **Ubuntu 22.04,** but you should easily be able to run the server on other distributions.

If you already have a working VPS with configured SSH access, skip to [Download AppImage step.](#download-appimage)

## Add an unprivileged user

You'll likely start with a clean VPS with just a root account.
In this case, first add an unprivileged user:

```sh
# This will ask you for password, choose whatever as it won't be used anyway
adduser ubuntu
passwd -d ubuntu
usermod -aG sudo ubuntu
su - ubuntu
```

## Setup SSH

Now we'll setup SSH access.
First generate your SSH keypair, I recommend using ``ssh-ed25519`` algorithm.
Then do this:

```sh
MY_KEY='PUT YOUR PUBLIC KEY HERE'
```

For example (this public key is mine, don't add it unless you want to give me access to your VPS):

```sh
MY_KEY='ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIFpBAHe/KyfrmzWcx5LxbAytckcJj2ssIJYAfNdkg318 ubuntu@hypersomnia.xyz'
```

To authorize this key, do this:

```sh
mkdir -p ~/.ssh
chmod 700 ~/.ssh
echo $MY_KEY >> ~/.ssh/authorized_keys
chmod 600 ~/.ssh/authorized_keys
```

### Disable password login

We want SSH to be the only way to log in to your server.
Backup your ``/etc/ssh/sshd_config`` just in case:

```sh
cp /etc/ssh/sshd_config ./sshd_config.bak
```

The following script will automatically modify the config to disable all options that would enable password logins (so you don't have to edit it manually with ``nano``):

```sh
sudo sed -i '/^PasswordAuthentication/s/^.*$/PasswordAuthentication no/' /etc/ssh/sshd_config
sudo sed -i '/^ChallengeResponseAuthentication/s/^.*$/ChallengeResponseAuthentication no/' /etc/ssh/sshd_config
sudo sed -i '/^UsePAM/s/^.*$/UsePAM no/' /etc/ssh/sshd_config

if ! grep -q "^PasswordAuthentication" /etc/ssh/sshd_config; then
    echo "PasswordAuthentication no" | sudo tee -a /etc/ssh/sshd_config
fi

if ! grep -q "^ChallengeResponseAuthentication" /etc/ssh/sshd_config; then
    echo "ChallengeResponseAuthentication no" | sudo tee -a /etc/ssh/sshd_config
fi

if ! grep -q "^UsePAM" /etc/ssh/sshd_config; then
    echo "UsePAM no" | sudo tee -a /etc/ssh/sshd_config
fi

sudo systemctl restart sshd
```

## Download AppImage

Now download the dedicated server.

```sh
wget https://hypersomnia.xyz/builds/latest/Hypersomnia.AppImage
chmod +x Hypersomnia.AppImage
```

## Install libraries

Install libraries required by the dedicated server's AppImage:

```sh
sudo apt install -y libgl1 libsm6 fuse
```

## Download all community maps

Run this command once, the app will quit once the download is complete:

```sh
./Hypersomnia.AppImage --sync-external-arenas-and-quit
```

The downloaded maps will go to ``~/.config/Hypersomnia/user/downloads/arenas``.

## Setup folders for all server instances

Let's create appdata folders for every server instance we want to run.
I recommend no more than 2 servers per vCore, with no more than 10 slots per server.

```sh
make_server_dir() {
	name="server$1"
	mkdir $name

	ln -s ~/.config/Hypersomnia/user/downloads/arenas $name/user/downloads/arenas
}

make_server_dir "1"
make_server_dir "2"
make_server_dir "3"
make_server_dir "4"
```
