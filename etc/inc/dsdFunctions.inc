<?php
require_once('config.inc');
require_once("gwlb.inc");
require_once("gwlb.inc");
require_once("interfaces.inc");


/*  ********* Functions to check the Internet Connectivity or SOC connectivity*******************/ 


function checkConnectivity($url=null)
{

   $soccheck=false;
    if($url==NULL){
        $url='http://10.10.1.133:9200'; 
        $soccheck=true;
	}
    $ch = curl_init($url);
   if($soccheck){
	  $field=array('name'=>'test','iden'=>'12345');
	$fieldjson=json_encode($field);
	curl_setopt($ch, CURLOPT_POST, true);
	curl_setopt($ch, CURLOPT_POSTFIELDS, $fieldjson);

	}
    curl_setopt($ch, CURLOPT_TIMEOUT, 5);
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 5);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $data = curl_exec($ch) ;
    $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);
    if($httpcode>=200 && $httpcode<600){
        return true;
    } else if($httpcode==0) {
        if($soccheck)
                {
		return false;
	
	}
        return false;
    }
 return true;
}





/* *******************  FUNCTION TO BACKUP THE FILE ****************** */

function backupFile($filename)
{
global $config,$g;
require_once('config.lib.inc');
conf_mount_rw();
$new_file=$g['system_preferences_updates_path'].'/'.basename($filename);
 if(!file_exists($g['system_preferences_updates_path']))
	exec("/bin/mkdir -p {$g['system_preferences_updates_path']}/backup");
 if(file_exists($new_file)){ 
 for($j=3;$j>=1;$j--)
        {
                $old_back_file=$g['system_preferences_updates_path']."/"."backup"."/".basename($filename).'.'.$j;
                if(file_exists($old_back_file)){
                        if($j != 1){
                                $prefile=$g['system_preferences_updates_path']."/"."backup"."/".basename($filename).'.'.($j-1);}
                        else
                                $prefile=$new_file;
                        unlink($old_back_file);
                        copy($prefile,$old_back_file);
                        echo "Backup successfully done\n";
                }
                else{
                        copy($new_file,$old_back_file) ;
                        echo "Backup created\n";
                        }
	}
}
else{
	for($j=1;$j<=3;$j++)
	{
		$back_file=$g['system_preferences_updates_path']."/"."backup"."/".basename($filename).'.'.$j;
		copy($filename,$back_file);
	}	

}
	copy($filename,$new_file);
	conf_mount_ro();
	return true;
}
/* ****************** FUNCTION TO EXTRACT THE TAR FILE AND STORE THE CONTENT OF FILE IN XML FILE  ***************  */


function extract_tar_data($file_name, $path) {
    global $g;
    if (!$g['sys_update_downloading_status']) {

    } else {
        echo "Extracting,,,,,";
        $tar_file = $path . '/' . $file_name;
        exec("/usr/bin/tar xf {$tar_file} -C {$path}");
        $files = scandir($path, 1);
        #check out if there are any comment in the files  if then remove them
        $i = 1;
        foreach ($files as $files) {
            if (!(($files == $file_name) or ( $files == '.') or ( $files == '..'))) {
                $reg = array();
                $filename = $path . '/' . $files;
                $destfile = fopen($filename . $i, 'w');
                $handle = @fopen($filename, "r");
                if ($handle) {
                    while (($buffer = fgets($handle, 4096)) !== false) {

                        $start = strpos($buffer, "#");
    $end = strpos($buffer, "\n");
                        if ($start !== false)
                            $res = substr($buffer, 0, $start);
                        else
                            $res = $buffer;
                        $a = explode("=", $res);
                        if (count($a) > 0) {
                            if (count($a) == 1 && !empty($a[0]) && trim($a[0]) != "")
                                $reg[$a[0]] = "";
                            else {
                                if (!empty($a[0]) && trim($a[0]) != "")
                                    $reg[$a[0]] = $a[1];
                            }
                            fwrite($destfile, $a[0]);
                        }
                    }
                    if (!feof($handle)) {
                        echo "Error: unexpected fgets() fail\n";
                    }
                    fclose($handle);
                    fclose($destfile);
                }
		backupFile($filename.$i);
		unlink($filename);
		unlink($filename.$i);
        }
      }
    }
return true;
}





/* ********function To download System Preferences Updates ******* */



