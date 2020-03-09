Sortu Utility
=============

The sortu program is a replacement for the sort and uniq programs.  It is common for Unix script writers to
want to count how many separate patterns are in a file.  For example, if you have a list of addresses, you may
want to see how many are from each state.  So you cut out the state part, sort these, and then pass them
through uniq -c.  Sortu does all this for you in a fraction of the time.

Sortu uses a hash table and some decent line processing to provide this functionality. It has some basic field
and delimiter handling which should do most basic awk or cut features to separate out the field that you are
sorting on.

## Arguments

To list all of the available options, use 'sortu --usage'.

-b (--blank-ignore)
	Ignore blank lines.
-c (--cumulative-numbers)
	Show cumulative counts in the output.
-C (--no-counts)
	Don't output string counts.  Just show the unique lines.
-d chars (--delimiter)
	Use with -f to specify a specific field you want to cut out of
	each line.  Default is a space (" ").
-f number (--field)
	Use with -d to specify a field you want to cut out of each line.  So if you have a file with
        name,rank,serial-number then you can specify -f 2 with a -d , to cut out the 2nd field
	separated by comma (,) which will show you the unique ranks out of the file.
-F format (--format)
	Specify an output format.  You can use the following special strings
	which are replaced in the output.

	%k key or line
	%n number of times the key appeared in the file
	%l length of the key
	%p percentage of the total lines
	%c cumulative count
-k (--key-sort)
	Sort by key or line, not the count.
-l (--loose-fields)
	Ignores white space between fields.  Use with -d to get the 2nd	non-blank field.
-m number (--minimum-matches)
	Minimum number of matches to show.
-M number (--maximum-matches)
	Maximum number of matches to show.
-i (--insensitive-case)
	Perform case insensitive matches.
-n (--numbers)
	Treat the line or field as a signed long number.
-N (--float-numbers)
	Treat the line or field as a floating point number.
-o (--order-sort)
	Output in order of discovery, not in sorted order.
-p (--percentage-show)
	Show percentage along with counts.
-r (--reverse-sort)
	Reverse the sort order.
-s offset (--start-offset)
	Start the key/line at this offset (0 is first).
-S offset (--stop-offset)
	Stop the key/line at this offset (0 is first).
-v (--verbose)
	Verbose messages.
file(s)
	File(s) to process otherwise use standard-in.

## Repository

The newest versions of the program are available via:  https://github.com/j256/sortu

Enjoy, Gray Watson
