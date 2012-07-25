/*
 *------------------------------------------------------------------------
 * vim: ts=8 sw=8
 *------------------------------------------------------------------------
 * Author:   tf135c (James Reynolds)
 * Filename: syscaller.c
 * Created:  2006-12-11 09:59:27
 *------------------------------------------------------------------------
 */

#define	_GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <mcheck.h>
#include <pthread.h>
#include <string.h>

#include <syscaller.h>

static	char const	name[] = "MALLOC_TRACE";
static	char *		where;
static	int		is_set;

/*
 *------------------------------------------------------------------------
 * syscaller: static constructor used to announce our presence
 *------------------------------------------------------------------------
 */

static void				_constructor
birth(
	void
)
{

	where = getenv( name );
	is_set = (where != NULL);
	if( !is_set )	{
		where = "mtrace.out";
		is_set = (setenv( name, where, 1 ) != -1);
	}
	if( is_set )	{
		mtrace();
	}
	if( is_set )	{
		static char	banner[] = "Memory tracing enabled.\n";
		size_t const	qty = sizeof( banner )-1;

		if( write( fileno( stderr ), banner, qty ) != (ssize_t) qty )	{
			/* Hmmm */
		}
	}
}

static	void				_destructor
death(
	void
)
{
	if( is_set )	{
		static char const	msg[] = "Hope your memory is good: ";
		size_t const		qty = sizeof(msg) - 1;

		muntrace();
		if( write( fileno( stderr ), msg, qty ) != (ssize_t) qty )	{
			/* Hm */
		}
		if( write( fileno( stderr ), where, strlen(where) ) == -1 ) {
			/* Hm */
		}
		if( write( fileno( stderr ), "\n", 1 ) == -1 )	{
			/* Hm */
		}
		abort();
	}
}
