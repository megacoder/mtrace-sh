/*
 *------------------------------------------------------------------------
 * vim: ts=8 sw=8
 *------------------------------------------------------------------------
 * Author:   tf135c (James Reynolds)
 * Filename: syscaller.c
 * Created:  2006-12-11 09:56:18
 *------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#if	HAVE_READLINE
# include <readline/readline.h>
# include <readline/history.h>
#else	/* !HAVE_READLINE */
# define HAVE_READLINE	0
#endif	/* !HAVE_READLINE */

#include <gcc-compat.h>

typedef	struct	{
	char *		spelling;
	int		(*func)( int, char * * );
	char *		doc;
} Command_t;

static	char *		me = "syscaller";
static	unsigned	nonfatal;
static	unsigned	debugLevel;

/*
 *------------------------------------------------------------------------
 * say: output printf-style message to stderr
 *------------------------------------------------------------------------
 */

static	void
vsay(
	char const * const	fmt,
	va_list			ap
)
{
	fprintf(  stderr, "%s: ", me );
	vfprintf( stderr, fmt, ap );
	fprintf(  stderr, ".\n" );
}

static	void _printf(1,2)
say(
	char const * const	fmt,
	...
)
{
	va_list		ap;

	va_start( ap, fmt );
	vsay( fmt, ap );
	va_end( ap );
}

/*
 *------------------------------------------------------------------------
 * debug: output debug message if sufficient demand
 *------------------------------------------------------------------------
 */

static	void			_printf(3,4)
debug(
	unsigned		theLevel,
	int			e,
	char const * const	fmt,
	...
)
{
	if( theLevel <= debugLevel )	{
		va_list		ap;

		fprintf( stderr, "%s ", me );
		va_start( ap, fmt );
		vfprintf( stderr, fmt, ap );
		va_end( ap );
		if( e )	{
			fprintf( stderr, "; errno=%d (%s)", e, strerror( e ) );
		}
		fprintf( stderr, ".\n" );
	}
}

/*
 *------------------------------------------------------------------------
 * usage: how to run this program
 *------------------------------------------------------------------------
 */

static	void _printf(1,2)
usage(
	char const * const	fmt,
	...
)
{
	static char const	opt_fmt[] = "  %-11s %s\n";

	if( fmt )	{
		va_list		ap;

		va_start( ap, fmt );
		vsay( fmt, ap );
		va_end( ap );
		printf( "\n" );
	}
	printf( "NAME\n" );
	printf( "  %s - FIXME\n", me );
	printf( "\n" );
	printf( "SYNOPSIS\n" );
	printf( "  %s", me );
	printf( " [-D]" );
	printf( "\n" );
	printf( "\n" );
	printf( "DESCRIPTION\n" );
	printf( "  The %s program is so neat, I can't begin to tell\n", me );
	printf( "  you just how wonderful it is.  You'll just have to\n" );
	printf( "  try it out for yourself.\n" );
	printf( "\n" );
	printf( "OPTIONS\n" );
	printf( opt_fmt, "-D",		"Increase debug output verbosity" );
	printf( "\n" );
}

/*
 */

static	int
cmd_exit(
	int		argc,
	char * *	argv
)
{
	int		status;

	status = 0;
	if( argc > 1 )	{
		status = atoi( argv[ 1 ] );
	}
	exit( status );
}

static	int		cmd_help( int, char * * );

static	Command_t	commands[] =	{
	{
		.spelling	= "exit",
		.func		= cmd_exit,
		.doc		= "exit [status]",
	},
	{
		.spelling	= "help",
		.func		= cmd_help,
		.doc		= "display help message",
	},
};
static	size_t		nCommands = DIM( commands );

static	int
cmd_help(
	int		argc,
	char * *	argv
)
{
	int		status;

	status = -1;
	do	{
		size_t		i;
		char *		verb;

		if( argc > 1 )	{
			verb = argv[ 1 ];
		} else	{
			verb = NULL;
		}
		for( i = 0; i < nCommands; ++i )	{
			Command_t * const	cmd = commands + i;

			if( verb )	{
				if( !strcmp( verb, cmd->spelling ) )	{
					printf( "%s\n", cmd->doc );
					status = 0;
				}
			} else	{
				printf( "%s\t%s\n", cmd->spelling, cmd->doc );
				status = 0;
			}
		}
	} while( 0 );
	return( status );
}

/*
 *------------------------------------------------------------------------
 * process: do whatever is necesssary
 *------------------------------------------------------------------------
 */

