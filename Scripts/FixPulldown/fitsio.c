/*=============================================================================
 *
 *  This file contains routines for reading and writing FITS files.  
 *  These routines are used by the mrc2fts program for reading and writing
 *  FITS format files.
 *
 *---------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <malloc/malloc.h>
#include <memory.h>
#include <math.h>
#include <pwd.h>

#include "fiocom.h"   /* include the common definitions for the FITS routines*/
#include "swapbytes.h"

char * build_card();
char * build_card2();
char * build_comment();
void fillout_head();
void printcard();
void  printfitshead();

FILE *fp1;
/*---------------------------------------------------------------------------
 *  init_fitshead - initialize the FITS header from scratch, putting in the
 *                  necessary lines on file size, scaling parameters, and
 *                  bits per pixel.  This information should be in the BUFFER
 *                  structure "bufp" before calling this routine.
 *---------------------------------------------------------------------------*/
char * init_fitshead(bufp)
   struct BUFFER *bufp;
{
   char * cptr;
   char vbuf[70];

   /*  allocate memory for header structure */
   if (bufp->fhead == NULL)
   {
      if (NULL == (bufp->fhead = (struct fhead_t*)
                                 malloc( (u_int)sizeof(struct fhead_t))))
          return NULL;
   }

   bufp->fhead->next = NULL;
   bufp->naxis = 2;
   bufp->size = bufp->bitpix / 8;

   /* Set the number of lines in header */
   bufp->Nheader = 1;
   bufp->Ncurbuf = 1;

   /*  Now build header, putting in each necessary line with a call to
       build_card */
   cptr = (char*)bufp->fhead->buf;
   cptr = build_card( cptr, "SIMPLE", "T", "DATA IS IN FITS FORMAT",bufp);
   sprintf(vbuf,"%d",bufp->bitpix);
   cptr = build_card( cptr, "BITPIX", vbuf,"BITS PER PIXEL(<0:=REAL)",bufp);
   cptr = build_card( cptr, "NAXIS", "2", "NUMBER OF AXES",bufp);
   sprintf(vbuf,"%d",bufp->naxis1);
   cptr = build_card( cptr, "NAXIS1", vbuf,"PIXELS ON 1ST MOST VARYING AXIS",bufp);
   sprintf(vbuf,"%d",bufp->naxis2);
   cptr = build_card( cptr, "NAXIS2", vbuf,"PIXELS ON 2ND MOST VARYING AXIS",bufp);
   sprintf(vbuf,"%18.12e",bufp->bscale);
   cptr = build_card( cptr, "BSCALE", vbuf,"VALUE= DATA*BSCALE+BZERO",bufp);
   sprintf(vbuf,"%18.12e",bufp->bzero);
   cptr = build_card( cptr, "BZERO", vbuf," ",bufp);
   cptr = build_card( cptr, "END"," ","END OF FITS HEADER INFORMATION",bufp);


   /* Now fill in remaining header with the space character 
   fillout_head(cptr,bufp);*/

   return cptr;

}
/*---------------------------------------------------------------------------
 *  fillout_head - fill the empty space in the FITS header with the space
 *                 character
 *---------------------------------------------------------------------------*/
void fillout_head(cptr, bufp)
   char * cptr;
   struct BUFFER * bufp;
{
   int i;
   char outbuf[85];

   memset( &outbuf[0], ' ', 80);
   outbuf[80] = '\0';

   for (i=bufp->Ncurbuf+1; i<37; i++)
   {
      strncpy(cptr, outbuf,80);
      cptr += 80;
   }
}
/*----------------------------------------------------------------------------
 *  readfits - read in FITS files.  The header information is stored in the
 *             BUFFER structure "bufp", and the data is put into the userarray.
 *             Only the necessary parameters are read from the FITS input
 *             header.  If other parameters are to be read from the headers,
 *             the parameters should be added to the BUFFER structure and read
 *             in as the parameters are below.
 *----------------------------------------------------------------------------*/
