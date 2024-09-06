#!/usr/bin/perl

$replace_FIF = 0;  # Replace the FIF table (size/WCS of output mosaic) with a first-guess?
$topdir = "/Users/jrrigby1/SCIENCE/Lensed-LBGs/Spitzer_1153p0755/";
$template_dir = $topdir . "Scripts/";
$mopex_bin = "/Users/jrrigby1/Applications/mopex/bin";
$gethead = "/Users/jrrigby1/Applications/wcstools-3.9.6/bin/gethead";

$dothis = $gethead ." -t object aorkey CHNLNUM crval1 crval2 Downloaded/r*/ch*/bcd/*0005*cbcd.fits";
@infiles = `$dothis`;

foreach(@infiles)
  {
    ($file, $object, $aor, $channel, $ra, $dec)=split(/\t/, $_);
    print "DEBUGGING  ", $object, "  ", $aor, "\n";

    if ($object =~ /PSZ/) {print "Ah, the Planck Arc.\n"}
    elsif ($object =~ /RCS0310_5_2007/) {$object = "RCS0327-13";}  # Rename
    elsif ($object =~ /0224/)  {$object = "RCS022433-0002"; print "MATCHED!"}  # Rename
    elsif ($object =~ /52909/)  {$object = "COOLAMPS_1153p0755"; print "MATCHED!"}  # Rename
    else
      {	
	if ($object =~ /sdssj0915/) {$object = "SDSSJ0915+3826";}  # Rename
	if ($object =~ /sdssj1343/) {$object = "SDSSJ1343+4155";}  # Rename   
	$object =~ s/-1-1\Z//;
	$object =~ s/-1\Z//;   # strip off "-1" at end of object name, weird artifact of AOR design.
	print "Object named $object, ";
	$object =~ s/(SDSSJ\S+[+-]\S\S)(\S?\S?)//;  # Truncate RA to sign and two characters
	$object = $1;
      }
    print "truncated to $object\n";
    $ra_short  = sprintf("%.4f", $ra);
    $dec_short = sprintf("%.4f", $dec);
    $outdir   = $object . '/Ch' . $channel;    
    $nldir = $object . '/Ch' . $channel . '/cdf/';    # dir where mopex looks for namelists
    $outfile = "ch" . $channel . "_r" . $aor . "_cbcd.list";

    system("mkdir -v $object");  # level1
    system("mkdir -v $outdir");  # level2
    system("mkdir -v $nldir");# level3, dir for mopex scripts
    
    system("ls Downloaded/r$aor/ch$channel/bcd/*cbcd.fits > temp");
    $command = 'sed "s,Downloaded,../../Downloaded,g" < temp > temp2';  #fix the path
    $foo = `$command`;
    system("grep -v 0000_0000 temp2 > $outdir/$outfile");  # ignore weird first frame.
    
    # Make namelists to run Mopex in batch mode
    @template = qw(crop-frames-mosaic.nl overlap.nl mosaic.nl);
    foreach(@template)
      {
	$fullname = $template_dir . $_;
	$nl_outfile = $nldir . $_;
	system("sed \"s,THE_CLUSTER_DIR,$topdir$outdir,g\" < $fullname > $nl_outfile");
	print "Created mopex namelist  $nl_outfile\n";
      }


    $fif_out = $outdir . "/jrr_mosaic_fif.tbl";
    if((! -e $fif_out) or $replace_FIF )   # if no FIF (or forced),  make a new FIF file and add coords
      {
	if(1 == $channel)
	  {
	    $fif_in  = $template_dir . "jrr_mosaic_fif.tbl";

	    system("sed \"s,RA_GOES_HERE,$ra_short,g\" < $fif_in > temp");
	    system("sed \"s,DEC_GOES_HERE,$dec_short,g\" < temp > $fif_out");
	  }
	if(2 == $channel)     # Force ch2 to use the ch1 FIF, so same WCS
	  {
	    system("unlink $outdir" ."/jrr_mosaic_fif.tbl"); 
	    $fif_in   =  "../Ch1/jrr_mosaic_fif.tbl";
	    system("ln -s $fif_in $outdir");
	  }
      }
  }

print "\n\n**************  ADVISORY **************\n";
if(1 == $replace_FIF)
  {
    print "You will want to adjust the first-guess for central coordinates in each FIF file, e.g. $fif_out\n";
    print "This script will patiently wait for you to do that.\n";
    print "When you are done, hit Ctrl-D,";
    print "and I will make the scripts to run all the delta_bias and mopex mosaics\n";
    while(<>){print "$_"; }
  }


# Make scripts to, in batch mode, run all the delta bias, and all the mopex mosaics
$dbias_script = "run_all_dbias.sh";
$mopex_script = "run_all_mopex.sh";
open(DB, ">$dbias_script") or die "cannot create outfile $dbias_script\n";
open(MX, ">$mopex_script") or die "cannot create outfile $mopex_script\n";
#@cbcd = `ls */Ch?/all*_bcbcd.list`;
@cbcd = `ls */Ch?/*_cbcd.list`;
print DB "ur_setup\n";
foreach(@cbcd)
  {
    $_ =~ s/(\S+)(ch\S+)//;
    print "I grabbed $1 and $2\n";
    print DB "cd $1\n";
    print DB "../../Scripts/Spitzer_dbias.pl   $2\n";
    print DB "cd ../../\n";

    print MX "cd $1\n";
    print MX "$mopex_bin/mosaic.pl  -n crop-frames-mosaic.nl\n";
    print MX "$mopex_bin/overlap.pl -n overlap.nl\n";
    print MX "$mopex_bin/mosaic.pl  -n mosaic.nl\n";
    print MX "cd ../../\n";
  }
close(DB);  close(MX);


