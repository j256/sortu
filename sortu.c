/*
 * Sort/uniq program
 *
 * Copyright 1999 by Gray Watson
 *
 * $Id$
 */

#include <errno.h>
#include <string.h>

#include "argv.h"
#include "table.h"

#define DEFAULT_DELIM	" "
#define LINE_SIZE	1024

/*
 * NOTE: RANLIB_PROG and STRIP_PROG should be defined in Buildfile
 */

/* argument variables */
static	int		count_sort_b = 0;	/* sort by count not data */
static	int		no_counts_b = 0;	/* don't output str counts */
static	char		*delim_str = DEFAULT_DELIM; /* field delim char */
static	int		field = -1;		/* field to use */
static	int		min_matches = 0;	/* minimum number of matches */
static	int		max_matches = 0;	/* max number of matches */
static	int		numbers_b = 0;		/* fields are numbers */
static	int		verbose_b = 0;		/* verbose flag */
static	argv_array_t	files;			/* work files */

/* argument array */
static	argv_t	args[] = {
  { 'c',	"count-sort",	ARGV_BOOL_INT,		&count_sort_b,
    NULL,		"sort by the count not key" },
  { 'C',	"no-counts",	ARGV_BOOL_INT,		&no_counts_b,
    NULL,		"don't output string counts" },
  { 'd',	"delimiter",	ARGV_CHAR_P,		&delim_str,
    "char",		"field delimiter string (default \" \")" },
  { 'f',	"field",	ARGV_INT,		&field,
    "number",		"which field to use otherwise 1st" },
  { 'm',	"minimum-matches", ARGV_INT,		&min_matches,
    "number",		"minimum # matches to show" },
  { 'M',	"maximum-matches", ARGV_INT,		&max_matches,
    "number",		"maximum # matches to show" },
  { 'n',	"numbers",	ARGV_BOOL_INT,		&numbers_b,
    NULL,		"treat field as signed long number" },
  { 'v',	"verbose",	ARGV_BOOL_INT,		&verbose_b,
    NULL,		"verbose mode" },
  { ARGV_MAYBE,	NULL,		ARGV_CHAR_P | ARGV_FLAG_ARRAY, &files,
    "file(s)",		"file(s) to process" },
  { ARGV_LAST }
};

/* 
 * static int count_compare
 *
 * DESCRIPTION:
 *
 * Compare our entries in the table.
 *
 * RETURNS:
 *
 * -1, 0, or 1 if key1 is <, ==, or > than key2.
 *
 * ARGUMENTS:
 *
 * key1 - Pointer to the first key entry.
 *
 * key1_size - Pointer to the size of the first key entry.
 *
 * data1 - Pointer to the first data entry.
 *
 * data1_size - Pointer to the size of the first data entry.
 *
 * key2 - Pointer to the second key entry.
 *
 * key2_size - Pointer to the size of the second key entry.
 *
 * data2 - Pointer to the second data entry.
 *
 * data2_size - Pointer to the size of the second data entry.
 */
static	int	count_compare(const void *key1, const int key1_size,
			      const void *data1, const int data1_size,
			      const void *key2, const int key2_size,
			      const void *data2, const int data2_size)
{
  const long		*long1_p, *long2_p;
  const unsigned long	*ulong1_p, *ulong2_p;
  const char		*str1_p, *str2_p;
  
  if (count_sort_b) {
    if (numbers_b) {
      /* reverse numeric sort */
      ulong1_p = key1;
      ulong2_p = key2;
      return *ulong1_p - *ulong2_p;
    }
    else {
      /* forward string sort */
      str1_p = key1;
      str2_p = key2;
      return strcmp(str1_p, str2_p);
    }
  }
  else {
    long1_p = data1;
    long2_p = data2;
    return *long2_p - *long1_p;
  }
}

