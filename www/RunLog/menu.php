<?php

include_once("config/config.php");

//$menufile="../../config/trend_plots.cfg";

$database = $_GET['database'];

include_once("header.html");


function make_db_list( $mysql_dbs, $database ) 
{

  // ==================================================
  //       *** build list of databases ***
  // ==================================================

  for ($i=0; $i<count($mysql_dbs); $i++) {
    $db = $mysql_dbs[$i];
    if ( "$database" == "$db" ) 
      $selected = "selected";
    else
      $selected = "";
    echo "       <OPTION label='$db' value='$db' $selected>$db</OPTION>\n";    
  }


}



function make_run_list( $database ) 
{
  
  include("config/config.php");

  $db_link = mysql_connect($mysql_host, $mysql_user, $mysql_password);
  if (!$db_link ) {
    die('Could not connect to mysql: ' . mysql_error());
  }
  
  mysql_select_db($database);

  $result = mysql_query("SELECT MIN(run),MAX(run) FROM ".DB_TABLE." WHERE run>60000;");

  $row = mysql_fetch_row($result);
  
  $run_min = (int)$row[0];
  $run_max = (int)$row[1];
  $nruns=$run_max-$run_min;

  $ngroups = (int)($nruns / 100)+3;
  
  for ( $i=0; $i<$ngroups; $i++) {
    $r1 = (int)($run_min / 100);
    $r1 *= 100;
    $r1 += $i*100;
    $r2 = $r1 + 100;
    echo "<INPUT type='radio' name='menuitem' disabled></INPUT><a href='runlog_view.php?run_min=".$r1."&run_max=".$r2."&db=".$database."' target='mainframe' onclick='selectMe($i);'>".$r1."-".$r2."</a><br>";
  }

  mysql_free_result($result); 
  mysql_close($db_link);

}






?> 

<script type="text/javascript" language="JavaScript">

function selectMe(id) {  
  
  document.mainmenu.menuitem[id].checked=true;
  /*
  //parent.frames[1].location.reload(doc);
  var run_nr = document.mainmenu.run_nr.value;
  var run_nr_manual = document.mainmenu.run_nr_manual.value;
  if ( run_nr_manual > 0) 
     run_nr = run_nr_manual;  
  parent.frames[1].location.replace(doc+"&run_nr="+run_nr);
  */
  return true;
}

</script>

</head>

<body class="menu">
<a href="../leftframe.php" target="menuframe" class="menu">Main menu</a> <font color="#FF0000">-&gt;</font>
<hr>

<form name="mainmenu" method="get">
     DB: <SELECT name="database" id='database' title='Database' onchange="ReloadMenu();">
     <OPTGROUP label="Databases">
END;

<?php make_db_list( $mysql_dbs, $database ); ?>

</OPTGROUP>
</SELECT>
<br />


<?php make_run_list( $database ); ?>

</form>

</body>
</html>