function downloadSystemUpdateFile(){
global $config,$g;
#$file_location= 'https://'.$config['dsdpubip']['dsdpubipval'].'/per_user_preferences/system_preference_update/system_preferences.tar.bz2';
      # $file_location='http://192.168.1.100/system_preferences.tar.bz2';
        $file_name= trim(basename($file_location));
        $path='/tmp/dsdupdate';
        if(!file_exists($path))
                exec("mkdir /tmp/dsdupdate");
        $file = fopen($path.'/'.$file_name, 'w');
        $g['sys_update_downloading_status']=true;
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL,$file_location);
        curl_setopt($ch, CURLOPT_FAILONERROR, true);
        curl_setopt($ch, CURLOPT_TIMEOUT, 3);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 3);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
         curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
        curl_setopt($ch, CURLOPT_FILE, $file);
        curl_exec($ch) or die("error".curl_error($ch));
        if(curl_error($ch)){
		unset($g['sys_update_downloading_status']);
                return false;
        }
        curl_close($ch);
        fclose($file);
	echo "Call to extract function,...";
       if(extract_tar_data($file_name,$path));
		{	unlink($path.'/'.$file_name);
			exec("/bin/rm -rf {$path}");
			return true;	
		}
	return false;
}

 


/* **************** function to write the user configurations **************** */


function writeUpdate($result){
	 global $config;
	echo $result;
                $json_array=json_decode($result);
                $monitor_url[]= $json_array->monitored_features;
                 $blocked_url[]=$json_array->blocked_features;
                $allowed_url[]=$json_array->allowed_features;
                 $security_level=$json_array->security_level;
                foreach($blocked_url as $block){
                        foreach($block as $blurl)
                                {
                                $name=explode(".",$blurl);
        $config[aliases][alias][]=array("name"=>$name[1],"address"=>$blurl,"descr"=>"Block ".$name[1],"type"=>'network',"detail"=>"Entry Added ".date('r'));
        $config[filter][rule][]=array("id"=>"","type"=>'reject',"interface"=>'lan',"ipprotocol"=>"inet","tag"=>"","tagged"=>"","max"=>"","max-src-nodes"=>"","max-src-conn"=>"","max-src-states"=>"","state-timout"=>"","statetype"=>"keep state","os"=>"","protocol"=>"tcp","source"=>array("any"=>
""),"destination"=>array("address"=>$name[1],"port"=>"80-443"),"log"=>"","descr"=>"");
		}
	}
	 foreach($monitor_url as $monitor){
                foreach($monitor as $mntrurl)
                             {
                                $name=explode(".",$mntrurl);
       $config[aliases][alias][]=array("name"=>$name[1],"address"=>$mntrurl,"descr"=>"Monitored ".$name[1],"type"=>'network',"detail"=>"Entry Added ".date('r'));
/*        $config[filter][rule][]=array("id"=>"","type"=>'monitor',"interface"=>'lan',"ipprotocol"=>"inet","tag"=>"","tagged"=>"","max"=>"","max-src-nodes"=>"","max-src-conn"=>"","max-src-states"=>"","state-timout"=>"","statetype"=>"keep state","os"=>"","protocol"=>"tcp","source"=>array("any"=>""),"destination"=>array("address"=>$name[1],"port"=>"80-443"),"log"=>"","descr"=>""); */
                                }
                }
         foreach($allowed_url as $allow){
                foreach($allow as $alwrurl)
                                {
                                $name=explode(".",$alwrurl);
        $config[aliases][alias][]=array("name"=>$name[1],"address"=>$alwrurl,"descr"=>"Allowed ".$name[1],"type"=>'network',"detail"=>"Entry Added ".date('r'));
        $config[filter][rule][]=array("id"=>"","type"=>'pass',"interface"=>'lan',"ipprotocol"=>"inet","source"=>array("network"=>"lan"),"destination"=>array("any"=>""),"descr"=>"");
                                }
                }
		ksort($config[filter][rule]);
		ksort($config[aliases][alias]);
       # write_config();
   return true;
}


/***********   Funtion to send Events to SOC ********************/

