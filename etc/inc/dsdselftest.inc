<?php
require_once('config.inc');
require_once("gwlb.inc");
require_once("gwlb.inc");
require_once("interfaces.inc");
require_once("dsdFunctions.inc");

function diskTest(){
	global $config;
	$disksize=shell_exec("df -h / | grep -v Filesystem | awk '{print $5}' | sed 's/%//g'");	
	$url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
	$ipaddr=publicIP();
	$logDateTime=new DateTime('NOW');
	if($disksize>=13 && $disksize<=90){
	$msg=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>'WARNING! My Disk is going out of Space. '.$disksize. '% remaining Space Only','event_type'=>'preventive_message');
	$data =json_encode($msg);
	if(!eventsonHttp("$url",$data))
        {
                echo "Preventive Message send Successfully not send successfully";
        }

	}
	if($disksize>90)
	{
	$msg=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>'CRITICAL! My Disk is going out of Space. '.$disksize. '% remaining Space Only','event_type'=>'preventive_message');
        $data =json_encode($msg);
        if(!eventsonHttp("$url",$data))
        {
                echo "Preventive Message send Successfully not send successfully";
        }

	
	}

/*echo $disksize."\n";
	$ds=disk_total_space("/");
	$ds=$ds/pow(1024,3);
	echo $ds."\n";
	$df=disk_free_space("/");
	$df=$df/pow(1024,3);
	echo $df."\n";
*/
}
function memTest(){
	 global $config;
	$free=exec("vmstat | grep 0 | awk '{print $5}'");
	$avail=exec("vmstat | grep 0 | awk '{print $4}'");
	$frepercntg=($free/$avail)*100;
	$url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
        $ipaddr=publicIP();
        $logDateTime=new DateTime('NOW');
        if($frepercntg>50 && $frepercntg<=60){
        $msg=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>'WARNING! My Memory is going out of Space. '.$frepercntg. '% remaining Space Only','event_type'=>'preventive_message');
        $data =json_encode($msg);
        if(!eventsonHttp("$url",$data))
        {
                echo "Preventive Message send Successfully not send successfully";
        }

        }
        if($frepercntg<=50)
        {
        $msg=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>'CRITICAL! My Memory is going out of Space. '.$frepercntg. '% remaining Space Only','event_type'=>'preventive_mes');
        $data =json_encode($msg);
        if(!eventsonHttp("$url",$data))
        {
                echo "Preventive Message send Successfully not send successfully";
        }

	  	
	#var_dump($b);	 


} 
}

function checkwansoc(){
	global $config;
	if(!checkConnectivity("http://www.google.com")){
                         if(!connectionFailure("http://www.wikipedia.org"))
                         {
                                 if(!connectionFailure("http://www.tutorialspoint.com"))
                                       checkwansoc(); 
                        }
                }
		if(checkdnsrr("theashokaresidency.co.in","A"))
        	{
        		if(!checkConnectivity())
         		     	checkwansoc();
		}
		else if(checkdnsrr("tkenterprisesagra.in","A")) 
		{
			if(!checkConnectivity())
                                checkwansoc();
	
		}
	}
	$nw=strtotime(date('Y-m-d H:m:s'));
	$hrtbttm=strtotime($config[dsdheartbeatfq][dsdhrtbttim]);
	if(round(abs($nw - $hrtbttm) / 60,2)>=4){
	$logDateTime=new DateTime('NOW');
	$msg=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>'Alert! My WAN or SOC Connectivity is lost since '.$config[dsdheartbeatfq][dsdhrtbttim],'event_type'=>'preventive_message');
	$data =json_encode($msg);
        $url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
	if(!eventsonHttp("$url",$data))
        {
                echo "Preventive Message send Successfully not send successfully";
        }	
}


/* Removes the Duplicate DHCP IP address
	***
	DHCP
	***
*/


function remove_duplicate($array, $field)
{
  foreach ($array as $sub)
   $cmp[] = $sub[$field];
  $unique = array_unique(array_reverse($cmp,true));
  foreach ($unique as $k => $rien)
   $new[] = $array[$k];
  return $new;
	
}

