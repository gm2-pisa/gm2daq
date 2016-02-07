<?php 
/**
 * @file    config.php
 * @author  Vladimir Tishchenko <g2@electron>
 * @date    Wed Feb  8 10:59:03 2012
 * @date    Last-Updated: Thu Jul 17 17:58:21 2014 (-0500)
 *          By : Data Acquisition
 *          Update #: 16 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @addtogroup inprogress InProgress 
 *  - \ref config.php
 * 
 * @page   config.php
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


define('EXP_NAME','SLAC');     // name of the experiment

define('DB_HOST', 'localhost');     // mysql host name
define('DB_PORT', '3306');          // database port number
define('DB_USER', 'daq');           // Your MySQL username
define('DB_PASSWORD', 'daq') ;      // ...and password
define('DB_NAME', 'SLAC');     // The name of the database
define('DB_TABLE','Runlog');        // The name of the Runlog table
define('DB_CHARSET', 'utf8');       // Don't change this


?>