int readfits(filename,bufp,userarray)
char *filename;
struct BUFFER * bufp;
float userarray[][YPIX];
{
    int             i;
    char            buf[85];
    int             loop,
                    fp,
                    numrec,
                    pixels_per_rec,
                    m,n,
                    bread;
    long            lpar;
    union intbufs   fits_buf;
    struct fhead_t *last_hdr;
    double          fpar;

    /*  open input file */
    if ((fp1 = fopen(filename,"r")) == NULL)
    {
       printf("rf: Error - file not found ( %s )\n",filename);
       return 1;
    }
    fp = fileno(fp1);

    /*  Allocate memory for FITS header structure  */
    if (NULL ==(bufp->fhead = (struct fhead_t*) malloc((u_int) sizeof(struct fhead_t))))
       goto Merror;

    bufp->fhead->next = NULL;

    /*  Read in first FITS header block */
    bread =  read(fp, (char*)bufp->fhead->buf, FITS_REC_LEN);

    if (FITS_REC_LEN!=bread)
       goto Ferror;

/*  Now search the header block for each of the parameters that must be
    there.  If one is not found, go to the error handler, otherwise read
    in the parameter and store it in the appropriate place in the header
    buffer.                                                             */
    if (-1 == (src_fhead(bufp->fhead, "SIMPLE", buf, sizeof(buf))))
       goto FTSerror;

    if (-1 == (src_fhead(bufp->fhead, "NAXIS", buf, sizeof(buf))))
       goto FTSerror;
    lpar = atol(buf);
    if (lpar != 2)
       goto FTSerror;
    bufp->naxis = (short)lpar;

    if (-1 == (src_fhead(bufp->fhead, "NAXIS1", buf, sizeof(buf))))
      goto FTSerror;
    lpar = atol(buf);
    if (lpar<1)
       goto FTSerror;
    bufp->naxis1 = (short)lpar;

    if (-1 == (src_fhead(bufp->fhead, "NAXIS2", buf, sizeof(buf))))
       goto FTSerror;
    lpar = atol(buf);
    if (lpar<1)
       goto FTSerror;
    bufp->naxis2 = (short)lpar;

    if (-1 == (src_fhead(bufp->fhead, "BITPIX", buf, sizeof(buf))))
       goto FTSerror;
    lpar = atol(buf);
    if (lpar<-32)
       goto FTSerror;
    bufp->bitpix = (short)lpar;
    bufp->size = bufp->bitpix / 8;
    bufp->N = bufp->naxis1 * bufp->naxis2;

    if (-1 == (src_fhead(bufp->fhead, "BZERO", buf, sizeof(buf))))
       bufp->bzero = 0;
    else
    {
       fpar = atof(buf);
       bufp->bzero = fpar;
    }

    if (-1 == (src_fhead(bufp->fhead, "BSCALE", buf, sizeof(buf))))
       bufp->bscale = 1.0;
    else
    {
       fpar = atof(buf);
       if (fpar == 0)
          goto FTSerror;
       bufp->bscale = fpar;
    }

    /*  Check to see if this is the only header block in file */
    if (0<= (bufp->Nheader = src_fhead(bufp->fhead,"END", buf, sizeof(buf))))
       loop = FALSE;
    else
       loop = TRUE;

    last_hdr = bufp->fhead;

    /*  If there is more than one header block, read them in until the end
        is found.                                                          */
    while (loop)
    {
       if (NULL == (last_hdr->next = (struct fhead_t*)
          malloc((u_int) sizeof(struct fhead_t))))
          goto Merror;

       last_hdr = last_hdr->next;
       last_hdr->next = NULL;

       bread = read(fp, (char*)last_hdr->buf, FITS_REC_LEN);
       if (FITS_REC_LEN!= bread)
          goto Ferror;

       if (0<=(bufp->Nheader=src_fhead(bufp->fhead, "END", buf, sizeof(buf))))
          loop = FALSE;
    }
    /*
     * The END card of the Basic-FITS header has been seen, and the blank
     * line padding has been skipped.   Now read the data into the buffer
     */

    /*  First, check to make sure data format is one that is supported */
    if ((bufp->bitpix>32) || (bufp->bitpix<-32))
    {
       printf("rf:  %d BITPIX format not supported.\n",bufp->bitpix);
       return(-4);
    }


    /*  calculate number of blocks to be read in from input file */
    if (bufp->size<0)
       numrec = ceil( (double)bufp->N * (-bufp->size) / FITS_REC_LEN);
    else
       numrec = ceil( (double)bufp->N * bufp->size / FITS_REC_LEN);

    pixels_per_rec = FITS_REC_LEN / bufp->size;
    if (pixels_per_rec<0)
       pixels_per_rec = - pixels_per_rec;

    bufp->max = -MAXLONG;
    bufp->min = MAXLONG;
    m = n = 0;

    /*  Now read in the data.  Read in each data block, then assign it to the
     *  proper places in the user array, depending on how many bits per pixel
     *  there are.  -32 (4-byte real), 32, 16, or 8 bit integer formats are 
     *  supported.    */
    for (loop=0; loop<numrec; loop++)
    {
       bread = fread((char*)fits_buf.chararr,1, FITS_REC_LEN,fp1);

       if (FITS_REC_LEN!=bread)
       {
          printf(
           "rf: Error reading FITS data- EOF encountered before end of data\n");
       }

       if (bufp->size == 4 || bufp->size == -4)           /* bitpix == 32 or -32 */
	   Sbbe4(fits_buf.chararr, pixels_per_rec);
       else if (bufp->size == 2)                          /* bitpix == 16  */
	   Sbbe2(fits_buf.chararr, pixels_per_rec);

       for (i=0; i<pixels_per_rec; i++)
       {
          if (bufp->size == 4)                               /* bitpix == 32 */
             userarray[m][n] = (float)fits_buf.longarr[i];
          else if (bufp->size == 2)                          /* bitpix == 16  */
             userarray[m][n] = (float)fits_buf.shortarr[i];
          else if (bufp->size == 1)                          /* bitpix == 8   */
             userarray[m][n] = (float)fits_buf.chararr[i];
          else if (bufp->size == -4)                         /* bitpix == -32 */
             userarray[m][n] = fits_buf.farray[i];
                                                             /*  4-byte reals */
/*Note: for the 4-byte real format, steps must be taken to assure that the  
 *      data is read in without loss of precision and to prevent overflow of
 *      the long integer format.  If one intends on reading in real format 
 *      files, the best way is to define userarray[][] as type float.  */

          /*  Now check for data extremes */
          if(userarray[m][n]>bufp->max)
             bufp->max = userarray[m][n];
          if(userarray[m][n]<bufp->min)
             bufp->min = userarray[m][n];

          if (++m == bufp->naxis1)
          {
             m=0;
             n++;
          }
          if (n == bufp->naxis2)
             i= FITS_REC_LEN;
       }
    }
    fclose(fp1);

    /*  Data successfully read in to userarray, return to program  */
    return (0);

/*  Error handling section */
Merror:
   printf("rf: Error allocating memory\n");
   return(-1);

Ferror:
   printf("rf: File read error\n");
   return(-2);

FTSerror:
   printf("rf: File not in FITS format\n");
   return(-3);
}