function eventsonHttp($url, $data, $hrtbt = null) {
    global $config,$g;
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_TIMEOUT, 3);
    curl_setopt($curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_setopt($curl, CURLOPT_POST, true);
    curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
    curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false);
    if ($hrtbt == "heartbeat")
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    $result = curl_exec($curl);
echo $result;
    if ($hrtbt == "heartbeat") {
	$rsltarr=json_decode($result);
	if($rsltarr->system_preferences_update=='YES')
		{
		$ipaddr = publicIP();
                $logDateTime = new DateTime('NOW');
     $arr = array('source_ip_address' => $ipaddr, 'sendtimestamp' => $logDateTime->format(DateTime::ISO8601), 'tags' => $config['dsdmachwaddr']['dsdmachwval'], 'summary' => "System Updated Successfully", 'event_type' => 'system_preference_update_success');
                     $data = json_encode($arr);
                     $url = 'https://' . $config['dsdpubip']['dsdpubipval'] . ':8080/events/';
                     eventsonHttp("$url", $data);
	}  if ($rsltarr->user_preferences_update=='YES') {
          /* if (writeUpdate($result)) { */
                $ipaddr = publicIP();
                $logDateTime = new DateTime('NOW');
  $arr = array('source_ip_address' => $ipaddr, 'sendtimestamp' => $logDateTime->format(DateTime::ISO8601), 'tags' => $config['dsdmachwaddr']['dsdmachwval'], 'summary' => "Updated Successfully", 'event_type' => 'user_preference_update_success');
                $data = json_encode($arr);
                $url = 'https://' . $config['dsdpubip']['dsdpubipval'] . ':8080/events/';
                eventsonHttp("$url", $data);
           # }
        }  if ($rsltarr->firmware_preferences_update=='YES') 
           {
		echo "Firmawra";
		} 
    }

    if (curl_error($curl)) {
        $p = shell_exec("nslookup theashokareidency.co.in");
        if (!checkConnectivity("http://www.google.com")) {
            if (!checkConnectivity("http://www.wikipedia.org")) {
                if (!checkConnectivity("http://www.tutorialspoint.com")) {
                    echo "check Your Internet Connection\n";
                    return 0;
                }
            }
        }
        if (!preg_match("/NXDOMAIN/i", $p)) {
            echo "NXDOMAIN Error\n";
            return 0;
        }
        if (!checkdnsrr("theashokaresidency.co.in", "A")) {
            echo "DNS failure Error\n";
            return 0;
        }

        if (!checkConnectivity()) {
            echo "Check Your SOC Connectivity\n";
            return 0;
        }
    }

    return true;
}


/*    ********  function to GET the Public IP address of the DSD Box***************/

function publicIP(){
global $config;
$iflist = get_configured_interface_with_descr(false, true);
foreach($iflist as $ifname => $friendly) {
                $ifconf = $config['interfaces'][$ifname];
                switch($ifconf['ipaddr']) {
                case "dhcp":
                        $class = "/DHCP4";
                        break;
                case "pppoe":
                        $class = "/PPPoE";
                        break;
                case "pptp":
                        $class = "/PPTP";
                        break;
                case "l2tp":
                        $class = "/L2TP";
                        break;
                default:
                        $class = "";
                        break;
                }
                switch($ifconf['ipaddrv6']) {
                case "dhcp6":
                        $class6 = "/DHCP6";
                        break;
                case "slaac":
                        $class6 = "/SLAAC";
                        break;
                case "6rd":
                        $class6 = "/6RD";
                        break;
                case "6to4":
                        $class6 = "/6to4";
                        break;
                case "track6":
                        $class6 = "/t6";
                        break;
                }
	      if($friendly=='wan' or $friendly=='WAN')
                          $ipaddr = get_interface_ip($ifname);

        }
return $ipaddr;
}





/*  ********** Function to send Disk Error Alert To SOC ****************/

function diskCheckError(){
        global $config, $g;
        if($g['booting']){
        echo "BOOTING";
                exec("cat /var/log/system.log", $logarr);
                (int)$i=$config['dsdlastlogactivity']['dsdsyslogindex'];
                if($i<sizeof($logarr)){
                        while($i<sizeof($logarr)){
                                $logent = preg_split("/\s+/", $logarr[(int) $i], 6);
                                    $entry_date_time = htmlspecialchars(join(" ", array_slice($logent, 0, 3)));
                                          if(strlen($entry_date_time) > 15){break;}
                                else{
                                  $logDateTime=new DateTime($entry_date_time);
                                 $entry_text = ($logent[3] ==  $config['system']['hostname']) ? "" : $logent[3] . " ";
                                  $entry_text .= htmlspecialchars($logent[4] . " " . $logent[5]);
                                if (preg_match('/(uncorrectable sectors|checksum mismatch|I\/O failure|Currently unreadable)/', $entry_text)) {
                                          $ipaddr=publicIP();
              $msg[]=array('sendtimestamp'=> $logDateTime->format(DateTime::ISO8601),'summary'=>'Alert! DISK_ERROR:'.$entry_text);
                                }
                                 else echo "Not Matched\n";
                           }
			$i=$i+1;
                        }
		}
			if(sizeof($msg)>2){
       $arr=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>$msg,'event_type'=>'preventive_message');
       $data= json_encode($arr);
       $url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
        if(!eventsonHttp("$url",$data))
                return 0;
	}
	return 1;
}
}