static	void
process(
	unsigned	argc,
	char * *	argv
)
{
	char * const	verb = argv[ 0 ];
	Command_t *	cmd;

	debug( 1, 0, "argc=%u", argc );
	for(
		cmd = commands;
		cmd < (commands + DIM( commands ));
		++cmd
	)	{
		if( !strcmp( verb, cmd->spelling ) )	{
			int	status;

			status = (*cmd->func)( argc, argv );
			if( status )	{
				say(
					"'%s' returned %d.\n",
					verb,
					status
				);
			}
			break;
		}
	}
	if( cmd >= (commands + DIM( commands )) )	{
		say( "unknown command '%s'", verb );
	}
}

#if	!HAVE_READLINE
/*
 *------------------------------------------------------------------------
 * readline: simulate GNU readline() system if we don't have it
 *------------------------------------------------------------------------
 */

static	char *
readline(
	char const *	prompt
)
{
	static size_t const	len = BUFSIZ + 1;
	char *		result;

	result = malloc( len );
	if( result )	{
		if( prompt && prompt[0] )	{
			printf( "FIXME %s", prompt );
			fflush( stdout );
		}
		if( !fgets( result, len, stdin ) )	{
			free( result );
			result = NULL;
		}
	}
	return( result );
}

void
add_history(
	char const * const	line	_unused
)
{
	/*NOTHING*/
}
#endif	/* !HAVE_READLINE */

/*
 *------------------------------------------------------------------------
 * main: central control logic
 *------------------------------------------------------------------------
 */

int
main(
	int		argc,
	char * *	argv
)
{
	do	{
		char *		bp;
		int		c;

		/* Figure out our process name				*/
		me = argv[ 0 ];
		if( (bp = strrchr( me, '/' )) != NULL )	{
			me = bp + 1;
		}
		/* Process any command line switches			*/
		opterr = 0;
		while( (c = getopt( argc, argv, "D" )) != EOF )	{
			switch( c )	{
			default:
				say( "switch -%c not implemented yet", c );
				++nonfatal;
				break;
			case '?':
				say( "unknown switch -%c", optopt );
				++nonfatal;
				break;
			case 'D':
				++debugLevel;
				break;
			}
		}
		/* Can't proceed if we've had bad switches		*/
		if( nonfatal )	{
			usage( "Illegal switch(es) used" );
			break;
		}
	} while( 0 );
	if( !nonfatal ) do	{
		/* Process command line arguments, if any		*/
		if( optind < argc )	{
			process( argc - optind, argv + optind );
		} else	{
			int const	interactive = isatty( fileno( stdin ) );
			char		prompt[ BUFSIZ + 1 ];
			size_t		needed;

			/* Build the prompt, even if we don't need it	*/
			needed = snprintf(
				prompt,
				sizeof( prompt ),
				"%s> ",
				me
			);
			prompt[ DIM( prompt ) - 1 ] = '\0';
			if( needed >= sizeof( prompt ) )	{
				say( "BUMMER! Prompt was truncated" );
				exit( 1 );
				/*NOTREACHED*/
			}
			/* Get an process each stdin line		*/
	   		while( !feof( stdin ) )	{
				static char const	comment = '#';
				char * const	line = readline(
					interactive ? prompt : NULL
				);
				char *		lp;
				int		my_argc;
				char * *	my_argv = alloca(
					sizeof( my_argv[0] ) *
					( 1 + ( line ? strlen( line ) : 0 ) )
				);
				
				/* Recognize EOF			*/
				if( !line )	{
					if( interactive )	{
						printf( "\n[EOF]\n" );
					}
					break;
				}
				/* Drop comments			*/
				lp = strchr( line, comment );
				if( lp )	{
					*lp = '\0';
				}
				/* Trim trailing whitespace		*/
				lp = line + strlen( line );
				while( (lp > line) && isspace( lp[-1] ) ) {
					*--lp = '\0';
				}
				/* Skip leading whitespace		*/
				for( lp = line; *lp && isspace( *lp ); ++lp );
				/* Process the line, if any remains	*/
				if( *lp )	{
					static char const	delims[] = {
						" \t\r\n"
					};
					char *	s;
					add_history( lp );
					/* Tokenize line		*/
					my_argc = 0;
					for(
						s = lp;
						(my_argv[ my_argc ] =
							strtok( s, delims ));
						s = NULL, ++my_argc
					);
					/* Do it			*/
					process( my_argc, my_argv );
				}
				/* Discard the line, we're done		*/
				free( line );
			}
		}
	} while( 0 );
	return( nonfatal ? 1 : 0 );
}
