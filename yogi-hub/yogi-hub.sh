#!/bin/sh

### BEGIN INIT INFO
# Provides:          yogi-hub
# Required-Start:    $all
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: YOGI-HUB
# Description:       Web server and central Node for YOGI networks
### END INIT INFO

# Author: Johannes Bergmann <mail@johannes-bergmann.de>

NAME="yogi-hub"
DAEMON="/usr/bin/$NAME"
CONFIG_FILE="/etc/$NAME.json"
DAEMON_ARGS="$CONFIG_FILE"
LOGFILE="/var/log/$NAME"
PIDFILE="/var/run/$NAME.pid"

print_result() {
    if [ $? -ne 0 ]; then
        echo "FAILED"
    else
        echo "OK"
    fi
}

case "$1" in
    start)
        echo -n "Starting daemon $NAME... "
        if [ ! -f $PIDFILE ]; then
            start-stop-daemon --start --quiet --background --make-pidfile --pidfile $PIDFILE --startas /bin/bash -- -c "exec $DAEMON $DAEMON_ARGS > $LOGFILE 2>&1"
            print_result
        else
            echo "SKIPPED"
        fi
        ;;
    stop)
        echo -n "Stopping daemon $NAME... "
        if [ -f $PIDFILE ]; then
            start-stop-daemon --stop --pidfile $PIDFILE
            print_result
            rm -f $PIDFILE
        else
            echo "SKIPPED"
        fi
        ;;
    restart)
        $0 stop
        sleep 1
        $0 start
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac

exit 0

