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
static	int		ignore_blanks_b = 0;	/* ignore blank lines */
static	int		no_counts_b = 0;	/* don't output str counts */
static	char		*delim_str = DEFAULT_DELIM; /* field delim char */
static	int		field = -1;		/* field to use */
static	int		case_insens_b = 0;	/* case insensitive matches */
static	int		key_sort_b = 0;		/* sort by key not count */
static	int		loose_fields_b = 0;	/* loose field match */
static	int		min_matches = 0;	/* minimum number of matches */
static	int		max_matches = 0;	/* max number of matches */
static	int		numbers_b = 0;		/* fields are numbers */
static	int		float_numbers_b = 0;	/* fields are floats */
static	int		reverse_sort_b = 0;	/* reverse the sort order */
static	int		verbose_b = 0;		/* verbose flag */
static	argv_array_t	files;			/* work files */

/* argument array */
static	argv_t	args[] = {
  { 'b',	"blank-ignore",	ARGV_BOOL_INT,		&ignore_blanks_b,
    NULL,		"ignore blank lines" },
  { 'C',	"no-counts",	ARGV_BOOL_INT,		&no_counts_b,
    NULL,		"don't output string counts" },
  { 'd',	"delimiter",	ARGV_CHAR_P,		&delim_str,
    "chars",		"field delim string (default \" \")" },
  { 'f',	"field",	ARGV_INT,		&field,
    "number",		"which field to use otherwise 1st" },
  { 'k',	"key-sort",	ARGV_BOOL_INT,		&key_sort_b,
    NULL,		"sort by key not count" },
  { 'l',	"loose-fields",	ARGV_BOOL_INT,		&loose_fields_b,
    NULL,		"ignores white space between fields" },
  { 'm',	"minimum-matches", ARGV_INT,		&min_matches,
    "number",		"minimum # matches to show" },
  { 'M',	"maximum-matches", ARGV_INT,		&max_matches,
    "number",		"maximum # matches to show" },
  { 'i',	"insensitive-case", ARGV_BOOL_INT,	&case_insens_b,
    NULL,		"perform case insensitive matches" },
  { ARGV_OR },
  { 'n',	"numbers",	ARGV_BOOL_INT,		&numbers_b,
    NULL,		"treat field as signed long number" },
  { 'N',	"float-numbers", ARGV_BOOL_INT,		&float_numbers_b,
    NULL,		"treat field as floating point" },
  { 'r',	"reverse-sort",	ARGV_BOOL_INT,		&reverse_sort_b,
    NULL,		"reverse the sort" },
  { 'v',	"verbose",	ARGV_BOOL_INT,		&verbose_b,
    NULL,		"verbose mode" },
  { ARGV_MAYBE,	NULL,		ARGV_CHAR_P | ARGV_FLAG_ARRAY, &files,
    "file(s)",		"file(s) to process else stdin" },
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
 * key1 -> Pointer to the first key entry.
 *
 * key1_size -> Pointer to the size of the first key entry.
 *
 * data1 -> Pointer to the first data entry.
 *
 * data1_size -> Pointer to the size of the first data entry.
 *
 * key2 -> Pointer to the second key entry.
 *
 * key2_size -> Pointer to the size of the second key entry.
 *
 * data2 -> Pointer to the second data entry.
 *
 * data2_size -> Pointer to the size of the second data entry.
 */
static	int	count_compare(const void *key1_p, const int key1_size,
			      const void *data1_p, const int data1_size,
			      const void *key2_p, const int key2_size,
			      const void *data2_p, const int data2_size)
{
  const double	*double1_p, *double2_p;
  const long	*long1_p, *long2_p;
  const char	*str1_p, *str2_p;
  int		result;
  
  /* if we aren't sorting by key then sort the count */
  if (! key_sort_b) {
    long1_p = data1_p;
    long2_p = data2_p;
    if (reverse_sort_b) {
      result = *long2_p - *long1_p;
    }
    else {
      result = *long1_p - *long2_p;
    }
    /* if the count is == then sort by key */
    if (result != 0) {
      return result;
    }
  }
  
  if (numbers_b) {
    /* reverse numeric sort */
    long1_p = key1_p;
    long2_p = key2_p;
    if (reverse_sort_b) {
      return *long2_p - *long1_p;
    }
    else {
      return *long1_p - *long2_p;
    }
  }
  else if (float_numbers_b) {
    /* reverse numeric sort */
    double1_p = key1_p;
    double2_p = key2_p;
    if (reverse_sort_b) {
      return *double2_p - *double1_p;
    }
    else {
      return *double1_p - *double2_p;
    }
  }
  else {
    /* forward string sort */
    str1_p = key1_p;
    str2_p = key2_p;
    if (reverse_sort_b) {
      return strcmp(str2_p, str1_p);
    }
    else {
      return strcmp(str1_p, str2_p);
    }
  }
}

int	main(int argc, char **argv)
{
  FILE		*infile;
  char		*filename, line[LINE_SIZE], *tok, *line_p;
  int		file_c, ret, field_c, key_size, entry_n;
  unsigned long	count, *count_p;
  long		value;
  double	double_value;
  void		*key_p;
  table_t	*tab;
  table_entry_t	**entries, **entries_p;
  
  argv_process(args, argc, argv);
  
  /* if we aren't showing the counts, we might as well sort by the key */
  if (no_counts_b) {
    key_sort_b = 1;
  }

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
  ret = table_set_data_alignment(tab, sizeof(long));
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "%s: could not set table alignment: %s\n",
		  argv_program, table_strerror(ret));
    exit(1);
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
      
      /* if blank line then maybe ignore it */
      if (line[0] == '\0' && ignore_blanks_b) {
	continue;
      }
      
      /* default is the entire line */
      tok = line;
      line_p = line;
      for (field_c = field - 1; field_c >= 0;) {
	tok = strsep(&line_p, delim_str);
	if (tok == NULL) {
	  break;
	}
	/*
	 * we only decrement the field counter if loose-fields is not
	 * on and we have an empty token
	 */
	if (! (loose_fields_b && *tok == '\0')) {
	  field_c--;
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
      else if (float_numbers_b) {
	double_value = atof(tok);
	key_p = &double_value;
	key_size = sizeof(double_value);
      }
      else {
	if (case_insens_b) {
	  /* lower case the string */
	  for (line_p = tok; *line_p != '\0'; line_p++) {
	    *line_p = tolower(*line_p);
	  }
	}
	key_p = tok;
	key_size = -1;
      }
      
      count = 1;
      ret = table_insert(tab, key_p, key_size, &count, sizeof(count),
			 (void **)&count_p, 0);
      if (ret != TABLE_ERROR_NONE) {
	if (ret != TABLE_ERROR_OVERWRITE) {
	  (void)fprintf(stderr, "%s: could not add key to table: %s\n",
			argv_program, table_strerror(ret));
	  exit(1);
	}
	
	(*count_p)++;
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
    if (numbers_b || float_numbers_b) {
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
    if (ret == TABLE_ERROR_EMPTY) {
      entry_n = 0;
    }
    else {
      (void)fprintf(stderr, "%s: could not order the table: %s\n",
		    argv_program, table_strerror(ret));
      exit(1);
    }
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
    
    if (! no_counts_b) {
      (void)printf("%10lu ", *count_p);
    }
    
    if (numbers_b) {
      (void)printf("%10ld\n", *(long *)key_p);
    }
    else if (float_numbers_b) {
      (void)printf("%10.2f\n", *(double *)key_p);
    }
    else {
      (void)printf("%.*s\n", key_size, (char *)key_p);
    }
  }
  
  if (entries != NULL) {
    (void)table_order_free(tab, entries, entry_n);
  }
  (void)table_free(tab);
  
  argv_cleanup(args);
  exit(0);
}
