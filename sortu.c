/*
 * Sort/uniq program
 *
 * Copyright 1999 by Gray Watson
 *
 * $Id$
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "argv.h"
#include "table.h"

#define DEFAULT_DELIM	" "
#define LINE_SIZE	1024

/* struct for the order/count stuff */
typedef struct {
  unsigned long	so_count;			/* count that we have seen the item */
  int		so_order;			/* order that we get it for -o */
} sortu_t;

/* argument variables */
static	int		ignore_blanks_b = 0;	/* ignore blank lines */
static	int		cumulative_b = 0;	/* show cumulative numbers */
static	int		no_counts_b = 0;	/* don't output str counts */
static	char		*delim_str = DEFAULT_DELIM; /* field delim char */
static	int		field = -1;		/* field to use */
static	int		case_insens_b = 0;	/* case insensitive matches */
static	int		key_sort_b = 0;		/* sort by key not count */
static	int		loose_fields_b = 0;	/* loose field match */
static	int		min_matches = 0;	/* minimum number of matches */
static	int		max_matches = 0;	/* max number of matches */
static	int		numbers_b = 0;		/* fields are numbers */
static	int		numbers_float_b = 0;	/* fields are floats */
static	int		order_sort_b = 0;	/* keep order when sorting */
static	int		show_percentage_b = 0;	/* show percentage vals */
static	int		reverse_sort_b = 0;	/* reverse the sort order */
static	int		start_offset = 0;	/* field starts at this offset */
static	int		stop_offset = 0;	/* field stops at this offset */
static	int		verbose_b = 0;		/* verbose flag */
static	argv_array_t	files;			/* work files */

