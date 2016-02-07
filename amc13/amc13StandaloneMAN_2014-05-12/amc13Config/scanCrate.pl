#!/usr/bin/perl
#
# find all AMC13s in a crate, report serial number, IP, versions
# requires environment setup so that working AMC13Tool.exe is in the path
# also requires it be run from the 'amc13Config' directory so that
# the python tools mmcVersion.py etc can be found
#
# E. Hazen - 2014-04-08
#

require File::Temp;

# need to find a set of Python tools... default is just current dir
$tool_path = ".";

$lo_slot = 1;
$hi_slot = 13;

# loop over slots
for( $slot=$lo_slot; $slot<=$hi_slot; $slot++) {
    # first get the MMC version by running python tool
    open (RMMC, "$tool_path/mmcVersion.py --slot=$slot 2>&1 |") or die
	"Looking for mmcVersion.py in $tool_path";
    $maj = 0;
    $min = 0;
    while( $line = <RMMC>) {
	chomp $line;
	# yow!  match hex strings (must be a better way)
	if( $line =~ m{^\s*[0-9a-zA-Z][0-9a-zA-Z]\s+[0-9a-zA-Z][0-9a-zA-Z]\s*$}) {
	    ($smaj, $smin) = $line =~ m{^\s*([0-9a-zA-Z][0-9a-zA-Z])\s+([0-9a-zA-Z][0-9a-zA-Z])\s*$};
	    $maj = hex $smaj;
	    $min = hex $smin;
	}
    }
    close RMMC;
    # format MMC version nicely
    $mmcv = sprintf( "%-6s", sprintf("%d.%d", $maj, $min));

    # AMC13 must have at least 2.1
    if( $maj >= 2 && $min >= 1) {
	# now run readIPs
	open (RIPS, "$tool_path/readIPs.py --slot=$slot 2>&1 |") or die
	    "Looking for readIPs.py in $tool_path";
	$ips = "";
	while( $line = <RIPS>) {
	    chomp $line;
	    if( $line =~ m{\[\s*\d+\s*,}) {
		($b1,$b2,$b3,$b4) = $line =~ m{\[\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\]};
		$ips .= " " . sprintf( "%16s", sprintf( "%d.%d.%d.%d", $b1, $b2, $b3, $b4));
	    }
	}
	close RIPS;

	# now try to get the serial number and firmware versions
	# for this we need a temporary script to run AMC13Tool.exe
	$sip = substr $ips, 1, 16;
	($fh, $fname) = File::Temp::tempfile();
	print $fh "rs 0xa\n";
	print $fh "fv\n";
	print $fh "q\n";
	close $fh;
	open( AMC, "AMC13Tool.exe -U -I $sip -X $fname |") or die
	    "Opening AMC13Tool.exe -U -I $sip -X $fname |";
	# set all to 0 in case they aren't found
	$serno = "0x0";
	$vver = "0x0";
	$sver = "0x0";
	while( $line = <AMC>) {
	    chomp $line;
	    if( $line =~ /0000000a: /) {
		($serno) = $line =~ /:\s+(\w+)/;
	    }
	    if( $line =~ /tex Firmware Ver/) {
		($vver) = $line =~ /Version:\s+(\w+)/;
	    }
	    if( $line =~ /tan Firmware Ver/) {
		($sver) = $line =~ /Version:\s+(\w+)/;
	    }
	}	    
	close AMC;
	# finally, output the formatted stuff
	printf "%2d: MMC: %s IP: %s vv: 0x%04x sv: 0x%04x sn: %d\n",
	$slot, $mmcv, $ips, hex($vver), hex($sver), hex($serno);
    } else {
	if( $maj == 0 && $min == 0) {
	    $mmcv = "-none-";
	}
	printf "%2d: MMC: %s\n", $slot, $mmcv;
    }
}

