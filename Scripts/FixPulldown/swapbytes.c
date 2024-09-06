/*---------------------------------------------------------------
           FILE: swapbytes.c
      $Revision: 2.1 $
         $State: Exp $
  Last submitted $Date: 94/10/26 18:19:45 $ $Author: jh $

   Locked out by $Locker: jh $

  REVISION HISTORY
  $Log:	swapbytes.c,v $
 * Revision 2.1  94/10/26  18:19:45  jh
 * Separated data-moving function from byte-swapping function.  This is a major
 * change that is not backward-compatible!  The number of arguments has changed
 * in all functions.  Swaps occur in place.  To do a move-with-swap, use
 * memmove prior to calling these functions to move the data to the new place
 * first.
 * 
 * Revision 1.2  94/10/26  16:25:21  jh
 * Copyright changed to distributable.
 * 
 * Revision 1.1  94/10/26  16:17:25  jh
 * Initial revision
 * 

  $Source: /u1/jh/comp/C/RCS/swapbytes.c,v $

  Written in 1994 by Joseph Harrington <jh@mit.edu>.
  This file may be freely distributed.  If you change the
  external behavior, please rename the file, functions, and
  macros to something else, so other people who use these
  functions won't be confused.  Please document all changes
  in the space above.  The author welcomes suggestions.
---------------------------------------------------------------*/
#ifndef LINT
static char rcsid[] = "$Header: /u1/jh/comp/C/RCS/swapbytes.c,v 2.1 94/10/26 18:19:45 jh Exp Locker: jh $";
static char copyright[] =
    "Written in 1994 by Joseph Harrington <jh@mit.edu>.";
#endif
/*---------------------------------------------------------------
swapbytes.c = functions to change byte order

External object files or libraries: none.
---------------------------------------------------------------*/
/* inclusions */

#include <stddef.h>
#include <string.h>

/*----------------------swap2bytes-------------------------------
Effect: Reverses the order of adjacent sets of 2 bytes.

Requires: *ptr:    array of bytes to swap
           n_swap: number of swaps to do
Modifies: *ptr

Implementation notes: Tested bit-shifting method and this is
	just as fast under optimization and doesn't require
	word alignment.
---------------------------------------------------------------*/
void
swap2bytes(void *ptr, size_t n_swap){
    register char t;
    register char *ptrc;

    ptrc = (char *)ptr;
    while (n_swap > 0){
        t        = ptrc[1];
        ptrc[1]  = ptrc[0];
        ptrc[0]  = t;
        ptrc    += 2;
        n_swap--;
    }
    return;
}
/*----------------------swap4bytes-------------------------------
Effect: Reverses the order of adjacent sets of 4 bytes.

Requires: *ptr:    array of bytes to swap
           n_swap: number of swaps to do
Modifies: *ptr

Implementation notes: Tested bit-shifting method and this is
	just as fast under optimization and doesn't require
	word alignment.
---------------------------------------------------------------*/
void
swap4bytes(void *ptr, size_t n_swap){
    register char t;
    register char *ptrc;

    ptrc = (char *)ptr;
    while (n_swap > 0){
        t        = ptrc[3];
        ptrc[3]  = ptrc[0];
        ptrc[0]  = t;
        t        = ptrc[2];
        ptrc[2]  = ptrc[1];
        ptrc[1]  = t;
        ptrc    += 4;
        n_swap--;
    }
    return;
}
/*----------------------swap8bytes-------------------------------
Effect: Reverses the order of adjacent sets of 8 bytes.

Requires: *ptr:    array of bytes to swap
           n_swap: number of swaps to do
Modifies: *ptr

Implementation notes: Tested bit-shifting method and this is
	just as fast under optimization and doesn't require
	word alignment.
---------------------------------------------------------------*/
void
swap8bytes(void *ptr, size_t n_swap){
    register char  t;
    register char *ptrc;

    ptrc = (char *)ptr;
    while (n_swap > 0){
        t        = ptrc[7];
        ptrc[7]  = ptrc[0];
        ptrc[0]  = t;
        t        = ptrc[6];
        ptrc[6]  = ptrc[1];
        ptrc[1]  = t;
        t        = ptrc[5];
        ptrc[5]  = ptrc[2];
        ptrc[2]  = t;
        t        = ptrc[4];
        ptrc[4]  = ptrc[3];
        ptrc[3]  = t;
        ptrc    += 8;
        n_swap--;
    }
    return;
}
