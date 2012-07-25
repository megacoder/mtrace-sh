#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <gcc-compat.h>

int
main(
	int		argc,
	char * *	argv
)
{
	int		i;

	if( argc < 0 )	{
		argv[0] = argv[0];
	}
	printf( "Leaking memory at:\n" );
	for( i = 0; i < 10; ++i )	{
		size_t const	qty = 4096 * i;
		char * const	foo = malloc( qty );
		printf( "0x%016lx  %#8lx\n", (unsigned long) foo, (unsigned long) qty );
	}
	exit( 0 );
}