/* ********* Functions to send the Last Known Activity of the DSD ***********/

function lastknownActivity(){
global $config,$g;
if($g[booting]){
 if(!isset($config['dsdlastlogactivity']['dsdsyslogindex'])){
	 $config['dsdlastlogactivity']['dsdsyslogindex']='0';
	 write_config();
	}
if($g['booting'] && !isset($config['dsdpubip']['dsdpubipval'])){
        $file='/etc/rc.soc';
        $config['dsdpubip']['dsdpubipval']=file_get_contents($file);
 		write_config();
	}
	if($g['booting'] && !isset($config['dsdmachwaddr']['dsdmachwval'])){
	$dsdhostname=uniqid(); 
	$config['system']['hostname']=$dsdhostname;	
        $config['dsdmachwaddr']['dsdmachwval']=$dsdhostname;
        	write_config();
	}
$lastLogFile='/var/log/system.log';
exec("cat /var/log/system.log", $logarr);
$lastLogtime=$config['dsdlastlogactivity']['dsdsyslogindex'];
$logent = preg_split("/\s+/", $logarr[(int) $lastLogtime], 6);
$entry_date_time = htmlspecialchars(join(" ", array_slice($logent, 0, 3)));
       $logDateTime=new DateTime($entry_date_time);
       $entry_text = ($logent[3] ==  $config['system']['hostname']) ? "" : $logent[3] . " ";
       $entry_text .= htmlspecialchars($logent[4] . " " . $logent[5]);
	$ipaddr=publicIP();
       $arr=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>$entry_text,'booting_last_log'=>'This was the last log of the DSD');
       $data= json_encode($arr);
       $url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
	if(!eventsonHttp("$url",$data))
		return 0;
	if(!diskCheckError());
		return 0;
	return 1;
}
}




/*  ****************DSD HEARTBEAT ***************************/
function dsdHeartBeat(){
	global $config;
	if(!isset($config['dsdheartbeatfq']['dsdfreqmin'])){
        $config['dsdheartbeatfq']['dsdfreqmin']='2';
 		write_config();
	}
	if(!isset($config['dsdpubip']['dsdpubipval'])){
        $file='/etc/rc.soc';
        $config['dsdpubip']['dsdpubipval']=file_get_contents($file);
 		write_config();
	}
	if(!isset($config['dsdmachwaddr']['dsdmachwval'])){
        $config['dsdmachwaddr']['dsdmachwval']=uniqid();
        	write_config();
	}
	$ipaddr=publicIp();
	$logDateTime=new DateTime($entry_date_time);
	$arr = array('source_ip_address' => $ipaddr, 'heartbeat' => 'true','tags'=>$config['dsdmachwaddr']['dsdmachwval'],'message'=>uniqid('dsdmsg'),'sendtimestamp'=>$logDateTime->format(DateTime::ISO8601));
	$data=json_encode($arr);
	$url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
	if(eventsonHttp($url,$data,$hrtbt='heartbeat')){
		if($config[dsdresumption][state]=='true'){
		#	resumptionMessage();	
			$config[dsdresumption][state]='false';
			$config[dsdresumption][overdatetime]=date('Y-m-d H:m:s');
		}
		$config[dsdheartbeatfq][dsdhrtbttim]=date('Y-m-d H:m:s');
			write_config();
	}
	else{
		if(!($config[dsdresumption][state]=='true')){
			$config[dsdresumption][state]='true';
			$config[dsdresumption][datetime]=date('Y-m-d H:m:s');
			write_config();
		}
		return 0;			
	}
	return 1;
}

/* ********************* DSD SEND SYSLOG MESSAGES ******************* **/