/* argument array */
static	argv_t	args[] = {
  { 'b',	"blank-ignore",	ARGV_BOOL_INT,		&ignore_blanks_b,
    NULL,		"ignore blank lines" },
  { 'c',	"cumulative-numbers", ARGV_BOOL_INT,	&cumulative_b,
    NULL,		"show percentage along with count" },
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
  { 'N',	"float-numbers", ARGV_BOOL_INT,		&numbers_float_b,
    NULL,		"treat field as floating point" },
  { 'o',	"order-sort",	ARGV_BOOL_INT,		&order_sort_b,
    NULL,		"output in order of discovery" },
  { 'p',	"percentage-show", ARGV_BOOL_INT,	&show_percentage_b,
    NULL,		"show percentage along with count" },
  { 'r',	"reverse-sort",	ARGV_BOOL_INT,		&reverse_sort_b,
    NULL,		"reverse the sort" },
  { 's',	"start-offset",	ARGV_INT,		&start_offset,
    "offset",		"field starts at offet" },
  { 'S',	"stop-offset",	ARGV_INT,		&stop_offset,
    "offset",		"field stops at offset" },
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
  const sortu_t	*sortu1_p = data1_p, *sortu2_p = data2_p;
  const double	*double1_p, *double2_p;
  const long	*long1_p, *long2_p;
  const char	*str1_p, *str2_p;
  int		result;
  
  if (order_sort_b) {
    /* the order will always be uniq */
    return sortu1_p->so_order - sortu2_p->so_order;
  }
  
  /* if we aren't sorting by key then sort the count */
  if (! key_sort_b) {
    if (reverse_sort_b) {
      result = sortu2_p->so_count - sortu1_p->so_count;
    }
    else {
      result = sortu1_p->so_count - sortu2_p->so_count;
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
  else if (numbers_float_b) {
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
  char		*filename, line[LINE_SIZE], *tok, *line_p, *line_bounds_p;
  int		file_c, ret, field_c, key_size, entry_n;
  unsigned long	total, subtotal, perc;
  long		value;
  double	double_value;
  void		*key_p;
  table_t	*tab;
  sortu_t	sortu, *sortu_p;
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
  
  /* initialize our sortu insert structure */
  sortu.so_count = 1;
  sortu.so_order = 0;
  
  file_c = 0;
  while (1) {
    
    /* process each of the files */
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
    
    if (numbers_b) {
      key_p = &value;
      key_size = sizeof(value);
    }
    else if (numbers_float_b) {
      key_p = &double_value;
      key_size = sizeof(double_value);
    }
    
    while (fgets(line, sizeof(line), infile) != NULL) {
      
      /* cut off the \n */
      for (line_bounds_p = line;
	   *line_bounds_p != '\n' && *line_bounds_p != '\0';
	   line_bounds_p++) {
      }
      if (stop_offset > 0 && line_bounds_p > line + stop_offset) {
	line_bounds_p = line + stop_offset;
      }
      *line_bounds_p = '\0';
      
      /* if blank line then maybe ignore it */
      if (line[0] == '\0' && ignore_blanks_b) {
	continue;
      }
      
      /* default is the entire line */
      tok = line;
      line_p = line;
      for (field_c = field - 1; field_c >= 0;) {
	
	/* find the correct field in the line if necessary */ 
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
      }
      else if (numbers_float_b) {
	double_value = atof(tok);
      }
      else {
	key_p = tok + start_offset;
	if (case_insens_b) {
	  /* lower case the string */
	  for (; line_p < line_bounds_p; line_p++) {
	    *line_p = tolower(*line_p);
	  }
	}
	key_size = line_bounds_p - (char *)key_p;
	/* check to make sure we have a field */
	if (key_size <= 0) {
	  continue;
	}
      }
      
      /* add it into the table */
      ret = table_insert(tab, key_p, key_size, &sortu, sizeof(sortu),
			 (void **)&sortu_p, 0);
      if (ret == TABLE_ERROR_NONE) {
	sortu.so_order++;
      }
      else {
	if (ret != TABLE_ERROR_OVERWRITE) {
	  (void)fprintf(stderr, "%s: could not add key to table: %s\n",
			argv_program, table_strerror(ret));
	  exit(1);
	}
	
	/* it exists already so add one to the count */
	sortu_p->so_count++;
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
    (void)printf("%10.10s", "Count:");
    if (cumulative_b) {
      (void)printf(" %10.10s", "Cumulate:");
    }
    if (show_percentage_b) {
      (void)printf(" %5.5s", "%:");
      if (cumulative_b) {
	(void)printf(" %5.5s", "C %:");
      }
    }
    if (numbers_b || numbers_float_b) {
      (void)printf(" %10.10s\n", "Data:");
    }
    else {
      (void)printf(" %-10.10s\n", "Data:");
    }
    (void)printf("----------");
    if (cumulative_b) {
      (void)printf(" ----------");
    }
    if (show_percentage_b) {
      (void)printf(" -----");
      if (cumulative_b) {
	(void)printf(" -----");
      }
    }
    (void)printf(" ----------\n");
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
  
  /* get the total */
  total = 0;
  for (entries_p = entries; entries_p < entries + entry_n; entries_p++) {
    /* get each entry to print */
    ret = table_entry(tab, *entries_p, (void **)&key_p, &key_size,
		      (void **)&sortu_p, NULL);
    if (ret != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "%s: could not get table entry: %s\n",
		    argv_program, table_strerror(ret));
      exit(1);
    }
    
    /* limit the matches if necessary */
    if (sortu_p->so_count < min_matches
	|| (max_matches > 0 && sortu_p->so_count > max_matches)) {
      continue;
    }
    
    total += sortu_p->so_count;
  }
  
  subtotal = 0;
  for (entries_p = entries; entries_p < entries + entry_n; entries_p++) {
    /* get each entry to print */
    ret = table_entry(tab, *entries_p, (void **)&key_p, &key_size,
		      (void **)&sortu_p, NULL);
    if (ret != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "%s: could not get table entry: %s\n",
		    argv_program, table_strerror(ret));
      exit(1);
    }
    
    /* limit the matches if necessary */
    if (sortu_p->so_count < min_matches
	|| (max_matches > 0 && sortu_p->so_count > max_matches)) {
      continue;
    }
    
    subtotal += sortu_p->so_count;
    
    if (! no_counts_b) {
      (void)printf("%10lu ", sortu_p->so_count);
      
      if (cumulative_b) {
	(void)printf("%10lu ", subtotal);
      }
    }
    
    if (show_percentage_b) {
      if (total > 1000000) {
	perc = sortu_p->so_count / (total / 100);
      }
      else {
	perc = sortu_p->so_count * 100 / total;
      }
      (void)printf("%4ld%% ", perc);
      
      if (cumulative_b) {
	if (total > 1000000) {
	  perc = subtotal / (total / 100);
	}
	else {
	  perc = subtotal * 100 / total;
	}
	(void)printf("%4ld%% ", perc);
      }
    }
    
    if (numbers_b) {
      (void)printf("%10ld\n", *(long *)key_p);
    }
    else if (numbers_float_b) {
      (void)printf("%10.2f\n", *(double *)key_p);
    }
    else {
      (void)printf("%.*s\n", key_size, (char *)key_p);
    }
  }
  
  if (verbose_b) {
    (void)printf("---------- ");
    if (cumulative_b) {
      (void)printf("---------- ");
    }
    if (show_percentage_b) {
      (void)printf("----- ");
      if (cumulative_b) {
	(void)printf("----- ");
      }
    }
    (void)printf("----------\n");
    (void)printf("%10ld ", total);
    if (cumulative_b) {
      (void)printf("%10ld ", subtotal);
    }
    if (show_percentage_b) {
      (void)printf("%5.5s ", "100%");
      if (cumulative_b) {
	(void)printf("%5.5s ", "100%");
      }
    }
    if (numbers_b || numbers_float_b) {
      (void)printf("%10.10s\n", "Total");
    }
    else {
      (void)printf("%-10.10s\n", "Total");
    }
  }
  
  if (entries != NULL) {
    (void)table_order_free(tab, entries, entry_n);
  }
  (void)table_free(tab);
  
  argv_cleanup(args);
  exit(0);
}