/*----------------------------------------------------------------------------
 *  writefits - write a file in the proper FITS format, using the header as
 *              defined in the BUFFER "bufp", and the data in userarray.  The
 *              FITS buffer in bufp must have already been built, using
 *              init_fitshead or some other routines using the build_card
 *              functions.  Or the buffer may have been read in from a file
 *              using readfits, and the header is being written to a new file
 *              without changes.
 *---------------------------------------------------------------------------*/
int writefits(filename,bufp,userarray)
char *filename;
struct BUFFER *bufp;
float userarray[][YPIX];
{
   int loop,
       numrec,
       items_per_rec,
       bw,i, m, n,
       fp;
   union intbufs databuf;
   struct fhead_t *fhead;   


   /*  Check header buffer for header information */   
   fhead = bufp->fhead;   
   if (fhead==NULL)
   {
      printf("rf: Error - no header block defined, file not written\n");
      return(-1);
   }

   /*  Open output file */
   if ((fp1 = fopen(filename, "wb")) == NULL)
   {
      printf("wf: Error opening file for writing\n");
      return(-1);
   }

   fp= fileno(fp1);

   /*  Write FITS header to file */
   while( fhead!= NULL)
   {
      if (FITS_REC_LEN!=write(fp,fhead->buf,FITS_REC_LEN))
         goto Ferror;

      fhead = fhead->next;
   }
   /*  Output data */
   items_per_rec = FITS_REC_LEN/bufp->size;
   if (items_per_rec<0)
      items_per_rec = -items_per_rec;


   /*  Write FITS data to file.  First, calculate the number of data blocks to
    *  be written.  Then fill each data block and write it to the file,
    *  according to the bits/pixel value defined in the header block.
    */
   if (bufp->size>0)
      numrec = ceil((double)bufp->N * bufp->size / FITS_REC_LEN);
   else
      numrec = ceil((double)bufp->N *(-bufp->size) / FITS_REC_LEN);
   m = n = 0;
   for (loop=0; loop<numrec; loop++)
   {
      for (i=0; i<items_per_rec; i++)
      {
         /*  First, fill fits record with pixels, in the proper order  */
         if (n<bufp->naxis2)
         {
            if (bufp->size == 4)
               databuf.longarr[i] = (long)userarray[m][n];
            else if (bufp->size == 2)
               databuf.shortarr[i] = (short)userarray[m][n];
            else if (bufp->size == 1)
               databuf.chararr[i] = (unsigned char)userarray[m][n];
            else if (bufp->size == -4)
               databuf.farray[i] = userarray[m][n];
            if (++m == bufp->naxis1)
            {
               n++;
               m = 0;
            }
         }
         else
         {
            /*  Done with data array, fill out remaining buffer with zeros.  */
            if (bufp->size == 4)
               databuf.longarr[i] = 0;
            else if (bufp->size == 2)
               databuf.shortarr[i] = 0;
            else if (bufp->size == 1)
               databuf.chararr[i] = 0;
            else if (bufp->size == -4)
               databuf.farray[i] = 0.0;
         }
      }
      /*  Now write FITS data record  */

      if (bufp->size == 4 || bufp->size == -4)        /* bitpix == 32 or -32 */
	  Sbbe4(databuf.chararr, items_per_rec);
      else if (bufp->size == 2)                          /* bitpix == 16  */
	  Sbbe2(databuf.chararr, items_per_rec);

      bw = write(fp,databuf.chararr,FITS_REC_LEN);
      if (bw != FITS_REC_LEN)
         goto Ferror;
   }
   /*  Finished, close file and exit */
   fclose(fp1);
   return 0;

Ferror:
   printf("wf:  Error writing FITS file\n");
   return -1;
}
/*-----------------------------------------------------------------------------
 *  src_fhead - search the fits header for the given keyword.  If found, return
 *              the string containing the data value, if any, and the line
 *              number where the keyword was found.
 *---------------------------------------------------------------------------*/
