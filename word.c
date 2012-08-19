/*_ word.c   Sun Jun 26 1988 */
/*
 * The routines in this file implement commands that work word at a time.
 * There are all sorts of word mode commands. If I do any sentence and/or
 * paragraph mode commands, they are likely to be put in this file.
 */

#include        <stdio.h>
#include	<ctype.h>
#include        "ed.h"


/* Word wrap. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.  Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 *      NOTE:  This function may leaving trailing blanks.
 * Returns TRUE on success, FALSE on errors.
 */
word_wrap()
{
        register int cnt;
	register struct LINE *oldp;

        oldp = curwp->w_dotp;
        cnt = -1;
        do {                            
                cnt++;
                if (! backchar(0, 1))
		    goto err;
        } while (! inword());
        if (! word_back(FALSE, 1))
	    goto err;
	/* If still on same line (but not at the beginning)	*/
        if (oldp == curwp->w_dotp && curwp->w_doto)
	{   int i;

	    if (!random_backdel(0, 1))
		goto err;
	    if (!random_newline(FALSE, 1))
		goto err;
	    oldp = lback(curwp->w_dotp);
	    i = 0;
	    while (1)
	    {	char c;

		c = lgetc(oldp,i);
		if (c != ' ' && c != '\t')
		    break;
		line_insert(1,c);
		i++;
	    }
        }
	while (inword() == TRUE)
	    if (forwchar(FALSE, 1) == FALSE)
		goto err;
        return forwchar(FALSE, cnt);

err:
	return FALSE;
}

/****************************
 * Word wrap the current line.
 */

int word_wrap_line(f, n)
{
    int i;
    int j;
    int col;
    int c;
    int inword;
    int lasti;
    LINE* oldp;
    LINE* dotpsave;
    int dotosave;

    if (n < 0)
	goto err;

    if (window_marking(curwp))
    {   REGION region;
	int s;

	if ((s = getregion(&region)) != TRUE)
	    return s;
	dotpsave = curwp->w_dotp;
	dotosave = curwp->w_doto;
	curwp->w_dotp = region.r_linep;
	curwp->w_doto = region.r_offset;
	n = region.r_nlines;
    }

    while (n-- > 0)
    {
      L1:
	col = 0;
	lasti = 0;
	inword = 0;
	for (i = 0; i < llength(curwp->w_dotp); i++)
	{
	    c = lgetc(curwp->w_dotp, i);
	    if (c == ' ' || c == '\t')
	    {
		if (inword)
		    lasti = i;
		inword = 0;
	    }
	    else
	    {
		inword = 1;
	    }
	    col = getcol(curwp->w_dotp, i);
	    if (col >= term.t_ncol && lasti)
	    {
		if (!forwchar(0, lasti - curwp->w_doto))
		    goto err;
		if (!random_newline(0,1))
		    goto err;

		/* Remove leading whitespace from new line	*/
		while (1)
		{
		    if (!llength(curwp->w_dotp))
			break;
		    c = lgetc(curwp->w_dotp, 0);
		    if (c == ' ' || c == '\t')
		    {
			if (!random_forwdel(0, 1))
			    goto err;
		    }
		    else
			break;
		}

		/* Match indenting of original line (oldp)	*/
		oldp = lback(curwp->w_dotp);
		for (j = 0; j < llength(oldp); j++)
		{
		    c = lgetc(oldp, j);
		    if (c == ' ' || c == '\t')
		    {
			if (!line_insert(1, c))
			    goto err;
		    }
		    else
			break;
		}

		goto L1;
	    }
	}
	if (!forwline(0, 1))
	    goto err;
    }
    if (window_marking(curwp))
    {
	if (dotosave > llength(dotpsave))
	    dotosave = llength(dotpsave);
	curwp->w_dotp = dotpsave;
	curwp->w_doto = dotosave;
    }
    return TRUE;

err:
    return FALSE;
}

/*************************
 * Select word that the cursor is on.
 */

word_select(f,n)
{
    int inw;
    int s;

    inw = inword();
    do
	s = backchar(FALSE, 1);
    while (s && inword() == inw);

    return s &&
	forwchar(FALSE,1) &&
	basic_setmark(FALSE,1) &&
	word_forw(f,n);
}

/******************************
 * Select line that the cursor is on.
 */

word_lineselect(f,n)
{
    return (curwp->w_doto == 0 || gotobol(FALSE,1)) &&
	basic_setmark(FALSE,1) &&
	forwline(f,n);
}

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backchar" and "forwchar" routines. Error if you try to
 * move beyond the buffers.
 */
