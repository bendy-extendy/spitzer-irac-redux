#!/usr/bin/perl
# Spitzer_dbias.pl
# Purpose is to create the residual bias (delta dark) frame from 
# IRAC Spitzer images, and subtract it off.  Creates bias-subtracted bcbcd images.
# This perl script does the file management, calls a python script to do the image math.
# jrigby 8/2014.  Modifed 1/2021 to work on new laptop gs66-rustin

# Change paths as needed.  These are set for jrigby, gs66-rustin
$sethead = "/Users/jrrigby1/Applications/wcstools-3.9.6/bin/sethead";  # Where WCStools sethead lives
$scripts_dir =  "/Users/jrrigby1/SCIENCE/Lensed-LBGs/Spitzer_1153p0755/Scripts/";

$argsize=@ARGV;
if($argsize != 1)  {die "ERROR:  Input is list of cbcd.fits Spitzer files to perform delta bias correction.  Filename format:  ch1_r48118784_cbcd.list\n";}
$infile = $ARGV[0];
print "DEBUGGING infile is ", $infile, "\n";

# Make the input lists of files needed by iraf
$foo = $infile;
@split = split('_', $foo);
$ch  = $split[0];
$aor = $split[1];
print "Running Spitzer_dbias.pl on " . $ch  . " " . $aor . "\n";

$cbuncfile = $ch . '_' . $aor . "_cbunc.list";  
$maskfile  = $ch . '_' . $aor . "_mask.list";
$bcbcdfile = $ch . '_' . $aor . "_bcbcd.list";  #the extra b in bcbcd means the delta bias has been subtracted 

$tempdir = "Dbias_" . $ch . '_' . $aor . "/";  # temp dir for the masks and bias-corrected frames.
system("mkdir -v $tempdir");

system("sed s/cbcd/cbunc/g < $infile >$cbuncfile");  # this one is trivial to make
open(MASK, ">$maskfile") or die "Cannot create maskfile $maskfile\n";
open(BC, ">$bcbcdfile") or die "Cannot create bcfile $bcbcdfile\n";

open(IN, "<$infile") or die "cannot read infile $infile\n";
while(<IN>)
  {
    next if (/\#/);  #skip comments
    $_ =~s/^\s+//;   #remove leading whitespace
    chomp;
    $_ =~s/(\S+)bcd\/(SPITZER\S+)_cbcd.fits//;
    $bcbcd  = $tempdir . $2 . "_bcbcd.fits";
    $mask   = $tempdir . $2 . "_mask.fits";
    print MASK "$mask\n";
    print BC   "$bcbcd\n";
  }

print "Spitzer_dbias.pl has created lists of masks (not used) and temporary bias-corrected bcbcd.\n";
close(MASK);  close(BC);  close(IN);

# temporarily commenting this out to debug perl

#print "Do the image math in python as    [scripts_dir] / delta_bias.py AOR ch?\n";
$thiscommand = $scripts_dir . "delta_bias.py " . $aor . " " . $ch;
print "  command to call python: " . $thiscommand . "\n";
system($thiscommand);  # NEW PYTHON stage to remove iraf  Scripts/delta_bias.py AOR ch1

print "Delta-bias-corrected files are $tempdir*bcbcd.fits\n";
print "Cleaning up.\n";
system("mkdir -v Junk");
system("mv *.par Junk");
print "Once you have mosaicked them w Mopex, you can delete that dir.\n";

print "Made some all*list lists of all the bcbcd cbunc files, to feed Mopex\n";
system("cat  ch?_r*bcbcd.list  > all_bcbcd.list");
system("cat  ch?_r*cbunc.list  > all_cbunc.list");
system("wc all*list");