function sendSyslog(){
 	global $config;
	 $arr=array();
        $lastLogFile='/var/log/system.log';
	exec("cat /var/log/system.log", $logarr);
	if(!isset($config['dsdlastlogactivity']['dsdsyslogindex']))
	{
		(int)$i=0;
		while($i<sizeof($logarr)){
         	       $logent = preg_split("/\s+/", $logarr[(int) $i], 6);
              	 	 $entry_date_time = htmlspecialchars(join(" ", array_slice($logent, 0, 3)));
                	 if(strlen($entry_date_time) > 15){break;}
                	 else{
                  	  $logDateTime=new DateTime($entry_date_time);
               		  $entry_text = ($logent[3] ==  $config['system']['hostname']) ? "" : $logent[3] . " ";
               			 $entry_text .= htmlspecialchars($logent[4] . " " . $logent[5]);
                 	 $arr[]=array('sendtimestamp'=>$logDateTime->format(DateTime::ISO8601),'summary'=>$entry_text);
               		 $i=$i+1;
                 		if(sizeof($arr)>100){echo "Size Limite Exceed"; break;}
		
                	}
		}
	      		$ipaddr=publicIP();
			$logDateTime=new DateTime('NOW');
			$sum=json_encode($arr);
			$arriy=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>$sum,'event_type'=>'event_log');
 			$data= json_encode($arriy);
			print_r($data);
       			 $url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
  			 if(!eventsonHttp("$url",$data))
        		{
               			 echo "Events are not send successfully";
        		}
        		else{
               			 $config['dsdlastlogactivity']['dsdsyslogindex']=sizeof($logarr);
             			  write_config();
        		}
       	}
	else{
		(int)$i=$config['dsdlastlogactivity']['dsdsyslogindex'];	
		while($i<sizeof($logarr)){
		$logent = preg_split("/\s+/", $logarr[(int) $i], 6);
        	$entry_date_time = htmlspecialchars(join(" ", array_slice($logent, 0, 3)));
       		 if(strlen($entry_date_time) > 15){break;}
      		 else{
    		    $logDateTime=new DateTime($entry_date_time);
    		 $entry_text = ($logent[3] ==  $config['system']['hostname']) ? "" : $logent[3] . " ";
       		$entry_text .= htmlspecialchars($logent[4] . " " . $logent[5]);
		$ipaddr=publicIP();
		$logDateTime=new DateTime('NOW');
      		  $arr=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>$entry_text,'event_type'=>'event_log');
        	$data= json_encode($arr);
		echo $data;
		 $url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';	
		if(!eventsonHttp("$url",$data))
 		       {

 	       }
        	else{
                $config['dsdlastlogactivity']['dsdsyslogindex']=sizeof($logarr);
#               write_config();
        	}

	}
	$i=$i+1;
}

}
}






/**********************    To send The resumption Message *******************/
function resumptionMessage(){
	global $config;
	$arr=array();
	$lastLogFile='/var/log/system.log';
exec("cat /var/log/system.log", $logarr);
(int) $i=file_get_contents('/etc/rc.lastlog');
#(int)$i=$config['dsdlastlogactivity']['dsdsyslogindex'];
while($i<sizeof($logarr)){
$logent = preg_split("/\s+/", $logarr[(int) $i], 6);
 	$entry_date_time = htmlspecialchars(join(" ", array_slice($logent, 0, 3)));
        if(strlen($entry_date_time) > 15){break;}
       else{
        $logDateTime=new DateTime($entry_date_time); 
     $entry_text = ($logent[3] ==  $config['system']['hostname']) ? "" : $logent[3] . " ";
       $entry_text .= htmlspecialchars($logent[4] . " " . $logent[5]);
	$arr[]=array('sendtimestamp'=>$logDateTime->format(DateTime::ISO8601),'summary'=>$entry_text);
	$i=$i+1;
	if(sizeof($arr)>100){echo "Size Limite Exceed"; break;}
}
}
 $ipaddr=publicIP();
$logDateTime=new DateTime('NOW');
$sum=json_encode($arr);
$arriy=array('source_ip_address' => $ipaddr,'sendtimestamp'=> $logDateTime->format(DateTime::ISO8601), 'tags'=>$config['dsdmachwaddr']['dsdmachwval'],'summary'=>$sum,'event_type'=>'resumption');
 $data= json_encode($arriy);
print_r($data);
	$url='https://'. $config['dsdpubip']['dsdpubipval'].':8080/events/';
   if(!eventsonHttp("$url",$data))
	{
		echo "Resumption is not send successfully";
	}
	else{
		$config['dsdlastlogactivity']['dsdsyslogindex']=sizeof($logarr);
		write_config();
		return 1;
	}
return 0;
}
?>
