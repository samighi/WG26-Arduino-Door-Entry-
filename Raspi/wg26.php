<?php

$logfilename = "wg26pgm.log";


function logger($logEvent,$card,$desc)
{
global $logfilename;
$today = date("Ymd-H:i:s");  


  $newLog = "$today|$logEvent|$card|$desc|";
  $allLog = file_get_contents($logfilename);
  $newLog .= hash("md5",$allLog) . "\n";
  
  file_put_contents($logfilename,$newLog,FILE_APPEND);
  
}


function logger2($logEvent)
{
global $logfilename;
$today = date("Ymd-H:i:s");  


  // $newLog = "$today|$logEvent";
  $allLog = file_get_contents($logfilename);
  $newLog = "$today|" . hash("md5",$allLog) ."|$logEvent\n";
  
  file_put_contents($logfilename,$newLog,FILE_APPEND);
  echo $logEvent;
  
}



//
// main 
//
// $device = '/dev/ttyUSB0';
// $write = 'w+';
// sleep(3);
// 
// $handle = fopen( $device, $write ); # Write +

include "php_serial.class.php";

$serial = new phpSerial;
#$serial->deviceSet("/dev/ttyACM0");
$serial->deviceSet("/dev/ttyUSB0");
$serial->confBaudRate("9600");
$serial->confParity("none");
$serial->confCharacterLength(8);
$serial->confFlowControl("none");
$serial->confStopBits(2);
$serial->deviceOpen();


sleep(3);


// Or to read from


$strAllLog = "";

while (true)
{
 // $strLog = fgets( $handle ); # Read data from device
 
$strLog = $serial->readPort();
//echo $strLog;

if ($strLog != "") 
{
 $strAllLog .= $strLog;
 sleep(5);
 continue;
}
 
  if ($strAllLog != "") { 
	  $arrLog = explode("\n",$strAllLog);
	  print_r($arrLog);
	  foreach ( $arrLog as $log) 
	  { 
	      logger2($log);
	  }
    }
    
$strAllLog = "";
}

 $serial->deviceClose();

// fclose ($handle); # Close device file

?>
