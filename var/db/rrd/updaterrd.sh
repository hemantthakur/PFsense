#!/bin/sh

export TERM=dumb

echo $$ > /var/run/updaterrd.sh.pid
counter=1
while [ "$counter" -ne 0 ]
do

# polling traffic for interface wan em0 IPv4/IPv6 counters 
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/wan-traffic.rrd N:`/sbin/pfctl -vvsI -i em0 | awk '\
/In4\/Pass/ { b4pi = $6 };/Out4\/Pass/ { b4po = $6 };/In4\/Block/ { b4bi = $6 };/Out4\/Block/ { b4bo = $6 };\
/In6\/Pass/ { b6pi = $6 };/Out6\/Pass/ { b6po = $6 };/In6\/Block/ { b6bi = $6 };/Out6\/Block/ { b6bo = $6 };\
END {print b4pi ":" b4po ":" b4bi ":" b4bo ":" b6pi ":" b6po ":" b6bi ":" b6bo};'`

# polling packets for interface wan em0 
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/wan-packets.rrd N:`/sbin/pfctl -vvsI -i em0 | awk '\
/In4\/Pass/ { b4pi = $4 };/Out4\/Pass/ { b4po = $4 };/In4\/Block/ { b4bi = $4 };/Out4\/Block/ { b4bo = $4 };\
/In6\/Pass/ { b6pi = $4 };/Out6\/Pass/ { b6po = $4 };/In6\/Block/ { b6bi = $4 };/Out6\/Block/ { b6bo = $4 };\
END {print b4pi ":" b4po ":" b4bi ":" b4bo ":" b6pi ":" b6po ":" b6bi ":" b6bo};'`

# polling traffic for interface lan em1 IPv4/IPv6 counters 
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/lan-traffic.rrd N:`/sbin/pfctl -vvsI -i em1 | awk '\
/In4\/Pass/ { b4pi = $6 };/Out4\/Pass/ { b4po = $6 };/In4\/Block/ { b4bi = $6 };/Out4\/Block/ { b4bo = $6 };\
/In6\/Pass/ { b6pi = $6 };/Out6\/Pass/ { b6po = $6 };/In6\/Block/ { b6bi = $6 };/Out6\/Block/ { b6bo = $6 };\
END {print b4pi ":" b4po ":" b4bi ":" b4bo ":" b6pi ":" b6po ":" b6bi ":" b6bo};'`

# polling packets for interface lan em1 
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/lan-packets.rrd N:`/sbin/pfctl -vvsI -i em1 | awk '\
/In4\/Pass/ { b4pi = $4 };/Out4\/Pass/ { b4po = $4 };/In4\/Block/ { b4bi = $4 };/Out4\/Block/ { b4bo = $4 };\
/In6\/Pass/ { b6pi = $4 };/Out6\/Pass/ { b6po = $4 };/In6\/Block/ { b6bi = $4 };/Out6\/Block/ { b6bo = $4 };\
END {print b4pi ":" b4po ":" b4bi ":" b4bo ":" b6pi ":" b6po ":" b6bi ":" b6bo};'`

# polling traffic for interface ipsec enc0 IPv4/IPv6 counters 
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/ipsec-traffic.rrd N:`/sbin/pfctl -vvsI -i enc0 | awk '\
/In4\/Pass/ { b4pi = $6 };/Out4\/Pass/ { b4po = $6 };/In4\/Block/ { b4bi = $6 };/Out4\/Block/ { b4bo = $6 };\
/In6\/Pass/ { b6pi = $6 };/Out6\/Pass/ { b6po = $6 };/In6\/Block/ { b6bi = $6 };/Out6\/Block/ { b6bo = $6 };\
END {print b4pi ":" b4po ":" b4bi ":" b4bo ":" b6pi ":" b6po ":" b6bi ":" b6bo};'`

# polling packets for interface ipsec enc0 
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/ipsec-packets.rrd N:`/sbin/pfctl -vvsI -i enc0 | awk '\
/In4\/Pass/ { b4pi = $4 };/Out4\/Pass/ { b4po = $4 };/In4\/Block/ { b4bi = $4 };/Out4\/Block/ { b4bo = $4 };\
/In6\/Pass/ { b6pi = $4 };/Out6\/Pass/ { b6po = $4 };/In6\/Block/ { b6bi = $4 };/Out6\/Block/ { b6bo = $4 };\
END {print b4pi ":" b4po ":" b4bi ":" b4bo ":" b6pi ":" b6po ":" b6bi ":" b6bo};'`

pfctl_si_out="` /sbin/pfctl -si > /tmp/pfctl_si_out `"
pfctl_ss_out="` /sbin/pfctl -ss > /tmp/pfctl_ss_out`"
pfrate="` cat /tmp/pfctl_si_out | egrep "inserts|removals" | awk '{ pfrate = $3 + pfrate } {print pfrate}'|tail -1 `"
pfstates="` cat /tmp/pfctl_ss_out | egrep -v "<\-.*?<\-|\->.*?\->" | wc -l|sed 's/ //g'`"
pfnat="` cat /tmp/pfctl_ss_out | egrep '<\-.*?<\-|\->.*?\->' | wc -l|sed 's/ //g' `"
srcip="` cat /tmp/pfctl_ss_out | egrep -v '<\-.*?<\-|\->.*?\->' | grep '\->' | awk '{print $3}' | awk -F: '{print $1}' | sort -u|wc -l|sed 's/ //g' `"
dstip="` cat /tmp/pfctl_ss_out | egrep -v '<\-.*?<\-|\->.*?\->' | grep '<\-' | awk '{print $3}' | awk -F: '{print $1}' | sort -u|wc -l|sed 's/ //g' `"
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/system-states.rrd N:$pfrate:$pfstates:$pfnat:$srcip:$dstip

CPU=`/usr/local/sbin/cpustats | cut -f1-4 -d':'`
PROCS=`ps uxaH | wc -l | awk '{print $1;}'`
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/system-processor.rrd N:${CPU}:${PROCS}
MEM=`/sbin/sysctl -n vm.stats.vm.v_page_count vm.stats.vm.v_active_count vm.stats.vm.v_inactive_count vm.stats.vm.v_free_count vm.stats.vm.v_cache_count vm.stats.vm.v_wire_count |  /usr/bin/awk '{getline active;getline inactive;getline free;getline cache;getline wire;printf ((active/$0) * 100)":"((inactive/$0) * 100)":"((free/$0) * 100)":"((cache/$0) * 100)":"(wire/$0 * 100)}'`
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/system-memory.rrd N:${MEM}
MBUF=`/usr/bin/netstat -m |  /usr/bin/awk '/mbuf clusters in use/ { gsub(/\//, ":", $1); print $1; }'`
/usr/bin/nice -n20 /usr/local/bin/rrdtool update /var/db/rrd/system-mbuf.rrd N:${MBUF}
sleep 60
done