function checkDhcplease(){
	global $config,$g;
        $leasesfile = "{$g['dhcpd_chroot_path']}/var/db/dhcpd.leases";
        #$leases_contents = file($leasesfile);
        $newleases_contents = array();
        $i=0;
        $dhcp_range_from=array_pop(explode('.', $config['dhcpd']['lan']['range']['from']));
        $dhcp_range_to=array_pop(explode('.',$config['dhcpd']['lan']['range']['to']));
        $total_leases_count=$dhcp_range_to - $dhcp_range_from;
        $awk = "/usr/bin/awk";
        $cleanpattern = "'{ gsub(\"#.*\", \"\");} { gsub(\";\", \"\"); print;}'";
        $splitpattern = "'BEGIN { RS=\"}\";} {for (i=1; i<=NF; i++) printf \"%s \", \$i; printf \"}\\n\";}'";
        exec("/bin/cat {$leasesfile} | {$awk} {$cleanpattern} | {$awk} {$splitpattern}", $leases_content);
        $leases_count = count($leases_content);
        exec("/usr/sbin/arp -an", $rawdata);
        $arpdata = array();
        foreach ($rawdata as $line) {
        $elements = explode(' ',$line);
        if ($elements[3] != "(incomplete)") {
                $arpent = array();
                $arpent['ip'] = trim(str_replace(array('(',')'),'',$elements[1]));
		  // $arpent['mac'] = trim($elements[3]);
                // $arpent['interface'] = trim($elements[5]);
        $arpdata[] = $arpent['ip'];
        }
        }
        unset($rawdata);
        $pools = array();
        $leases = array();
        $i = 0;
        $l = 0;
        $p = 0;
        foreach($leases_content as $lease) {
        /* split the line by space */
        $data = explode(" ", $lease);
        /* walk the fields */
        $f = 0;
        $fcount = count($data);
        /* with less then 20 fields there is nothing useful */
        if($fcount < 20) {
                $i++;
                continue;
        }
        while($f < $fcount) {
                switch($data[$f]) {
                        case "failover":
                                $pools[$p]['name'] = $data[$f+2];
                                $pools[$p]['mystate'] = $data[$f+7];
                                $pools[$p]['peerstate'] = $data[$f+14];
                                $pools[$p]['mydate'] = $data[$f+10];
                                $pools[$p]['mydate'] .= " " . $data[$f+11];
                                $pools[$p]['peerdate'] = $data[$f+17];
                                $pools[$p]['peerdate'] .= " " . $data[$f+18];
                                $p++;
                                $i++;
                                continue 3;
				 case "lease":
                                $leases[$l]['ip'] = $data[$f+1];
                                $leases[$l]['type'] = "dynamic";
                                $f = $f+2;
                                break;
                        case "starts":
                                $leases[$l]['start'] = $data[$f+2];
                                $leases[$l]['start'] .= " " . $data[$f+3];
                                $f = $f+3;
                                break;
                         case "ends":
                                $leases[$l]['end'] = $data[$f+2];
                                $leases[$l]['end'] .= " " . $data[$f+3];
                                $f = $f+3;
                                break;
                        case "tstp":
                                $f = $f+3;
                                break;
                        case "tsfp":
                                $f = $f+3;
                                break;
                        case "atsfp":
                                $f = $f+3;
                                break;
                        case "cltt":
                                $f = $f+3;
                                break;
                        case "binding":
                                switch($data[$f+2]) {
                                        case "active":
                                                $leases[$l]['act'] = "active";
                                                break;
                                        case "free":
                                                $leases[$l]['act'] = "expired";
                                                $leases[$l]['online'] = "offline";
                                                break;
                                        case "backup":
                                                $leases[$l]['act'] = "reserved";
					$leases[$l]['online'] = "offline";
                                                break;
                                }
                                $f = $f+1;
                                break;
                        case "next":
                                /* skip the next binding statement */
                                $f = $f+3;
                                break;

                        case "rewind":
                                /* skip the rewind binding statement */
                                $f = $f+3;
                                break;
                        case "hardware":
                                $leases[$l]['mac'] = $data[$f+2];
                                /* check if it's online and the lease is active */
                                if (in_array($leases[$l]['ip'], $arpdata)) {
                                        $leases[$l]['online'] = 'online';
                                } else {
                                        $leases[$l]['online'] = 'offline';
                                }
                                $f = $f+2;
                                break;
                        case "client-hostname":
                                if($data[$f+1] <> "") {
                                        $leases[$l]['hostname'] = preg_replace('/"/','',$data[$f+1]);
                                } else {
                                        $hostname = gethostbyaddr($leases[$l]['ip']);
                                        if($hostname <> "") {
                                                $leases[$l]['hostname'] = $hostname;
                                        }
                                }
                                $f = $f+1;
                                break;
                        case "uid":
                                $f = $f+1;

		 break;
                }
                $f++;
        }
        $l++;
        $i++;
        /* slowly chisel away at the source array */
        array_shift($leases_content);
}
/* remove the old array */
unset($lease_content);

/* remove duplicate items by mac address */
if(count($leases) > 0) {
        $leases = remove_duplicate($leases,"ip");
}
$ipused=0;
foreach($leases as $lease){
        foreach($lease as $leas => $status){
                if($leas=='act' && $status=='active')
                $ipused=$ipused+1;
        }
}
$remaining_lease=$total_leases_count-$ipused;
echo $remaining_lease;
	if($remaining_lease<5){
		 $logDateTime=new DateTime('NOW');
		$ipaddr=publicIP();
       		 $msg=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>'Alert! I have only '.$remaining_lease.' IP remaining in my DHCP Pool ','event_type'=>'preventive_message');	
	$data =json_encode($msg);
        $url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
        if(!eventsonHttp("$url",$data))
        	return 0;
    }
return 1;
}



