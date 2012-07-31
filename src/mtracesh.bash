#!/bin/bash
ME="${0##*/}"		# Drop any path information
ME="${ME%%.*}"		# Drop any suffix
USAGE="usage: ${ME} [-o file] cmd [args..]"
OFILE="${MALLOC_TRACE}"
ULIMIT=
MTRACESH="${PWD}/libmtracesh.so"
while getopts o:u: c; do
	case "${c}" in
	o )	OFILE="${OPTARG}";;
	u )	ULIMIT="${OPTARG}";;
	* )	echo "${USAGE}" >&2; exit 1;;
	esac
done
shift $(( ${OPTIND} - 1 ))
if [ $# -lt 1 ]; then
	echo "${USAGE}" >&2
	exit 1
fi
if [ -z "${OFILE}" ]; then
	OFILE="mtrace.out"
fi
CMD="${1}"
shift
if [ ! -z "${ULIMIT}" ]; then
	ulimit -c "${ULIMIT}"
fi
# echo eval MALLOC_TRACE="${OFILE}" LD_PRELOAD="${MTRACESH}" "${CMD}" "${@}"
eval MALLOC_TRACE="${OFILE}" LD_PRELOAD="${MTRACESH}" "${CMD}" "${@}"
