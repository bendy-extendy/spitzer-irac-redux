# spitzer-irac-redux

This is a walk-through of how to reduce warm Spitzer IRAC data for lensing
galaxy clusters, as described in section 3.2 of Florian et al. 2021 (ApJ, 916, 50).
By Jane Rigby (Jane.Rigby@nasa.gov), 9/2024

The high-level workflow is:
- run a perl script to make directories and set paths
- Apply to individual frames the column pull-down correction "bandcor_warm" by Matt Ashby,
  to mitigate column artifacts from bright sources
- make a delta bias file, to correct for residual bias pattern noise and persistance, and
  apply it to the individual frames.
- Mosaic the individual frames together using the MOPEX command-line tools.
Again, see Florian et al. 2021 for details.


Here's the step-by step instructions.  Commands to enter on the command line are prefaced with >>

0) Download and install Spitzer's MOPEX mosaicking tool.  Get it at 
   https://irsa.ipac.caltech.edu/data/SPITZER/docs/dataanalysistools/tools/mopex/

Note: As of 9/2024, there are not MOPEX binaries available for modern M1/M2/M3 Apple Mac computers.
I am in communication with the IRSA helpdesk about this issue.  I have not tried building from source.
Instead, I just moved everything to a Linux machine, and ran MOPEX there.

When you install MOPEX, pay attention to the instructions under "Starting the command-line MOPEX"
on this webpage:
  https://irsa.ipac.caltech.edu/data/SPITZER/docs/dataanalysistools/tools/mopex/mopexdownload/
Make sure the paths are right.

1) Install (if you don't already have it) WCSTools, by Jessica Mink  http://tdc-www.harvard.edu/wcstools/

2) Download the data from the Spitzer Heritage Archive, and put it in the Downloaded/ dir.
   It will be sorted into AOR directories, where each AOR is an observation request.  You will use the
   "corrected basic calibrated data", or "cbcd" files.

3) cd to the high-level directory, for example: 
>>   cd /Users/jrrigby1/SCIENCE/Lensed-LBGs/My-lensed-galaxy/

4) Edit the paths in these files: Scripts/make_spitzer_redux_dirs.pl and Scripts/Spitzer_dbias.pl
   Especially check the paths to gethead, MOPEX, and the Scripts/

5) Check your python install:
    a) Make the first line of Scripts/delta_bias.py has the path to your python install
    b) Make sure the python modules astropy and ccdproc are installed

6) Spitzer allowed some odd Object names in the header.  You may want to standardize them,
especially if you are combining multiple datasets of the same source.  Do this by editing
Scripts/make_spitzer_redux_dirs.pl

7) Run the script to make the directories:
>>   Scripts/make_spitzer_redux_dirs.pl

8) Set up environment variables for MOPEX, by by running the script that came with MOPEX:
>>   ~/Applications/mopex/mopex-script-env.csh

9)   Create the script to correct for pulldown:
>>    Scripts/prep_pulldown_correction.pl   

10) Actually run the script to correct for pulldown. This directly edits the downloaded *cbcd.fits files
>>   ./run_all_pulldown.sh

11)  Make a delta bias file, and apply it (w Python). You should check that the output .list files make sense
>>   ./run_all_dbias.sh 

Everything above this step works on a modern Mac.  However, since as of 9/2024 there are not MOPEX
binaries for modern OSX machines, teh step below will fail with a segfault when running mosaic_covg.
Therefore, if you are on a mac, go switch to a Linux machine for subsequent steps.

12) If you've just switched to a Linux box, remember to fix all the paths, and also source mopex: 
>> source /Home/eud/jrrigby1/software/mopex/mopex-script-env.csh

13) Actually do the mosaicking with MOPEX
>>   ./run_all_mopex.sh

14) Check the final mosaics in Targetname/Ch[12]/Combine-mosaic/mosaic.fits
  You may not like where the final mosaic is centered.   If not, simply edit the
  jrr_mosaic_fif.tbl file in the Ch?/ directory, and set the desired central RA and DEC as CRVAl1 and CRVAL2.
  You can also change the size or pixel scale of the output mosaic.  In the example scripts,
  the output pixel scale is 0.6", so already subsampled to about half the IRAC native pixel scale.
 
  Then rerun ./run_all_mopex.sh  to regenerate the mosaic.

15) If you use this workflow, please remember to cite Florian et al. 2021.  Thanks! -Jane


