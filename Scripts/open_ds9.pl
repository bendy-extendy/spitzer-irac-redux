#!/usr/bin/perl


@dirs = `ls -d SDSS*`;
foreach(@dirs)
  {
    chomp;
    $thisdir = $_;
    print "DIR $thisdir    ";
    @mosaic = `ls $thisdir/Ch?/Combine-mosaic/mosaic.fits`;
    chomp(@mosaic);
    print "debug, MOSAIC @mosaic\n";
    system("xpaset -p ds9 frame 1");
    system("xpaset -p ds9 file $mosaic[0]");
    system("xpaset -p ds9 frame 2");
    system("xpaset -p ds9 file $mosaic[1]");
    system("xpaset -p ds9 frame 1");
    system("xpaset -p ds9 zoom to fit");
    system("xpaset -p ds9 match frame wcs");

    $fif = $thisdir . "/Ch1/jrr_mosaic_fif.tbl";
    system("xemacs $fif");
#    print "Press any key to continue\n";
#    $foo = <>; 
  }