word_back(f, n)
{
        if (n < 0)
                return (word_forw(f, -n));
        if (backchar(FALSE, 1) == FALSE)
                return (FALSE);
        while (n--) {
#if 0
	    while (inword() == FALSE) {
		if (backchar(FALSE, 1) == FALSE)
		    return (FALSE);
	    }
	    while (inword()) {
		if (backchar(FALSE, 1) == FALSE)
		    return (FALSE);
	    }
#else
	    int inw;

	    inw = inword();
	    do
		if (backchar(FALSE, 1) == FALSE)
		    return (FALSE);
	    while (inword() == inw);
#endif
        }
        return (forwchar(FALSE, 1));
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
word_forw(f, n)
{
        if (n < 0)
                return (word_back(f, -n));
        while (n--) {
#if 0
	    while( inword() == TRUE )
		if( forwchar(FALSE, 1) == FALSE)
		    return( FALSE );
	    while (inword() == FALSE) {
		if (forwchar(FALSE, 1) == FALSE)
		    return (FALSE);
	    }
#else
	    int inw;

	    inw = inword();
	    do
		if (forwchar(FALSE, 1) == FALSE)
		    return (FALSE);
	    while (inword() == inw);
#endif
        }
        return (TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move,
 * convert any characters to upper case. Error if you try and move beyond the
 * end of the buffer. Bound to "M-U".
 */
word_upper(f, n)
{
    return word_setcase(f,n,0);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert characters to lower case. Error if you try and move over the end of
 * the buffer. Bound to "M-L".
 */
word_lower(f, n)
{
    return word_setcase(f,n,1);
}

/*************************
 * Move the cursor forward by the specified number of words. As you move
 * convert the first character of the word to upper case, and subsequent
 * characters to lower case. Error if you try and move past the end of the
 * buffer. Bound to "M-C".
 */

capword(f, n)
{
    return word_setcase(f,n,2);
}

int word_setcase(f,n,flag)
{
    register int    c;

    if (n < 0)
	return (FALSE);
    while (n--) {
	while (inword() == FALSE) {
	    if (forwchar(FALSE, 1) == FALSE)
		return (FALSE);
	}
	if (flag == 2 && inword() != FALSE) {
	    c = lgetc(curwp->w_dotp, curwp->w_doto);
	    if (islower(c))
	    {   c -= 'a'-'A';
		lputc(curwp->w_dotp, curwp->w_doto, c);
		line_change(WFHARD);
	    }
	    if (forwchar(FALSE, 1) == FALSE)
		return (FALSE);
	}
	while (inword() != FALSE) {
	    c = lgetc(curwp->w_dotp, curwp->w_doto);
	    switch (flag)
	    {   case 0:
		    if (islower(c)) {
			c -= 'a'-'A';
			goto L1;
		    }
		    break;
		case 1:
		case 2:
		    if (isupper(c)) {
			c += 'a'-'A';
		    L1: lputc(curwp->w_dotp, curwp->w_doto, c);
			line_change(WFHARD);
		    }
		    break;
	    }
	    if (forwchar(FALSE, 1) == FALSE)
		return (FALSE);
	}
    }
    return (TRUE);
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. Bound to "M-D".
 */
delfword(f, n)
{
        register int    size;
        register LINE   *dotp;
        register int    doto;

        if (n < 0)
                return (FALSE);
        dotp = curwp->w_dotp;
        doto = curwp->w_doto;
        size = 0;
        while (n--) {
                while (inword() == FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        ++size;
                }
                while (inword() != FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        ++size;
                }
        }
        curwp->w_dotp = dotp;
        curwp->w_doto = doto;
        return (line_delete(size, TRUE));
}

/*
 * Kill backwards by "n" words. Move backwards by the desired number of words,
 * counting the characters. When dot is finally moved to its resting place,
 * fire off the kill command. Bound to "M-Rubout" and to "M-Backspace".
 */
delbword(f, n)
{
        register int    size;

        if (n < 0)
                return (FALSE);
        if (backchar(FALSE, 1) == FALSE)
                return (FALSE);
        size = 0;
        while (n--) {
                while (inword() == FALSE) {
                        if (backchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        ++size;
                }
                while (inword() != FALSE) {
                        if (backchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        ++size;
                }
        }
        if (forwchar(FALSE, 1) == FALSE)
                return (FALSE);
        return (line_delete(size, TRUE));
}

/*
 * Return TRUE if the character at dot is a character that is considered to be
 * part of a word. The word character list is hard coded. Should be setable.
 * This routine MUST return only a 1 or a 0.
 */
inword()
{
        register int    c;

        if (curwp->w_doto == llength(curwp->w_dotp))
                return (FALSE);
        c = lgetc(curwp->w_dotp, curwp->w_doto);
	return (isalnum(c) ||
		 c=='$' || c=='_');	/* For identifiers      */
}
