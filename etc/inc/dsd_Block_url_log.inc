<?php
require_once('config.inc');
require_once("gwlb.inc");
require_once("gwlb.inc");
require_once("interfaces.inc");
require_once('filter_log.inc');
require_once('dsdselftest.inc');
require_once('dsdFunctions.inc');
function clear_log_file($logfile = "/var/log/filter.log") {
               echo "call to clear";
		 exec("rm -rf {$logfile}");
                touch($logfile);
}


function blockedUrl(){
	global $config,$g,$buffer_rules_rdr, $buffer_rules_normal;
        $buffer = explode("\n",`pfctl -vvsr | grep '^@'`);
        foreach ($buffer as $line) {
                list($key, $value) = explode (" ", $line, 2);
                $buffer_rules_normal[$key] = $value;
        }
	$filter_logfile = "{$g['varlog_path']}/filter.log";
	exec("cat {$filter_logfile} | grep em1 | grep block",$logarr );
	#clear_logfile();
	$past_log_time=null;
	$past_log_text=null;
	echo sizeof($logarr);
	if(sizeof($logarr)>=1){
	echo "Log aa gya" ;
	foreach ($logarr as $logent){
		 $logent = preg_split("/\s+/", $logent, 6);
		$entry_date_time = htmlspecialchars(join(" ", array_slice($logent, 0, 3)));
		 $entry_text .= htmlspecialchars($logent[4] . " " . $logent[5]);
		if((strcmp($entry_date_time,$past_log_time))&&strcmp($entry_text,$past_log_text))
		{	$entry=	explode("rule",$entry_text);
			$rule=explode("/",$entry[1]);
			 $rule_no='@'.trim($rule[0]);
			 $ipaddr=publicIP();
			 $logDateTime=new DateTime('NOW');
			$alert=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>$buffer_rules_normal[$rule_no]);
	$data=json_encode($alert);
		$url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
		echo $data;
        #	eventsonHttp("$url",$data);
	
		}
		$past_log_time=$entry_date_time;
		$past_log_text=$entry_text;
	}
#clear_log_file();	
}
}



/* ***************DSD Function TO send Alert whenever a new Device get connected **************/


function newDevice(){
global $config,$g;
        $arr=array();
         $filter_logfile = "{$g['varlog_path']}/dhcpd.log";
exec("cat /var/log/dhcpd.log", $logarr);
if(isset($config['dsdlastlogactivity']['dsddhcpdlogindex']))
        (int) $i=$config['dsdlastlogactivity']['dsddhcpdlogindex'];
else
        (int) $i=0;
while($i<sizeof($logarr)){
$logent = preg_split("/\s+/", $logarr[(int) $i-1], 6);
        $entry_date_time = htmlspecialchars(join(" ", array_slice($logent, 0, 3)));
        $logDateTime=new DateTime($entry_date_time);
     $entry_text = ($logent[3] ==  $config['system']['hostname']) ? "" : $logent[3] . " ";
       $entry_text .= htmlspecialchars($logent[4] . " " . $logent[5]);
        if(preg_match("/\bDHCPREQUEST\b/i",$entry_text)){
         $ipaddr=publicIP();
        $logDateTime=new DateTime('NOW');
        $arriy=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dSdmachwval'],'summary'=>$entry_text,'event_type'=>'New_device_Connection');
 $data= json_encode($arriy);
print_r($data);
        $url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
  if(!eventsonHttp("$url",$data))
        {
                return 0;
        }
}
$i=$i+1;
}
 $config['dsdlastlogactivity']['dsddhcpdlogindex']=sizeof($logarr);
           write_config();
/* check for DHCP Lease */
checkDhcplease();
return 1;

}