int src_fhead(hdr, keyword, val_str, val_str_size)
struct fhead_t *hdr;
char           *keyword;
char           *val_str;
int             val_str_size;
{
   int   i, j, lnumber;
   char  *cptr,
         *fits_buf_ptr;
   char  kw[10];
   char line[81];

   strcpy(val_str,"");
   lnumber=0;
   while (hdr)
   {
      fits_buf_ptr = (char*) hdr->buf;
      for (i=0; i<36; i++)            /* Search all 36 lines of the header */
      {
         lnumber++;
         memcpy(line, fits_buf_ptr, 80);    /* Transfer header line to buffer*/
         line[80] = 0x00;             /* Terminate character string properly */
         memcpy(kw,line,8);           /* extract the keyword                 */
         kw[8] = 0x00;                /*Terminate the keyword string properly*/
         j = 9;
         while ((--j>1) && (kw[j-1] == ' '))    /*Remove any spaces from the */
           kw[j-1] = kw[j];                     /*keyword                    */

         /* If the keyword matches any of those in the header, read out the
          * data field */
         if (!strcmp(keyword, kw))
         {
            for (cptr = line+9; *cptr == ' ' && cptr < line+80; cptr++);
            /*  If line holds a quote, read value from within quotes,
             *  otherwise return all non-space characters in data field. */
               if (*cptr == '\'')
               {
                  cptr++;
                  if (NULL!=(cptr= strtok(cptr,"'")))
                     strcpy( val_str, cptr);
               }
               else
               {
                  if (NULL!=(cptr = strtok(cptr," ")))
                     strncpy( val_str, cptr, val_str_size);
               }
               return lnumber;
             }
             fits_buf_ptr+=80;
         }
      hdr = hdr->next;
   }
   return -1;
}
/*-----------------------------------------------------------------------------
 *  src_fhead - search the fits header for the given keyword.  If found, return
 *              the string containing the data value, if any, and the line
 *              number where the keyword was found.
 *---------------------------------------------------------------------------*/
