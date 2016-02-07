<?php
/**
 * @file    runlog_field_update.php
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Wed Feb  8 10:57:42 2012
 * @date    Last-Updated: Wed Feb  8 12:59:18 2012 (-0500)
 *          By : g2
 *          Update #: 28 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @addtogroup inprogress InProgress 
 *  - \ref runlog_field_update.php
 * 
 * @page   runlog_field_update.php
 * 
 * @brief   
 * 
 * @details 
 * 
 * @todo Document this code 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */
  
include_once("config.php");

$run      = "";
$field    = "";
$val      = "";

if ( array_key_exists( 'run',      $_GET ) ) $run       = $_GET['run'];
if ( array_key_exists( 'field',    $_GET ) ) $field     = $_GET['field'];
if ( array_key_exists( 'val',      $_GET ) ) $val       = $_GET['val'];

if ( $run == "" || $field == "" ) die("Bad input parameters");

// connect to mysql server
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
  echo 'Connection to database failed: ' . $e->getMessage()."<br>";
  echo "MySQL error: [".mysql_error()."]<br>";
  die("Error opening database");
}

$isOk=TRUE;
// update database
$query="UPDATE `".DB_TABLE."` SET `".$field."`='".$val."' WHERE `run`='".$run."'";
$result = $dbh->query($query);
if ( $result === FALSE )
  $isOk = FALSE;

// read back from database
if ( $isOk )
  {
    $readback='';
    $query="SELECT `".$field."` FROM `".DB_TABLE."` WHERE `run`='".$run."'";
    $result = $dbh->query($query);
    if ( $result === FALSE )
      $isOk = FALSE;
    else
      {
	foreach ( $result as $row )
	  {
	    $readback=$row[0];
	    break;
	  }
	$result->CloseCursor();
	if ( $readback != $val ) $isOk=FALSE;
      }
  }

header("Content-type: image/png");

if ( $isOk )
  readfile("runlog.png");
else
  readfile("error.png");

?>