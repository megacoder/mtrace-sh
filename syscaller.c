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
#include <dlfcn.h>
#include <pthread.h>

#include <syscaller.h>

typedef	struct	{
	char *			name;
	pthread_spinlock_t	lock;
	unsigned long long	actual;
	unsigned long long	virtual;
} Statistic_t;

typedef enum	{
	Activity_open,
	Activity_close,
	Activity_read,
	Activity_write,
	Activity_last		/* Must be last				*/
} Activity_t;

static	pthread_spinlock_t	spinner;
static	Statistic_t		activities[ Activity_last ] =	{
	[Activity_open] =	{
		.name = "open"
	},
	[Activity_close] =	{
		.name = "close"
	},
	[Activity_read] =	{
		.name = "read"
	},
	[Activity_write] =	{
		.name = "write"
	}
};
static	Statistic_t *		Lactivity = activities + DIM( activities );

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
	static char	banner[] = "System calls wrapped.\n";
	size_t		n;
	ssize_t		qty;

	if( pthread_spin_init( &spinner, PTHREAD_PROCESS_PRIVATE ) )	{
		perror( __PRETTY_FUNCTION__ );
		exit( 1 );
	}
	qty = sizeof( banner ) - 1;
	n = write( fileno( stderr ), banner, qty );
	if( n != (size_t) qty )	{
		/* Hmmm */
	}
}

static	void				_destructor
death(
	void
)
{
	static char const		obit[] = "SysCall\tInvoked\tIssued\n";
	Statistic_t *			st;
	ssize_t				n;
	size_t				qty;

	qty = sizeof( obit ) - 1;
	n = write( fileno( stderr ), obit, qty );
	if( n != (ssize_t) qty )	{
		/* */
	}
	for( st = activities; st < Lactivity; ++st )	{
		printf(
			"%s\t%llu\t%llu\n",
			st->name,
			st->virtual,
			st->actual
		);
	}
}

static	void				_inline
get_lock(
	pthread_spinlock_t * const	lock
)
{
	if( unlikely( pthread_spin_lock( lock ) ) )	{
		perror( __PRETTY_FUNCTION__ );
		exit( 1 );
	}
}

static	void				_inline
put_lock(
	pthread_spinlock_t * const	lock
)
{
	if( unlikely( pthread_spin_unlock( lock ) ) )	{
		perror( __PRETTY_FUNCTION__ );
		exit( 1 );
	}
}

static	void				_inline
init_once(
	void * *	funcp,
	Activity_t	act
)
{
	Statistic_t * const	st = activities + act;

	get_lock( &spinner );
	if( unlikely( *funcp == NULL ) )	{
		*funcp = dlsym( RTLD_NEXT, st->name );
		if( pthread_spin_init( &st->lock, PTHREAD_PROCESS_PRIVATE ) ) {
			perror( st->name );
			exit( 1 );
		}
	}
	put_lock( &spinner );
}

static	void				_inline
another_virtual(
	Activity_t const	act
)
{
	Statistic_t * const	st = activities + act;

	get_lock( &st->lock );
	++(st->virtual);
	put_lock( &st->lock );
}

static	void				_inline
another_actual(
	Activity_t const	act
)
{
	Statistic_t * const	st = activities + act;

	get_lock( &st->lock );
	++(st->actual);
	put_lock( &st->lock );
}

/*
 *------------------------------------------------------------------------
 * open: open(2) wrapper
 *------------------------------------------------------------------------
 */

int
open(
	char const *		pathname,
	int			oflags,
	...
)
{
	static int		(*real_open)( char const *, int, ... );
	int			result;
	int			cflags;

	init_once( (void *) &real_open, Activity_open );
	if( unlikely( oflags & O_CREAT ) )	{
		va_list		ap;

		va_start( ap, oflags );
		cflags = va_arg( ap, int );
		va_end( ap );
	} else	{
		cflags = 0;
	}
	do	{
		another_actual( Activity_open );
		result = real_open( pathname, oflags, cflags );
	} while( result == EINTR );
	another_virtual( Activity_open );
	return( result );
}

int
close(
	int		fd
)
{
	static int	(*real_close)( int );
	int		result;

	init_once( (void *) &real_close, Activity_close );
	do	{
		another_actual( Activity_close );
		result = (*real_close)( fd );
	} while( result == EINTR );
	another_virtual( Activity_close );
	return( result );
}

ssize_t
read(
	int		fd,
	void *		buf,
	size_t		count
)
{
	static ssize_t	(*real_read)( int, void *, size_t );
	ssize_t		result;

	init_once( (void *) &real_read, Activity_read );
	do	{
		another_actual( Activity_read );
		result = (*real_read)( fd, buf, count );
	} while( result == (ssize_t) EINTR );
	another_virtual( Activity_read );
	return( result );
}

ssize_t
write(
	int		fd,
	void const *	buf,
	size_t		count
)
{
	static ssize_t	(*real_write)( int, void const *, size_t );
	ssize_t		result;

	init_once( (void *) &real_write, Activity_write );
	do	{
		another_actual( Activity_write );
		result = (*real_write)( fd, buf, count );
	} while( result == (ssize_t) EINTR );
	another_virtual( Activity_write );
	return( result );
}
