#!/usr/bin/perl
#use POSIX;
#use PGPLOT;
#require '/Volumes/Apps_and_Docs/jrrigby1/Lib/jrr.pl';
#require '/Volumes/Apps_and_Docs/jrrigby1/Utils/astroconst/astroconst.pl';

$inlist = "changed.list";

open(IN, "<$inlist") or die "cannot open infile $inlist\n";
while(<IN>)
  {
    chomp;
    @file = split;
    system("xpaset -p ds9 frame 1");
    system("xpaset -p ds9 file $file[0]");
    system("xpaset -p ds9 zoom to fit");
    system("xpaset -p ds9 frame 2");
    system("xpaset -p ds9 file $file[1]");
    system("xpaset -p ds9 zoom to fit");
    system("xpaset -p ds9 frame 1");
    print "Press any key to continue\n";
    $foo = <>; 
  }
