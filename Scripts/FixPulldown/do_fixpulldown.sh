#!/bin/sh

#   Shell script for running bandcor_warm on a set of files
#   change the path to the executable in the two statements below
#
#   Call the script with the threshold and file list as arguments, e.g.:
#
#   do_fixpulldown.sh 150 I1*_bcd.fits
# or 
#   do_fixpulldown.sh 150 @filelist
#
#
# Argument one specifies a threshold; only columns with pixels 
# having values higher than the threshold will be treated with
# the column pulldown correction.
# 
# The following values have been tested and seem to work well:
# 
#    Frame Time  12:  150.0
#    Frame Time  30:   60.0
#    Frame time 100:   20.0
#    Frame time 200:   10.0
#
# Argument two should be list of files, specified using
# optional wildcards (e.g., test.fits, or test*.fits)
# or with an @ at the beginning to specify a list of files 
# (e.g., @filelist where filelist is a text list containing file names
# There should be one file name per line

if [ $# -gt 1 ]
then

if expr match $2 "@" > -1   # test for a file containing a list of files
then 
list=${2#"@"}
for img in `cat $list`; do
 /Users/jrrigby1/SCIENCE/Lensed-LBGs/Spitzer_2020/Scripts/FixPulldown/bandcor_warm -f -t $1 -c -b 1 256 1 256 $img
done

else 
cutoff=$1
shift
while [ $# -gt 0 ]
do 
 /Users/jrrigby1/SCIENCE/Lensed-LBGs/Spitzer_2020/Scripts/FixPulldown/bandcor_warm -f -t $cutoff -c -b 1 256 1 256 $1
 shift
done

fi

else

echo ""
echo "The script do_fixpulldown.sh calls the program \"bandcor_warm\""
echo "which attempts to fix the column pulldown artifact for IRAC warm"
echo "mission data, channels 1 and 2.  The executable program must be"
echo "located in the same directory, or edit the script to point to the"
echo "right path.  "
echo
echo "Call the script with the threshold and file list as arguments, e.g.:"
echo
echo "do_fixpulldown.sh 150 I1*_bcd.fits"
echo " or" 
echo "do_fixpulldown.sh 150 @filelist"
echo 
echo "Argument one specifies a threshold; only columns with pixels "
echo "having values higher than the threshold will be treated with "
echo "the column pulldown correction."
echo  
echo "The following values have been tested and seem to work well:"
echo  
echo "    Frame Time  12:  150.0"
echo "    Frame Time  30:   60.0"
echo "    Frame time 100:   20.0"
echo "    Frame time 200:   10.0"
echo 
echo "Argument two should be list of files, specified using "
echo "optional wildcards (e.g., test.fits, or test*.fits)"
echo "or with an @ at the beginning to specify a list of files" 
echo "(e.g., @filelist) where filelist is a text list containing file"
echo "names. There should be one file name per line."
echo
echo

fi
