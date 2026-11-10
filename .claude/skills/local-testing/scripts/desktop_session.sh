#!/bin/bash
set -e
cmd=$1
D=$2
T=$3

usage() {
	echo "usage: desktop_session.sh start|stop <display> <dir>"
	echo "start: Xvfb :<display>, a private D-Bus session with the desktop portal (gtk backend) and private XDG dirs in <dir>;"
	echo "       a directory opened through the portal is appended to <dir>/opened_directories; source <dir>/env to use it"
	echo "stop: the session's whole process group (dbus-daemon and the portals it started) and the Xvfb"
	exit 2
}

[ -n "$cmd" ] && [ -n "$D" ] && [ -n "$T" ] || usage

case $cmd in
start)
	mkdir -p $T/runtime $T/config $T/data/applications
	chmod 700 $T/runtime

	if ! pgrep -f "^Xvfb :$D " > /dev/null; then
		(setsid Xvfb :$D -screen 0 ${SIZE:-1280x720}x24 > /dev/null 2>&1 < /dev/null &)
		sleep 1
	fi

	cat > $T/fake_file_manager.sh <<EOF
#!/bin/sh
printf '%s\n' "\$@" >> $T/opened_directories
EOF
	chmod +x $T/fake_file_manager.sh

	cat > $T/data/applications/hs-test-file-manager.desktop <<EOF
[Desktop Entry]
Type=Application
Name=Test file manager
Exec=$T/fake_file_manager.sh %U
MimeType=inode/directory;
EOF

	printf '[Default Applications]\ninode/directory=hs-test-file-manager.desktop\n' > $T/config/mimeapps.list

	cat > $T/env <<EOF
export DISPLAY=:$D
export XDG_RUNTIME_DIR=$T/runtime
export XDG_CONFIG_HOME=$T/config
export XDG_DATA_HOME=$T/data
export XDG_CURRENT_DESKTOP=i3
EOF

	: > $T/bus_address
	(. $T/env && setsid dbus-run-session -- sh -c "ps -o pgid= \$\$ > $T/session_pgid; echo \$DBUS_SESSION_BUS_ADDRESS > $T/bus_address; exec sleep ${SECS:-1800}" > $T/dbus.out 2>&1 < /dev/null &)

	for _ in $(seq 50); do
		[ -s $T/bus_address ] && break
		sleep 0.1
	done

	[ -s $T/bus_address ] || { echo "no session bus address after 5 s - see $T/dbus.out"; exit 1; }

	echo "export DBUS_SESSION_BUS_ADDRESS=$(cat $T/bus_address)" >> $T/env
	echo "$T/env"
	;;
stop)
	if [ -s $T/session_pgid ]; then
		kill -- -$(tr -d ' ' < $T/session_pgid) 2> /dev/null || true
		: > $T/session_pgid
	fi

	for p in $(pgrep -f "^Xvfb :$D "); do kill $p; done
	;;
*)
	usage
	;;
esac
