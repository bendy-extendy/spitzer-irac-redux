#!/usr/bin/perl

$outfile = "run_all_pulldown.sh";
open(PD, ">$outfile") or die "cannot create outfile $outfile\n";

@dir = `ls -d Downloaded/r*/ch?/bcd`;
chomp(@dir);
foreach(@dir)
  {
    print PD "cd $_\n";
    print PD "../../../../Scripts/FixPulldown/do_fixpulldown.sh 60.0 *cbcd.fits\n";
    print PD "echo Finished in $_\n";
    print PD "cd ../../../../\n";
  }
print "Made $outfile.  Run it to do the pulldown correction on the ORIGINAL CBCDs\n";


# Should make ~4 files of equal length, to speed this up
