# No redirects, so safe to do cl < delta_bias.cl
!/Users/jrrigby1/WORK/Lensing/Spitzer/Scripts/setup_Spitzer_bias.pl ch1_cbcd_r48118784.list
#
print "STATUS: deleting intermediate files leftover from previous runs of this script"
del r48118784_bias.fits  verify=no allvers=yes subfile=yes go_ahead=yes
del @ch1_cbcd_r48118784_bcbcd.list verify=no allvers=yes subfile=yes go_ahead=yes
del @ch1_cbcd_r48118784_mask.list verify=no  mode=h 
!mv -f imcomb.log imcomb.old
!/Users/jrrigby1/WORK/Lensing/Spitzer/Scripts/delete_maskname_in_header.pl ch1_cbcd_r48118784.list
#
print "STATUS: Making an object mask for each image"
nproto
objmasks @ch1_cbcd_r48118784.list  @ch1_cbcd_r48118784_mask.list omtype=boolean mask="" hsigma=3 lsigma=10 hdetect=yes ldetect=no neighbo=8 minpix=6 ngrow=2 agrow=2
# This task adds a header keyword objmask to each cbcd.fits file, pointing to the mask
#
print "STATUS:  Median combining the cbcd images to make a delta bias frame."
immatch
unlearn imcomb
imcomb input=@ch1_cbcd_r48118784.list output=r48118784_bias.fits logfile=imcomb.log combine=median reject=sigclip mclip=yes lsigma=3 hsigma=3 masktyp="!OBJMASK goodvalue" maskval=0 scale=none zero=mode weight=none statsec="[17:247,10:240]"
# the zero=mode here is important to deal with the even-odd frame
!cat imcomb.log
#
print "STATUS:  Subtracting delta bias from each cbcd image.  Outputs are *bcbcd.fits"
imutil
imarith  @ch1_cbcd_r48118784.list - r48118784_bias.fits @ch1_cbcd_r48118784_bcbcd.list
