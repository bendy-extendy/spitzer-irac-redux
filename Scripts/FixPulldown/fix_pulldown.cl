################################################################
#  2009 November 19, M. L. N. Ashby
#  Procedure fix_pulldown.cl
#
#  This is just an IRAF-based driver that calls
#  bandcor_warm repeatedly to fix column pulldown 
#  for warm-mission cbcds.  It operates on all
#  IRAC CBCD frames listed in input parameter filelist.
#
#  2010 Jan 27: Removed redundant input parameter Chan.
#
################################################################
procedure fix_pdown(filelist,lower)
string filelist
real lower
struct *filenames
struct *curfile
begin
	int index,stop,halt,inext
        real x1, x2, y1, y2, threshold
	string answer,s1,s2,s3,bindir
	int imagenum,inputnum,i1,i2,j1,j2,im1, im2, ysize,itmp
        int numimages,refreshi,nfiles
	string exfile,chnum,aornum
        string cname,maskname,inimg,flist,tmpname,tmpname2

inimg = filelist
threshold = lower

 if(access("imgets.log"))
    delete("imgets.log",verify-)
 if(access("list_in1"))         #delete old copy of template f
     delete("list_in1",verify-)
 if(substr(inimg,1,1) == "@") {  #input is a list, not a single image
   i = strlen(inimg)
   filenames = substr(inimg,2,i)
   flist=substr(inimg,2,i)
 }
 else {
   if(access("list_in"))         #delete old copy of template f
     delete("list_in",verify-)
   files(inimg, > "list_in")     #expand file template into a l
   filenames  = "list_in"
        flist="list_in"
 }

        bindir="/home/mashby/iraf/imclean_dist/bin/"
 
        halt=0
        i1=1
        if (fscan(filenames,cname)!=EOF)
           print(cname, >"list_in1")
        while(fscan(filenames,cname)!=EOF) {
           print(cname, >>"list_in1")
           i1=i1+1
        }
        flist="list_in1"
        filenames=flist

        # Set the threshold below which column pulldown is NOT treated
        # For the time being, this has just been empirically determined 
	# to be 150 for 12s frames, and about 20 for 100s frames.
        # print("debug01 ", lower, threshold, cname)

        # These are hardwired here for the 256x256 pixel IRAC cbcd images
        # so the entire image is automatically treated
        i1 = 1
        i2 = 256
        j1 = 1
        j2 = 256
	while(fscan(filenames,cname)!=EOF) {
            s1="!"//bindir//"bandcor_warm -f -t "//threshold//" -c -b "//i1//" "//i2//" "//j1//" "//j2//" "//cname
            # print (s1) 
            print (s1) | cl()

        }
end
