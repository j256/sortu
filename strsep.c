/*
 * Strsep-ish routines...
 *
 * Copyright 1998 by Gray Watson
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
 * The author may be reached via http://256.com/gray/
 */

static	char	*rcs_id =
"$Id$";

#ifndef HAS_STRSEP
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
char	*strsep(char **string_p, const char *delim)
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
#endif

/*
 * const char *strsep_len
 *
 * DESCRIPTION:
 *
 * This is a function is much like strsep however it is non
 * destructive.  This tokenizes a string by returning the next token
 * in a string and by passing back, in the len_p argument, the length
 * of the token up to the first delimiter character.  You pass in the
 * address of a string pointer which will be shifted allong the buffer
 * being processed.
 *
 * This will count the true number of delimiter characters in the
 * string and will return an empty token (one with a length of 0) if
 * there are two delimiter characters in a row.
 *
 * Consider the following example:
 *
 * char *tok, *str_p = "1,2,3, hello there "; int len;
 *
 * while (1) { tok = strsep_len(&str_p, 0L, " ,", 2, &len); if (tok ==
 * 0L) { break; } }
 *
 * strsep_len will return as tokens/len: "1" len 1, "2" len 1, "3" len
 * 1, "" len 0, "hello" len 5, "there" len 5, "" len 0.  Notice the
 * two empty "" tokens where there were two delimiter characters in a
 * row ", " and at the end of the string where there was an extra
 * delimiter character.  If you want to ignore these tokens then add a
 * test to see if the length is 0.
 *
 * RETURNS:
 *
 * Success - Pointer to the next delimited token in the string.
 *
 * Failure - 0L if there are no more tokens.
 *
 * ARGUMENTS:
 *
 * string_p -> Pointer to a string pointer which will be searched for
 * delimiters.  NOTE: this should be const char ** but cannot be
 * because of some compiler spooge.
 *
 * bounds_p -> The end of the string.  If 0L then look for the \0.
 *
 * delim -> List of delimiter characters which separate our tokens.
 * It does not have to remain constant through all calls across the
 * same string.
 *
 * delim_len -> Length of the delimiter string.  This is used to allow
 * the programmer to specify whether \0 is a delimiter or not.
 * Specify < 0 here to have it look for (but not include as a delim)
 * the \0.
 *
 * len_p <- Returns the length of the token which was descovered in
 * the buffer.
 */
char	*strsep_len(char **string_p, const char *bounds_p,
		    const char *delim, const int delim_len, int *len_p)
{
  char		*str_p, *tok;
  const char	*delim_p, *loc_bounds_p, *delim_bounds_p;
  
  /* no tokens left? */
  str_p = *string_p;
  if (str_p == 0L) {
    if (len_p != 0L) {
      *len_p = 0;
    }
    return 0L;
  }
  
  /* find the max spot */
  if (bounds_p == 0L) {
    for (loc_bounds_p = str_p; *loc_bounds_p != '\0'; loc_bounds_p++) {
    }
  }
  else {
    loc_bounds_p = bounds_p;
  }
  
  /* find the delim max */
  if (delim_len >= 0) {
    delim_bounds_p = delim + delim_len;
  }
  else {
    for (delim_bounds_p = delim; *delim_bounds_p != '\0'; delim_bounds_p++) {
    }
  }
  
  /* now find end of token */
  tok = str_p;
  while (1) {
    
    /* did we reach the end of the string? */
    if (str_p >= loc_bounds_p) {
      *string_p = 0L;
      break;
    }
    
    /* look for a matching delimiter */
    for (delim_p = delim; delim_p < delim_bounds_p; delim_p++) {
      if (*delim_p == *str_p) {
	/* we have a match, set the string pointer ahead */
	*string_p = str_p + 1;
	if (len_p != 0L) {
	  *len_p = str_p - tok;
	}
	return tok;
      }
    }
    
    str_p++;
  }
  
  if (len_p != 0L) {
    *len_p = str_p - tok;
  }
  return tok;
}
