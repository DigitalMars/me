/*_ buffer.c   Sun Dec  4 1988 */
/*
 * Buffer management.
 * Some of the functions are internal,
 * and some are actually attached to user
 * keys. Like everyone else, they set hints
 * for the display system.
 */
#include        <stdio.h>
#include        "ed.h"

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
usebuffer(f, n)
{
    BUFFER *bp;
    int    s;
    char   bufn[NBUFN];

    bufn[0] = 0;
    if ((s=mlreply("Use buffer: ", bufn, NBUFN)) != TRUE)
	return (s);
    if ((bp=buffer_find(bufn, TRUE, 0)) == NULL)
	return (FALSE);
    return buffer_switch(bp);
}

/*********************************
 * Make next buffer in list the current one.
 * Put it into the current window if it isn't displayed.
 */

buffer_next(f, n)
{
    BUFFER *bp;

    bp = curbp->b_bufp;
    if (bp == NULL)
	bp = bheadp;
    return buffer_switch(bp);
}

/***************************
 * Switch to buffer bp.
 * Returns:
 *	TRUE or FALSE
 */

int buffer_switch(bp)
register BUFFER *bp;
{
    register WINDOW *wp;

    if (--curbp->b_nwnd == 0) {             /* Last use.            */
	curbp->b_dotp  = curwp->w_dotp;
	curbp->b_doto  = curwp->w_doto;
	curbp->b_markp = curwp->w_markp;
	curbp->b_marko = curwp->w_marko;
    }
    curbp = bp;                             /* Switch.              */
    curwp->w_bufp  = bp;
    curwp->w_linep = bp->b_linep;           /* For macros, ignored. */
    curwp->w_flag |= WFMODE|WFFORCE|WFHARD; /* Quite nasty.         */
    if (bp->b_nwnd++ == 0) {                /* First use.           */
	curwp->w_dotp  = bp->b_dotp;
	curwp->w_doto  = bp->b_doto;
	curwp->w_markp = bp->b_markp;
	curwp->w_marko = bp->b_marko;
	return (TRUE);
    }
    else
    {
	/* Look for the existing window onto buffer bp			*/
	for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
	{
	    if (wp!=curwp && wp->w_bufp==bp)
	    {
		curwp->w_dotp  = wp->w_dotp;
		curwp->w_doto  = wp->w_doto;
		curwp->w_markp = wp->w_markp;
		curwp->w_marko = wp->w_marko;
		break;
	    }
	}
    }
    return TRUE;
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
killbuffer(f, n)
{
	BUFFER *bp;
            register int    s;
        char            bufn[NBUFN];

	bufn[0] = 0;
        if ((s=mlreply("Kill buffer: ", bufn, NBUFN)) != TRUE)
                return (s);
        if ((bp=buffer_find(bufn, FALSE, 0)) == NULL) /* Easy if unknown.     */
                return (TRUE);
	return buffer_remove(bp);
}

/**********************
 * Remove buffer bp.
 * Returns:
 *	0	failed
 *	!=0	succeeded
 */

int buffer_remove(bp)
BUFFER *bp;
{
	register BUFFER *bp1;
        register BUFFER *bp2;

        if (bp->b_nwnd != 0) {                  /* Error if on screen.  */
                mlwrite("Buffer is being displayed");
                return (FALSE);
        }
        if (!buffer_clear(bp))			/* Blow text away	*/
	    return FALSE;
        free((char *) bp->b_linep);             /* Release header line. */
        bp1 = NULL;                             /* Find the header.     */
        bp2 = bheadp;
        while (bp2 != bp) {
                bp1 = bp2;
                bp2 = bp2->b_bufp;
        }
        bp2 = bp2->b_bufp;                      /* Next one in chain.   */
        if (bp1 == NULL)                        /* Unlink it.           */
                bheadp = bp2;
        else
                bp1->b_bufp = bp2;
        free((char *) bp);                      /* Release buffer block */
        return (TRUE);
}

/*
 * List all of the active
 * buffers. First update the special
 * buffer that holds the list. Next make
 * sure at least 1 window is displaying the
 * buffer list, splitting the screen if this
 * is what it takes. Lastly, repaint all of
 * the windows that are displaying the
 * list. Bound to "C-X C-B".
 */
listbuffers(f, n)
{
        register WINDOW *wp;
        register BUFFER *bp;
        register int    s;

        if ((s=makelist()) != TRUE)
                return (s);
        if (blistp->b_nwnd == 0) {              /* Not on screen yet.   */
                if ((wp=wpopup()) == NULL)
                        return (FALSE);
                bp = wp->w_bufp;
                if (--bp->b_nwnd == 0) {
                        bp->b_dotp  = wp->w_dotp;
                        bp->b_doto  = wp->w_doto;
                        bp->b_markp = wp->w_markp;
                        bp->b_marko = wp->w_marko;
                }
                wp->w_bufp  = blistp;
                ++blistp->b_nwnd;
        }
        wp = wheadp;
        while (wp != NULL) {
                if (wp->w_bufp == blistp) {
                        wp->w_linep = lforw(blistp->b_linep);
                        wp->w_dotp  = lforw(blistp->b_linep);
                        wp->w_doto  = 0;
                        wp->w_markp = NULL;
                        wp->w_marko = 0;
                        wp->w_flag |= WFMODE|WFHARD;
                }
                wp = wp->w_wndp;
        }
        return (TRUE);
}

/*
 * This routine rebuilds the
 * text in the special secret buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory).
 */
makelist()
{
        register char   *cp1;
        register char   *cp2;
        register int    c;
        register BUFFER *bp;
        register LINE   *lp;
        register int    nbytes;
        register int    s;
        register int    type;
        char            b[6+1];
        char            line[128];

        blistp->b_flag &= ~BFCHG;               /* Don't complain!      */
        if ((s=buffer_clear(blistp)) != TRUE)         /* Blow old text away   */
                return (s);
        strcpy(blistp->b_fname, "");
        if (addline("C   Size Buffer           File") == FALSE
        ||  addline("-   ---- ------           ----") == FALSE)
                return (FALSE);
	/* For all buffers      */
        for (bp = bheadp; bp != NULL; bp = bp->b_bufp)
	{
                if ((bp->b_flag&BFTEMP) != 0) { /* Skip magic ones.     */
                        continue;
                }
                cp1 = &line[0];                 /* Start at left edge   */
                if ((bp->b_flag&BFCHG) != 0)    /* "*" if changed       */
                        *cp1++ = '*';
                else
                        *cp1++ = ' ';
                *cp1++ = ' ';                   /* Gap.                 */
                nbytes = 0;                     /* Count bytes in buf.  */
                lp = lforw(bp->b_linep);
                while (lp != bp->b_linep) {
                        nbytes += llength(lp)+1;
                        lp = lforw(lp);
                }
                buffer_itoa(b, 6, nbytes);             /* 6 digit buffer size. */
		strcpy(cp1,b);
		cp1 += strlen(cp1);
                *cp1++ = ' ';                   /* Gap.                 */
		strcpy(cp1,bp->b_bname);	/* buffer name		*/
		cp1 += strlen(cp1);
                cp2 = &bp->b_fname[0];          /* File name            */
                if (*cp2 != 0) {
                        while (cp1 < &line[25])
                                *cp1++ = ' ';
			*cp1++ = ' ';
                        while ((c = *cp2++) != 0) {
                                if (cp1 < &line[sizeof(line)-1])
                                        *cp1++ = c;
                        }
                }
                *cp1 = 0;                       /* Add to the buffer.   */
                if (addline(line) == FALSE)
                        return (FALSE);
        }
        return (TRUE);                          /* All done             */
}

buffer_itoa(buf, width, num)
register char   buf[];
register int    width;
register int    num;
{
        buf[width] = 0;                         /* End of string.       */
        while (num >= 10) {                     /* Conditional digits.  */
                buf[--width] = (num%10) + '0';
                num /= 10;
        }
        buf[--width] = num + '0';               /* Always 1 digit.      */
        while (width != 0)                      /* Pad with blanks.     */
                buf[--width] = ' ';
}

/*
 * The argument "text" points to
 * a string. Append this line to the
 * buffer list buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 */
addline(text)
char    *text;
{
        register LINE   *lp;
        register int    i;
        register int    ntext;

        ntext = strlen(text);
        if ((lp=line_realloc(NULL,ntext)) == NULL)
                return (FALSE);
        for (i=0; i<ntext; ++i)
                lputc(lp, i, text[i]);
        blistp->b_linep->l_bp->l_fp = lp;       /* Hook onto the end    */
        lp->l_bp = blistp->b_linep->l_bp;
        blistp->b_linep->l_bp = lp;
        lp->l_fp = blistp->b_linep;
        if (blistp->b_dotp == blistp->b_linep)  /* If "." is at the end */
                blistp->b_dotp = lp;            /* move it to new line  */
        return (TRUE);
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Buffers
 * that hold magic internal stuff are
 * not considered; who cares if the
 * list of buffer names is hacked.
 * Return FALSE if no buffers
 * have been changed.
 */
anycb()
{
    register BUFFER *bp;

    for (bp = bheadp; bp; bp = bp->b_bufp)
    {
	if ((bp->b_flag & (BFTEMP | BFCHG)) == BFCHG)
	    return TRUE;
    }
    return FALSE;
}

/********************************
 * Find a buffer, by name. Return a pointer
 * to the BUFFER structure associated with it. If
 * the named buffer is found, but is a TEMP buffer (like
 * the buffer list) complain. If the buffer is not found
 * and the "cflag" is TRUE, create it. The "bflag" is
 * the settings for the flags in in buffer.
 * If bflag specifies a TEMP buffer, then a TEMP buffer can be selected.
 */

BUFFER *buffer_find(bname, cflag, bflag)
register char   *bname;
{
    register BUFFER *bp;
    register LINE   *lp;

    for (bp = bheadp; bp != NULL; bp = bp->b_bufp)
    {
	if (filcmp(bname, bp->b_bname) == 0)
	{   
	    if ((bflag & BFTEMP) == 0 && (bp->b_flag & BFTEMP) != 0)
	    {
		mlwrite("Cannot select builtin buffer");
		return (NULL);
	    }
	    return (bp);
	}
    }
    if (cflag != FALSE)
    {
	if ((bp=(BUFFER *)calloc(sizeof(BUFFER),1)) == NULL)
	    return (NULL);
	if ((lp=line_realloc(NULL,0)) == NULL)
	{   free((char *) bp);
	    return (NULL);
	}
	bp->b_bufp  = bheadp;
	bheadp = bp;
	bp->b_dotp  = lp;
	bp->b_flag  = bflag;
	bp->b_linep = lp;
	strcpy(bp->b_fname, "");
	strcpy(bp->b_bname, bname);
	lp->l_fp = lp;
	lp->l_bp = lp;
    }
    return (bp);
}

/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return TRUE if everything
 * looks good.
 */
buffer_clear(bp)
register BUFFER *bp;
{
        register LINE   *lp;
        register int    s;

	/*if (bp->b_flag & BFRDONLY)
	    return FALSE;*/
        if ((bp->b_flag&BFTEMP) == 0            /* Not scratch buffer.  */
        && (bp->b_flag&BFCHG) != 0              /* Something changed    */
        && (s=mlyesno("Discard changes [y/n]? ")) != TRUE)
                return (s);
        bp->b_flag  &= ~BFCHG;                  /* Not changed          */
        while ((lp=lforw(bp->b_linep)) != bp->b_linep)
                line_free(lp);
        bp->b_dotp  = bp->b_linep;              /* Fix "."              */
        bp->b_doto  = 0;
        bp->b_markp = NULL;                     /* Invalidate "mark"    */
        bp->b_marko = 0;
        return (TRUE);
}