int	main(int argc, char **argv)
{
  FILE		*infile;
  char		*filename, line[LINE_SIZE], *tok, *line_p;
  int		file_c, ret, field_c, key_size, entry_n;
  unsigned long	count, *count_p;
  long		value;
  void		*key_p;
  table_t	*tab;
  table_entry_t	**entries, **entries_p;
  
  argv_process(args, argc, argv);
  
  /* allocate table */
  tab = table_alloc(0, &ret);
  if (tab == NULL) {
    (void)fprintf(stderr, "%s: could not allocate table: %s\n",
		  argv_program, table_strerror(ret));
    exit(1);
  }
  
  /* set auto-adjust flag */
  ret = table_attr(tab, TABLE_FLAG_AUTO_ADJUST);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "%s: could not set auto-adjust for table: %s\n",
		  argv_program, table_strerror(ret));
    exit(1);
  }
  
  /* set table alignment */
  if (! no_counts_b) {
    ret = table_set_data_alignment(tab, sizeof(long));
    if (ret != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "%s: could not set table alignment: %s\n",
		    argv_program, table_strerror(ret));
      exit(1);
    }
  }
  
  file_c = 0;
  while (1) {
    
    if (file_c == 0 && ARGV_ARRAY_COUNT(files) == 0) {
      filename = "stdin";
      infile = stdin;
    }
    else {
      filename = ARGV_ARRAY_ENTRY(files, char *, file_c);
      infile = fopen(filename, "r");
      if (infile == NULL) {
	(void)fprintf(stderr, "%s: could not open file '%s': %s\n",
		      argv_program, filename, strerror(errno));
	exit(1);
      }
    }
    
    while (fgets(line, sizeof(line), infile) != NULL) {
      for (line_p = line; *line_p != '\n' && *line_p != '\0'; line_p++) {
      }
      *line_p = '\0';
      
      /* default is the entire line */
      tok = line;
      line_p = line;
      for (field_c = field - 1; field_c >= 0; field_c--) {
	tok = strsep(&line_p, delim_str);
	if (tok == NULL) {
	  break;
	}
      }
      
      /* oh well, no specified field */
      if (tok == NULL) {
	continue;
      }
      
      if (numbers_b) {
	value = atol(tok);
	key_p = &value;
	key_size = sizeof(value);
      }
      else {
	key_p = tok;
	key_size = -1;
      }
      
      if (no_counts_b) {
	ret = table_insert(tab, key_p, key_size, NULL, 0, NULL, 0);
      }
      else {
	count = 1;
	ret = table_insert(tab, key_p, key_size, &count, sizeof(count),
			   (void **)&count_p, 0);
      }
      if (ret != TABLE_ERROR_NONE) {
	if (ret != TABLE_ERROR_OVERWRITE) {
	  (void)fprintf(stderr, "%s: could not add key to table: %s\n",
			argv_program, table_strerror(ret));
	  exit(1);
	}
	
	if (! no_counts_b) {
	  (*count_p)++;
	}
      }
    }
    
    if (infile != stdin) {
      (void)fclose(infile);
    }
    
    /* we don't do a for() loop because the 1st is special */
    file_c++;
    if (file_c >= ARGV_ARRAY_COUNT(files)) {
      break;
    }
  }
  
  if (verbose_b && (! no_counts_b)) {
    if (numbers_b) {
      (void)printf("%10.10s %10.10s\n", "Count:", "Data:");
      (void)printf("---------- ----------\n");
    }
    else {
      (void)printf("%10.10s Data:\n", "Count:");
      (void)printf("---------- ----------\n");
    }
  }
  
  /* order the table */
  entries = table_order(tab, count_compare, &entry_n, &ret);
  if (entries == NULL) {
    (void)fprintf(stderr, "%s: could not order the table: %s\n",
		  argv_program, table_strerror(ret));
    exit(1);
  }
  
  for (entries_p = entries; entries_p < entries + entry_n; entries_p++) {
    /* get each entry to print */
    ret = table_entry(tab, *entries_p, (void **)&key_p, &key_size,
		      (void **)&count_p, NULL);
    if (ret != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "%s: could not get table entry: %s\n",
		    argv_program, table_strerror(ret));
      exit(1);
    }
    
    /* limit the matches if necessary */
    if (*count_p < min_matches
	|| (max_matches > 0 && *count_p > max_matches)) {
      continue;
    }
    
    if (no_counts_b) {
      (void)printf("%.*s\n", key_size, (char *)key_p);
    }
    else if (numbers_b) {
      (void)printf("%10lu %10ld\n", *count_p, *(long *)key_p);
    }
    else {
      (void)printf("%10lu %.*s\n", *count_p, key_size, (char *)key_p);
    }
  }
  
  (void)table_order_free(tab, entries, entry_n);
  (void)table_free(tab);
  
  argv_cleanup(args);
  exit(0);
}
