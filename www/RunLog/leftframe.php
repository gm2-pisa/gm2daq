<?php

include_once("config.php");
include_once("tools.php");

print_header("run list");

try
{
  $dbh = new PDO('mysql:host='.DB_HOST
		 .';port='.DB_PORT
		 .';dbname='.DB_NAME,
		 DB_USER,
		 DB_PASSWORD,
		 array(PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES ".DB_CHARSET,
		       PDO::ATTR_PERSISTENT => false
		       ) 
		 );
}
catch (PDOException $e)
{
  echo 'Connection to database failed: ' . $e->getMessage();
  echo mysql_error();
  die("Error opening database");
}
// count runs
$nruns=0;
$run_min=0;
$run_max=0;
$date_min=0;
$date_max=0;
$result = $dbh->query("SELECT count(`run`),min(`run`),max(`run`),min(`starttime`),max(`starttime`) FROM `".DB_TABLE."` WHERE `run` IS NOT NULL") or die(mysql_error());
foreach ( $result as $row )
  {
    $nruns=$row[0];
    $run_min=$row[1];
    $run_max=$row[2];
    $date_min=$row[3];
    $date_max=$row[4];
    break;
  }
$result->CloseCursor();    

//echo $nruns." ".$run_min." ".$run_max." <br>";
//echo $date_min." - ".$date_max." <br>";


// make the list of runs
$run_step=100;
$query="SELECT min(`run`),max(`run`) FROM `".DB_TABLE."` WHERE `run` IS NOT NULL GROUP BY (`run` DIV ".$run_step.") LIMIT $run_step";
$result = $dbh->query($query) or die(mysql_error());
foreach ( $result as $row )
  {
    $run_1=$row[0];
    $run_2=$row[1];
    echo '<a href="mainframe.php?run_min='.$run_1.'&run_max='.$run_2.'" target="mainframe">'.$run_1."-".$run_2."</a><br>";
  }
$result->CloseCursor();

?>