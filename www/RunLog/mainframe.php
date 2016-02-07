<?php
/**
 * @file    mainframe.php
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Wed Feb  8 10:58:27 2012
 * @date    Last-Updated: Thu Jul 17 17:32:14 2014 (-0500)
 *          By : Data Acquisition
 *          Update #: 26 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @addtogroup inprogress InProgress 
 *  - \ref mainframe.php
 * 
 * @page   mainframe.php
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
include_once("tools.php");

$run_min = 0;
$run_max = 0;
$database = "online";

if ( array_key_exists( 'run_min', $_GET ) ) $run_min  = $_GET['run_min'];
if ( array_key_exists( 'run_max', $_GET ) ) $run_max  = $_GET['run_max'];
if ( array_key_exists( 'db',      $_GET ) ) $database = $_GET['db'];

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


// by default select last 20 runs
if ( $run_max == 0 )
  {
    $result = $dbh->query('SELECT max(`run`) FROM `'.DB_TABLE.'` WHERE `run` IS NOT NULL') or die(mysql_error());
    foreach ( $result as $row )
      {
	$run_max=$row[0];
	$run_min=$run_max-20;
	if ( $run_min < 0 ) $run_min = 0;
      }
    $result->CloseCursor();        
  }

print_header(EXP_NAME);

?>

<script type="text/javascript"> 
  
function field_edit(field, run, val_old) 
{ 

  var val = window.prompt("Modify field " + field + " for run " + run, val_old); 
 
  if ( val != null) {
    document.images["runlogeditor"].src = "runlog_field_update.php?field=" + field + "&run=" + run + "&val=" + val;
    //alert(document.images["hiddenimage"].src);
    elem = document.getElementById(field + "_" + run);
    if ( elem ) {
      elem.innerHTML = "<span style='color: red; font-weight: bold; background-color: yellow;'>"+val+"</span>";
    }

  }

  return false; 
} 
 
</script> 


</head>

<center>
<img name="runlogeditor" src="runlog.png">
</center>

<table border="1" valign="middle" rules="all" frame="border" bgcolor="#FFFFFF">
  <tr bgcolor="#A0FFA0">
<?php

// get the list of columns
$query = 'DESCRIBE `'.DB_TABLE.'`';
$result = $dbh->query($query) or die(mysql_error());
  
$columns = array();
$id_run=0;
$i=0;
foreach ( $result as $row )
  {
    echo "    <th>".$row[0]."</th>\n";
    $columns[] = $row[0];
    if ( "$row[0]" == "run" ) $id_run = $i;
    $i++;
  }
$result->CloseCursor();    

echo"</tr>\n";

$result = $dbh->query('SELECT * FROM `'.DB_TABLE.'` WHERE `run`>='.$run_min.' AND `run`<='.$run_max.';') or die(mysql_error());

foreach ( $result as $row )
  {
    // new table row
    echo "<tr align='center'>";

    $run=$row['run'];
    
    // populate cells in the row
    foreach ( $columns as $colname )
      {
	// cell content
	$x = $row[$colname];
	if ( $x == "" ) $x = "-";
	$x_html = htmlspecialchars($x);

	// cell background
	$bg = "white";
	if ( $colname == "quality" )
	  {
	    if ( $x == "Y" ) {
	      $bg = "lightgreen";
	    } else if ( $x == "S" ) {
	      $bg = "lightblue";
	    } else if ( $x == "C" ) {
	      $bg = "RGB(200,200,0)";
	    } else if ( $x == "N" ) {
	      $bg = "#A0A0A0";
	    }
	  }
	
	
	if ( $colname == 'Run number' )
	  // we cannot change the run number
	  echo "<td>".$x."</td>";
	else
	  // make other cells editable
	  echo "    <td style='background: $bg;'><a href='#' onclick='field_edit(\"$colname\",\"$run\",\"$x\");return false;' style='text-decoration: none; font-size: 10px;' id='".$colname."_".$run."'>".$x_html."</a></td>";
      }
    echo "</tr>\n";
    
  }
$result->CloseCursor();    

?>




</table>

<br>

</body>
</html>

