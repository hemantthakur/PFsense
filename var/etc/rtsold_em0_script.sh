#!/bin/sh
# This shell script launches dhcp6c and configured gateways for this interface.
echo $2 > /tmp/em0_routerv6
echo $2 > /tmp/em0_defaultgwv6
if [ -f /var/run/dhcp6c_em0.pid ]; then
	/bin/pkill -F /var/run/dhcp6c_em0.pid
	/bin/sleep 1
fi
/usr/local/sbin/dhcp6c -d -c /var/etc/dhcp6c_wan.conf -p /var/run/dhcp6c_em0.pid em0
/usr/bin/logger -t rtsold "Starting dhcp6 client for interface wan(em0)"