void printfitshead(hdr)
struct fhead_t *hdr;
{
   int   i, j, lnumber;
   char  *cptr,
         *fits_buf_ptr;
   char  kw[10];
   char line[81];

   lnumber=0;
   while (hdr)
   {
      fits_buf_ptr = (char*) hdr->buf;
      for (i=0; i<36; i++)            /* Search all 36 lines of the header */
      {
         lnumber++;
         memcpy(line, fits_buf_ptr, 80);    /* Transfer header line to buffer*/
         line[80] = 0x00;             /* Terminate character string properly */
         printf("%s\n",line);
         fits_buf_ptr+=80;
      }
      hdr = hdr->next;
   }
}
/*----------------------------------------------------------------------------
 *  free_buffer - release memory that has been allocated for the FITS header
 *                buffer storage area.
 *---------------------------------------------------------------------------*/
int free_buffer(bufp)
struct BUFFER * bufp;
{
   struct fhead_t *nxt_hdr,
                  *tmp_hdr;

   nxt_hdr = bufp->fhead;
   while (nxt_hdr)
   {
      tmp_hdr = nxt_hdr;
      nxt_hdr = nxt_hdr->next;
      free( (char*)tmp_hdr);
   }
   bufp->fhead = NULL;
}
/*---------------------------------------------------------------------------
 *  printcard - print all card images
 *--------------------------------------------------------------------------*/
void printcard(card1)
char card1[];
{
    int             i;
    char            card2[80];

    strncpy (card2, card1,80);
    for (i = 79; i > 0; i--)
	if (card2[i] != ' ')
	    break;
    card2[i + 1] = '\0';
/*    if (i > 0) */
	printf ("%s\n", card2);
    return;
}
/*----------------------------------------------------------------------------
 * build_card - make a FITS header line with keyword, value (20 characters),
 *              and comment.  Put it into the FITS header buffer, and advance
 *              the pointer which points to the location of the next header
 *              line.
 *---------------------------------------------------------------------------*/
char * build_card( cptr, keyword, value, comment, fbuf)
   char * cptr;
   char * keyword;
   char * value;
   char * comment;
   struct  BUFFER *fbuf;
{
   char outbuf[85];
   int l;

   if (strlen(value)>19)
      sprintf(outbuf,"%-8.8s= %30.30s / %-s ",keyword,value,comment);
   else
      sprintf(outbuf,"%-8.8s= %20.20s / %-s ",keyword,value,comment);
   l = strlen(outbuf);
   if (l<80)
      memset( &outbuf[l],' ',80-l);
   outbuf[80] = '\0';
   fbuf->Nheader++;
   strncpy( cptr, outbuf,80);
   cptr += 80;

   if (fbuf->Ncurbuf++ == 36)
   {
      if (NULL == (fbuf->fhead->next = (struct fhead_t*)
          malloc((u_int) sizeof(struct fhead_t))))
          goto Merror;
      fbuf->fhead = fbuf->fhead->next;
      fbuf->fhead->next = NULL;

      fbuf->Ncurbuf = 1;
      cptr = (char*)fbuf->fhead->buf;
   }
   return ( cptr);
Merror:
   printf("Error allocating memory!\n");
   exit(1);
}
/*----------------------------------------------------------------------------
 *  build_card2 - make a FITS header line with 50 character value, no comment.
 *                Add it to FITS buffer, advance pointer to next header line.
 *---------------------------------------------------------------------------*/
char * build_card2( cptr, keyword, value, fbuf)
   char * cptr;
   char * keyword;
   char * value;
   struct BUFFER *fbuf;
{
   char outbuf[85];
   int l;

