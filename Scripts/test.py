from re import sub
from sys import argv
from os import remove
from ccdproc import ImageFileCollection
from ccdproc import Combiner, combine
from astropy.nddata import CCDData
from astropy.stats import sigma_clip
import pyds9

ch = 'ch1'
aor = 'r15102976'
# Proof of concept, to replace delta_bias.pl
thisdir = '../../../Downloaded' + '/' + aor + '/' + ch + '/bcd/'
d = pyds9.DS9('foo1')
ccd = {}  # dictionaries to hold the files
clipped = {}
clipped_ccd = {}
image_collection = ImageFileCollection(thisdir, glob_include='*cbcd.fits')  # All the files in this dir
for thisfile in image_collection.files:    
    # use astropy's sigma clip to mask with grow function.  output of sigma_clip is np masked array.  Convert to CCDData
    ccd[thisfile] = CCDData.read(thisdir + thisfile)
    clipped[thisfile] = sigma_clip(ccd[thisfile], sigma_lower=10, sigma_upper=3, maxiters=None, grow=2)
    clipped_ccd[thisfile] = CCDData(clipped[thisfile].data, mask=clipped[thisfile].mask, unit='MJy / sr')

display_progress = True
if display_progress :
    d.set("file " + thisdir + image_collection.files[0])
    d.set("frame 2")
    d.set_np2arr(clipped[image_collection.files[0]].mask.astype('float64'))   # This isn't a file, but a numpy array

# Median combine with extra sigma clipping, and write to a file
combiner = Combiner(list(clipped_ccd.values()))  
combiner.sigma_clipping(low_thresh=3, high_thresh=3)
combined_image = combiner.median_combine()
outfile = ch + '_' + aor + '_dbias.fits'
combined_image.write(outfile, overwrite=True)
d.set("frame 3")
d.set("file " + outfile)

# Now, subtract the dbias.fits file from each individual file, and write to Dbias_ch1_r15102976/ folder
for ccd, fname in image_collection.ccds(return_fname=True) :
    subtracted = ccd.subtract(combined_image)
    newname=sub('_cbcd.fits', '_bcbcd.fits', fname)
    print("oldname newname", fname, newname)
    subtracted.write(newname, overwrite=True)
print("Subtracted delta bias from each cbcd frame, to make bcdbd frames.")


print("All done delta bias python script.")
