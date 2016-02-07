<?php

function print_header($title)
{
  $header=''
    .'<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">'.PHP_EOL
    .'<html>'.PHP_EOL
    .'<head>'.PHP_EOL
    .'<title>'.$title.'</title>'.PHP_EOL
    .'<meta HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=utf-8">'
    .'<link rel="shortcut icon" href="favicon.ico" type="image/x-icon">'
    ;
  echo $header;
}

?>