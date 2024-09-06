# This is a template for the iraf script that will be generated 
# automatically and run by Spitzer_dbias.pl
# Purpose is to create the residual bias (delta dark) frame from 
# IRAC Spitzer images, and subtract it off.  Creates bias-subtracted bcbcd images.
# jrigby 8/2014
#
print "STATUS: deleting intermediate files leftover from previous runs of this script"
del ch1_r48118784_dbias.fits  verify=no allvers=yes subfile=yes go_ahead=yes
del @ch1_r48118784_bcbcd.list verify=no allvers=yes subfile=yes go_ahead=yes
del @ch1_r48118784_mask.list verify=no  mode=h 
!mv -f imcomb.log imcomb.old
!/Volumes/Apps_and_Docs/SCIENCE/Lensed-LBGs/Spitzer/Scripts/delete_maskname_in_header.pl ch1_r48118784_cbcd.list
#
print "STATUS: Making an object mask for each image"
nproto
objmasks @ch1_r48118784_cbcd.list  @ch1_r48118784_mask.list omtype=boolean mask="" hsigma=3 lsigma=10 hdetect=yes ldetect=no neighbo=8 minpix=6 ngrow=2 agrow=2
# This task adds a header keyword objmask to each cbcd.fits file, pointing to the mask
#
print "STATUS:  Median combining the cbcd images to make a delta bias frame."
immatch
imcomb input=@ch1_r48118784_cbcd.list output=ch1_r48118784_dbias.fits logfile=imcomb.log combine=median reject=sigclip mclip=yes lsigma=3 hsigma=3 masktyp="!OBJMASK goodvalue" maskval=0 scale=none zero=mode weight=none statsec="[17:247,10:240]"
# the zero=mode here is important to deal with the even-odd frame
!cat imcomb.log
#
print "STATUS:  Subtracting delta bias from each cbcd image.  Outputs are *bcbcd.fits"
imutil
imarith  @ch1_r48118784_cbcd.list - ch1_r48118784_dbias.fits @ch1_r48118784_bcbcd.list
print "All done delta bias script."
logout
