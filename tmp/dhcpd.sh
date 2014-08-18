/bin/mkdir -p /var/dhcpd
/bin/mkdir -p /var/dhcpd/dev
/bin/mkdir -p /var/dhcpd/etc
/bin/mkdir -p /var/dhcpd/usr/local/sbin
/bin/mkdir -p /var/dhcpd/var/db
/bin/mkdir -p /var/dhcpd/var/run
/bin/mkdir -p /var/dhcpd/usr
/bin/mkdir -p /var/dhcpd/lib
/bin/mkdir -p /var/dhcpd/run
/usr/sbin/chown -R dhcpd:_dhcp /var/dhcpd/*
/bin/cp -n /lib/libc.so.* /var/dhcpd/lib/
/bin/cp -n /usr/local/sbin/dhcpd /var/dhcpd/usr/local/sbin/
/bin/chmod a+rx /var/dhcpd/usr/local/sbin/dhcpd
/sbin/mount -t devfs devfs /var/dhcpd/dev
