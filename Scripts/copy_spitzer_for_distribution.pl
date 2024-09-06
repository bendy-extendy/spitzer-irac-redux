#!/usr/bin/perl
# Need to copy the final Spitzer mosaics out of the reduction directories, so that 
# I can upload them to vetinari.  Also give them more descriptive filenames (rather than relying on dir).
# jrigby, 8/2014

$root_out_dir = "Packed_up";

system("mkdir $root_out_dir");

@dir = `ls -d */Ch?/`; 
foreach(@dir)
  {
    chomp;
    $thisdir = $_;
    $_ =~ s/(\w+\S+)\/(Ch\S)//;
    $cluster = $1;
    $channel = $2;
    system("mkdir $root_out_dir/$cluster");
    system("mkdir $root_out_dir/$cluster/$channel");

    $in_dir  = $thisdir . "Combine-mosaic/";
    $out_dir = $root_out_dir . "/" . $thisdir ; 

    # Give the spitzer irac mosaics more descriptive filenames, sdssxxx-xx_irac_Ch1_mosaic.fits
    @file = `ls $in_dir*fits`;
    foreach(@file)
      {
	chomp;
	$old_filename = $_;
	$_ =~ s/(\S+)\/(\S+)\/Combine-mosaic\/(\S+)// ;
	$filename = $3;
	$new_filename = $cluster . "_irac_$channel" . "_" . $filename;
	print "Want to rename $old_filename to $new_filename\n";
	system("cp -pr $old_filename $out_dir$new_filename");
      }

  }
