/*
 * Test file for the argv routines...
 *
 * Copyright 1995 by Gray Watson
 *
 * This file is part of the argv library.
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
 * The author may be contacted at gray.watson@letters.com
 */

#if HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef LOCAL
#include "dmalloc.h"
#endif

#define ARGV_TEST_MAIN

#include "argv.h"

static	char	*rcs_id =
  "$Id$";

/* argument variables */
static	char		apple		= ARGV_FALSE;
static	int		binary		= 0;
static	char		check		= ARGV_FALSE;
static	int		octal		= 0;
static	int		hexadecimal	= 0;
static	int		decimal		= 0;
static	int		debug		= 0;
static	long		size		= 0L;
static	argv_array_t	baseball;
static	char		little		 = ' ';
static	char		w		= ARGV_FALSE;
static	int		x		= ARGV_FALSE;
static	char		y		= ARGV_FALSE;
static	char		z		= ARGV_FALSE;
static	argv_array_t	left;

static	argv_t	args[] = {
  { '\0',	"w-mark",	ARGV_BOOL,			&w,
      0L,		"w marks the spot" },
  { ARGV_OR },
  { 'x',	"x-mark",	ARGV_BOOL_INT,			&x,
      0L,		"x marks the spot" },
  { 'y',	0L,		ARGV_BOOL,			&y,
      0L,		"y marks the spot" },
  { ARGV_XOR },
  { 'z',	"z-mark",	ARGV_BOOL,			&z,
      0L,		"z marks the spot" },
  
  { 'a',	"apple",	ARGV_BOOL,			&apple,
      "apple",		"an apple a day..." },
  { 'c',	"check",	ARGV_BOOL_ARG | ARGV_FLAG_MAND,	&check,
      "yes/no",		"check it out" },
  { 'b',	"binary",	ARGV_BIN,			&binary,
      "number",		"a binary number" },
  { 'B',	"baseball",	ARGV_CHAR_P | ARGV_FLAG_ARRAY,	&baseball,
      "field",		"array of baseball FIELDS" },
  { 'd',	"decimal",	ARGV_INT,			&decimal,
      "number",		"decimal number" },
  { 'D',	"debug",	ARGV_INCR,			&debug,
      0L,		"debug flag" },
  { 'o',	"octal",	ARGV_OCT,			&octal,
      "number",		"an octal number" },
  { '\0',	"hexadecimal",	ARGV_HEX,			&hexadecimal,
      "number",		"a hexadecimal number" },
  { 's',	"size",		ARGV_SIZE,			&size,
      "size",		"size specifier as number [bkm]" },
  { ARGV_MAND,	0L,		ARGV_CHAR_P | ARGV_FLAG_ARRAY,	&left,
      "file1 [file2 files]",	"miscellaneous file arguments........." },
  
  { ARGV_LAST }
};

int	main(int argc, char ** argv)
{
  int		count;
  
  argv_help_string = "Argv library test program.";
  argv_version_string = "$Revision$";
  
  argv_process(args, argc, argv);
  
  (void)printf("program name = '%s' arg0 = '%s'\n",
	       argv_program, argv_argv[0]);
  
  (void)printf("apple = %s\n", (apple ? "true" : "false"));
  (void)printf("check = %s\n", (check ? "true" : "false"));
  (void)printf("binary = %d, octal %d, hexadecimal %d, decimal %d\n",
	       binary, octal, hexadecimal, decimal);
  (void)printf("debug = %d\n", debug);
  (void)printf("size = %ld\n", size);
  
  if (baseball.aa_entry_n == 0)
    (void)printf("No baseballs specified.\n");
  else {
    for (count = 0; count < baseball.aa_entry_n; count++)
      (void)printf("baseball[%d]: '%s'\n",
		   count, ARGV_ARRAY_ENTRY(baseball, char *, count));
  }
  
  (void)printf("little = '%c'\n", little);
  (void)printf("w = %s\n", (w ? "true" : "false"));
  (void)printf("x = %s\n", (x ? "true" : "false"));
  (void)printf("y = %s\n", (y ? "true" : "false"));
  (void)printf("z = %s\n", (z ? "true" : "false"));
  
  if (left.aa_entry_n > 0)
    for (count = 0; count < left.aa_entry_n; count++)
      (void)printf("left[%d]: '%s'\n",
		   count, ARGV_ARRAY_ENTRY(left, char *, count));
  
  (void)printf("\n");
  
  {
    char	buf[128];
    argv_copy_args(buf, sizeof(buf));
    (void)printf("All args = '%s %s'\n", argv_argv[0], buf);
  }
  
  {
    argv_t	*arg_p;
    int		len;
    char	buf[60];
    
    for (arg_p = args; arg_p->ar_short_arg != ARGV_LAST; arg_p++) {
      if (arg_p->ar_short_arg == ARGV_OR
	  || arg_p->ar_short_arg == ARGV_XOR) {
	continue;
      }
      (void)printf("Argument %d: ", arg_p - args);
      if (arg_p->ar_short_arg != '\0'
	  && arg_p->ar_short_arg != ARGV_MAND) {
	(void)printf("-%c ", arg_p->ar_short_arg);
      }
      if (arg_p->ar_long_arg != NULL) {
	(void)printf("--%s ", arg_p->ar_long_arg);
      }
      len = argv_value_string(arg_p, buf, sizeof(buf));
      (void)printf("%.*s\n", len, buf);
    }
  }
  
  argv_cleanup(args);
  
  (void)exit(0);
}
