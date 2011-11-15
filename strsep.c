/*
 * Strsep-ish routines...
 *
 * Copyright 2008 by Gray Watson
 *
 * This file is part of the sortu package.
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of Gray Watson not be used in advertising
 * or publicity pertaining to distribution of the document or software
 * without specific, written prior permission.
 *
 * Gray Watson makes no representations about the suitability of the
 * software described herein for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The author may be contacted via http://256.com/gray/
 *
 * The author may be reached via http://256.com/gray/
 */

static	char	*rcs_id =
"$Id$";

/*
 * char *strsep
 *
 * DESCRIPTION:
 *
 * This is a function which should be in libc in every Unix.  Grumble.
 * It basically replaces the strtok function because it is reentrant.
 * This tokenizes a string by returning the next token in a string and
 * punching a \0 on the first delimiter character past the token.  The
 * difference from strtok is that you pass in the address of a string
 * pointer which will be shifted allong the buffer being processed.
 * With strtok you passed in a 0L for subsequant calls.  Yeach.
 *
 * This will count the true number of delimiter characters in the
 * string and will return an empty token (one with \0 in the zeroth
 * position) if there are two delimiter characters in a row.
 *
 * Consider the following example:
 *
 * char *tok, *str_p = "1,2,3, hello there ";
 *
 * while (1) { tok = strsep(&str_p, " ,"); if (tok == 0L) { break; } }
 *
 * strsep will return as tokens: "1", "2", "3", "", "hello", "there",
 * "".  Notice the two empty "" tokens where there were two delimiter
 * characters in a row ", " and at the end of the string where there
 * was an extra delimiter character.  If you want to ignore these
 * tokens then add a test to see if the first character of the token
 * is \0.
 *
 * RETURNS:
 *
 * Success - Pointer to the next delimited token in the string.
 *
 * Failure - 0L if there are no more tokens.
 *
 * ARGUMENTS:
 *
 * string_p <-> Pointer to a string pointer which will be searched for
 * delimiters.  \0's will be added to this buffer.
 *
 * delim -> List of delimiter characters which separate our tokens.
 * It does not have to remain constant through all calls across the
 * same string.
 */
char	*argv_strsep(char **string_p, const char *delim)
{
  char		*str_p, *tok;
  const char	*delim_p;
  
  /* no tokens left? */
  str_p = *string_p;
  if (str_p == 0L) {
    return 0L;
  }
  
  /* now find end of token */
  tok = str_p;
  for (; *str_p != '\0'; str_p++) {
    
    for (delim_p = delim; *delim_p != '\0'; delim_p++) {
      if (*delim_p == *str_p) {
	/* punch the null */
	*str_p = '\0';
	*string_p = str_p + 1;
	return tok;
      }
    }
  }
  
  /* there are no more delimiter characters */
  *string_p = 0L;
  return tok;
}