   sprintf(outbuf,"%-8.8s= '%-.58s' ", keyword,value);
   l = strlen(outbuf);
   if (l < 80)
      memset( &outbuf[l], ' ', 80-l);
   outbuf[80] = '\0';
   strncpy(cptr, outbuf, 80);
   cptr += 80;

   if (fbuf->Ncurbuf++ == 36)
   {
      if (NULL == (fbuf->fhead->next = (struct fhead_t*)
          malloc((u_int) sizeof(struct fhead_t))))
          goto Merror;
      fbuf->fhead = fbuf->fhead->next;
      fbuf->fhead->next = NULL;

      fbuf->Ncurbuf = 1;
      cptr = (char*)fbuf->fhead->buf;
   }
   return(cptr);
Merror:
   printf("Error allocating memory!\n");
   exit(1);
}
/*----------------------------------------------------------------------------
 *  build_comment - make a FITS header line for COMMENT or HISTORY keyword.
 *                Add it to FITS buffer, advance pointer to next header line.
 *---------------------------------------------------------------------------*/
char * build_comment( cptr, keyword, value, fbuf)
   char * cptr;
   char * keyword;
   char * value;
   struct BUFFER *fbuf;
{
   char outbuf[85];
   int l;

   sprintf(outbuf,"%-8.8s  %-.70s", keyword,value);
   l = strlen(outbuf);
   if (l < 80)
      memset( &outbuf[l], ' ', 80-l);
   outbuf[80] = '\0';
   strncpy(cptr, outbuf, 80);
   cptr += 80;

   if (fbuf->Ncurbuf++ == 36)
   {
      if (NULL == (fbuf->fhead->next = (struct fhead_t*)
          malloc((u_int) sizeof(struct fhead_t))))
          goto Merror;
      fbuf->fhead = fbuf->fhead->next;
      fbuf->fhead->next = NULL;

      fbuf->Ncurbuf = 1;
      cptr = (char*)fbuf->fhead->buf;
   }
   return(cptr);
Merror:
   printf("Error allocating memory!\n");
   exit(1);
}
/*----------------------------------------------------------------------------
 *  writeppm - write a file in the proper ppm format, using the header as
 *              defined in the BUFFER "bufp", and the data in userarray.  The
 *              FITS buffer in bufp must have already been built, using
 *              init_fitshead or some other routines using the build_card
 *              functions.  Or the buffer may have been read in from a file
 *              using readfits, and the header is being written to a new file
 *              without changes.
 *---------------------------------------------------------------------------*/
int writeppm(filename,bufp,uar)
char *filename;
struct BUFFER *bufp;
float uar[][YPIX];
{
   int loop,
       numrec,
       items_per_rec,
       bw,i, m, n,
       fp;
   char dbyte;
   union intbufs databuf;
   struct fhead_t *fhead;

   /*  Check header buffer for header information */
   fhead = bufp->fhead;
   if (fhead==NULL)
   {
      printf("rf: Error - no header block defined, file not written\n");
      return(-1);
   }

   /*  Open output file */
   if ((fp1 = fopen(filename, "w")) == NULL)
   {
      printf("wf: Error opening file for writing\n");
      return(-1);
   }

   fp= fileno(fp1);

   numrec = m = n = 0;

   fprintf(fp1,"P6\n%d %d\n255\n",bufp->naxis1,bufp->naxis2);
   for (n=bufp->naxis2-1; n>-1; n--)
      for (m=0; m<bufp->naxis1; m++)
      {
/*
         fprintf(fp1,"%3d %3d %3d",(int)(uar[m][n]),(int)(uar[m][n]),(int)(uar[m][n]));
         if (++numrec==5)
         {
            fprintf(fp1,"\n");
            numrec=0;
         }
         else
            fprintf(fp1," ");
*/
         dbyte = (char)(uar[m][n]);
         putc(dbyte,fp1);
         putc(dbyte,fp1);
         putc(dbyte,fp1);
      }
   /*  Finished, close file and exit */
   fclose(fp1);
   return 0;
}

/*==================================End of File==============================*/
