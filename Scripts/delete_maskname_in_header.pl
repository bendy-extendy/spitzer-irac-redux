#!/usr/bin/perl
$delhead = "/Users/jrrigby1/anaconda3/pkgs/wcstools-3.9.5-1/bin/delhead -v";   # where wcstools delhead lives

$argsize=@ARGV;
if($argsize != 1)  {die "ERROR:  Input is cbcd files to remove header keyword BPM (bad pixel mask).\n";}

$infile = $ARGV[0];

open(IN, "<$infile") or die "cannot open infile $infile\n";
while(<IN>)
  {
    next if (/\#/);  #skip comments
    $_ =~s/^\s+//;   #remove leading whitespace
    chomp;
    $mask = $_;
    $mask =~ s/cbcd/mask/;
    $command = "$delhead $_  BPM";
    $foo = `$command`;
  }

