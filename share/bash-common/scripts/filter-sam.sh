#! /bin/bash

#
# filter-sam.sh
#

# Copyright 2013 Joseph Tran <Joseph.Tran@versailles.inra.fr>

# This script provides options to filter sam file 

# This software is governed by the CeCILL license, Version 2.0 (the "License"), under French law and
# abiding by the rules of distribution of free software.  You can  use, 
# modify and/ or redistribute the software under the terms of the CeCILL
# license, Version 2.0 (the "License"), as circulated by CEA, CNRS and INRIA at the following URL
# "http://www.cecill.info/licences/Licence_CeCILL_V2-en.txt". 

# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability. 

# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or 
# data to be ensured and,  more generally, to use and operate it in the 
# same conditions as regards security. 

# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license, Version 2.0 (the "License"), and that you accept its terms.

# Date: 2013-12-07
VERSION=dev

### LOGGING CONFIGURATION ###

# load log4sh (disabling properties file warning) and clear the default
# configuration
LOG4SH_CONFIGURATION='none' . /usr/local/share/log4sh/build/log4sh 2>/dev/null
[[ $? != 0 ]] && $(echo "Error loading log4sh lib" >&2; exit 1)
log4sh_resetConfiguration

# set the global logging level to INFO
logger_setLevel FATAL

# add and configure a FileAppender that outputs to STDERR, and activate the
# configuration
logger_addAppender stderr
appender_setType stderr FileAppender
appender_file_setFile stderr STDERR
appender_setLevel stderr FATAL
appender_setLayout stderr PatternLayout
appender_setPattern stderr '%d{HH:mm:ss,SSS} %-4rs [%F:%-5p] %t - %m' 
appender_activateOptions stderr

### LOAD LIB ###

[[ $VERSION -eq "dev" ]] && PROG_PATH=$(realpath $(dirname $0));LIB_PATH=${PROG_PATH}/../lib/bash-common_lib.inc || LIB_PATH=/usr/local/share/bash-common/lib/bash-common_lib.inc

. $LIB_PATH 2>&1
if [[ $? != 0 ]]; then logger_fatal "Error loading bash common lib"; exit 1; fi

### FUNCTIONS ###
Usage()
{
printf %s "\
Program: $(basename $0)
Version: $VERSION

Copyright 2013 Joseph Tran <Joseph.Tran@versailles.inra.fr>

Licensed under the CeCILL License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.cecill.info/licences/Licence_CeCILL_V2-en.txt

Usage:  $(basename $0) [--nm_max max_mismatches] [--nh_max max_hits] [-v|--verbose | -d|--debug [--debugfile debugfile]] infile.sam|infile.bam

Options:
--nm_max max_mismatches                   Maximum number of mismatches [positive integer]. See NM tag in SAM format.
--nh_max max_hits                         Maximum number of hits [positive integer]. See NH tag in SAM format.
-h|--help                                 Displays this message.
-v|--verbose                              Whether to be verbose: blah,blah,blah.
-d|--debug                                Whether to switch to debug mode. Very verbose.
--debugfile debugfile                     Whether to use a debugfile in debug mode, else outputs to stderr.

Input:
infile.sam|infile.bam                     Input file in SAM or BAM format. 

Output:
infile_filtered-[\$Tag:\$Max_Value]*.sam  Output file in SAM format having given name pattern.

"
}

# NOTE: This requires GNU getopt.  On Mac OS X and FreeBSD, you have to install this
# separately; 
CONFIGURE_OPTS=`getopt -o hvd --long help,verbose,debug,debugfile:,nm_max:,nh_max:,\
	-n 'filter-sam.sh' -- "$@"`

if [[ $? != 0 ]] ; then Usage >&2 ; exit 1 ; fi

# Note the quotes around `$CONFIGURE_OPTS'
eval set -- "$CONFIGURE_OPTS"

STDOUT_APPENDER=false
VERBOSE=false
DEBUG=false
DEBUGFILE=
TAG_FILTERS=
while true; do
	case "$1" in
		-h | --help ) Usage >&2; exit 1;;
		-v | --verbose ) VERBOSE=true; shift ;;
		-d | --debug ) DEBUG=true; shift ;;
		--nm_max ) 
			NM_MAX="$2"; 
			if [[ $NM_MAX != [0-9] ]]; then
				logger_fatal "Max number of mismatches NM value must be a positive integer between 0 and 9."; exit 1;
			else
				logger_info "Max number of mismatches NM value set to $NM_MAX."
			fi
			[[ -n $TAG_FILTERS ]] && TAG_FILTERS=$TAG_FILTERS",NM:$NM_MAX" || TAG_FILTERS="NM:$NM_MAX" 
			shift 2 ;;
		--nh_max ) 
			NH_MAX="$2";
			if [[ $NH_MAX != [1-9] ]]; then
				logger_fatal "Max number of hits NH value must be a positive integer between 1 and 9.";	exit 1;
			else
				logger_info "Max number of hits NH value set to $NH_MAX."
			fi
			[[ -n $TAG_FILTERS ]] && TAG_FILTERS=$TAG_FILTERS",NH:$NH_MAX" || TAG_FILTERS="NH:$NH_MAX" 
			shift 2 ;;
		--debugfile ) DEBUGFILE="$2"; shift 2 ;;
		-- ) shift; break ;;
		* ) break ;;
	esac
done

if [[ $VERBOSE == true ]]; then
	logger_setLevel INFO
	logger_addAppender console
	appender_setType console ConsoleAppender
	appender_setLevel console INFO 
	appender_setLayout console PatternLayout
	appender_setPattern console '%d{HH:mm:ss,SSS} %-4rs [%F:%-5p] %t - %m' 
	appender_activateOptions console
	appender_exists console && logger_info "Verbose flag is set. Console appender is enabled." || logger_warn "Verbose flag set but console appender was not enabled. Maybe a log4sh error occured."
	appender_exists console && STDOUT_APPENDER=true
fi

if [[ $DEBUG == true ]]; then
	logger_setLevel DEBUG
	appender_exists console && appender_close console; logger_info "Debug flag is set. Close verbose appender."
	logger_addAppender debugger 
	appender_setType debugger ConsoleAppender
	appender_setLevel debugger DEBUG 
	appender_setLayout debugger PatternLayout
	appender_setPattern debugger '%d{HH:mm:ss,SSS} %-4rs [%F:%-5p] %t - %m' 
	appender_activateOptions debugger
	appender_exists debugger && logger_info "Debug flag is set. Debugger appender is enabled." || logger_warn "Debug flag set but debugger appender was not enabled. Maybe a log4sh error occured."
	appender_exists debugger && STDOUT_APPENDER=true
fi

if [[ $DEBUG == true ]]; then
	if [[ -n $DEBUGFILE ]]; then
		appender_exists debugger && appender_close debugger ; logger_info "Debug and debugfile flags are set. Close debugger appender."
		logger_addAppender debuggerF 
		appender_setType debuggerF FileAppender
		appender_file_setFile debuggerF $DEBUGFILE
		appender_setLevel debuggerF DEBUG 
		appender_setLayout debuggerF PatternLayout
		appender_setPattern debuggerF '%d{HH:mm:ss,SSS} %-4rs [%F:%-5p] %t - %m' 
		appender_activateOptions debuggerF
		appender_exists debuggerF && logger_info "Debug and debugfile flags are set. Debugging infos will be output to $DEBUGFILE file." || logger_warn "Debug and debugfile flags are set but debugger file appender was not enabled. Maybe a log4sh error occured." 
		appender_exists debuggerF && STDOUT_APPENDER=true 
	else
		dbg_msg="Debug file is empty string. See Usage with --help option."; appender_exists console && logger_warn "$dbg_msg" || echo "$dbg_msg"
	fi
else
	if [[ -n $DEBUGFILE ]]; then
		dbg_msg="Debug file is set but no debug flag set. No debugging infos will be output to file nor to console. See Usage with --help option."; appender_exists console && logger_warn "$dbg_msg" || echo "$dbg_msg" >&2
	else
		dbg_msg="Debug file is not set nor debug flag. No debugging infos will be output to file nor to console. See Usage with --help option."; appender_exists console && logger_debug "$dbg_msg" || echo "$dbg_msg" >&2
	fi	
fi

### FILTERS SAM ###

## filter functions
getMimeType()
{
	INFILE=$1
	file --mime-type -b $INFILE	
}
is_sam()
{
	INFILE=$1
	[[ $(getMimeType $INFILE) == "text/plain" ]] && return  0 || return 1; 
}
is_bam()
{
	INFILE=$1
	[[ $(getMimeType $INFILE) == "application/x-gzip" ]] && return 0 || return 1;
}
# slow
filter_tag()
{
	gawk -v filter="$1" 'BEGIN{split(filter,f,":"); regex=f[1]":i:([0-9]+)"} {match($0, regex, a); if (a[1]<=f[2]) {print $0}}' 
}
# 
filter_int_tag()
{
	gawk -v filter="$1" 'BEGIN{split(filter,f,":"); regex=f[1]":i:[0-"f[2]"]"} {if ($0 !~ regex) {next} print $0}'
}
# very very slow
filter_int_tags()
{
	gawk -v filters="$1" \
		'BEGIN{
			split(filters,fa,",");
			pattern=""
			for (i in fa)
				{
					split(fa[i],f,":");
					regex=f[1]":i:[0-"f[2]"]";
					ra[i]=regex
				}
		} 
		{
			for (i in ra) 
			{
				if ($0 !~ ra[i]) 
				{
					next
				} 
			}
			print $0
		}'
}
# $1=pattern1; $2=pattern2
grep_nh_nm_tags()
{
	grep -P "${1}.*${2}|${2}.*${1}"
}
#
grep_int_tags()
{
	IFS=',' read -ra TARR <<< "$TAG_FILTERS"
	CMD="grep"
	for i in "${TAG_FILTERS[@]}"; do
		IFS=":" read -ra MARR <<< "$i"
		CMD=$CMD" -e \"${MARR[0]}:i:[0-${MARR[1]}]\""
	done
	eval "$CMD"
}
# time: 30min for 
apply_nm_mh_tag_filters()
{
	if [[ -n $NM_MAX ]]; then filter_tag "NM:$NM_MAX"; fi | if [[ -n $NH_MAX ]]; then filter_tag "NH:$NH_MAX"; fi
}
#
apply_inm_nh_tag_filters2()
{
	if [[ -n $NM_MAX ]]; then filter_int_tag "NM:$NM_MAX"; fi | if [[ -n $NH_MAX ]]; then filter_int_tag "NH:$NH_MAX"; fi
}
# time : 3h for 69780682 alignments => too slow
apply_int_tag_filters()
{
	if [[ -n $TAG_FILTERS ]]; then filter_int_tags $TAG_FILTERS; fi
}

# Filter Patterns
NMP="NM:i:[0-"$NM_MAX"][[:space:]]" 
NHP="NH:i:[1-"$NH_MAX"][[:space:]]"

#filter from file
	# process file passed in argument $1
	## check if file exists and is not empty
	infile=$1
	if [[ -s $infile ]]; then logger_info "Input file '$infile' exists."; else logger_fatal "Input file '$infile' does not exist."; Usage; exit 1; fi
	## identify sam/bam input
	is_sam $infile; rtns=$?
	[[ $rtns == 0 ]] && logger_info "Input file '$infile' is sam file." || logger_info "Input file '$infile' is not a sam file."
	is_bam $infile; rtnb=$?
	[[ $rtnb == 0 ]] && logger_info "Input file '$infile' is bam file." || logger_info "Input file '$infile' is not a bam file."
	if [[ $rtns == 1 && $rtnb == 1 ]]; then logger_fatal "Input file '$infile' is not a sam file nor a bam file. See Usage with --help option."; exit 1; fi 

	## filter sam/bam file
	outpath=$(realpath ${infile})
	outfile=${outpath%.*}_filtered
	[[ -n $NM_MAX ]] && outfile=${outfile}-NM_MAX_${NM_MAX}
	[[ -n $NH_MAX ]] && outfile=${outfile}-NH_MAX_${NH_MAX}
	logger_info "Set output filename to $outfile."
	
	logger_info "Extract header from '$infile'."
	[[ $rtns == 0 ]] && IS_SAM="-S"
	samtools view -H $IS_SAM $infile 2>${outfile}.err.tmp >${outfile}.sam.tmp 
	if [[ -s ${outfile}.err.tmp ]]; then logger_warn "Some warnings occured during sam header extraction step."; logger_debug "Warnings: $(echo; cat ${outfile}.err.tmp)"; else logger_info "Sam header extracted successfully."; fi
	if [[ -s ${outfile}.sam.tmp ]]; then 
		logger_info "Sam header infos: $(wc -l ${outfile}.sam.tmp)"; 
	else
		logger_error "Extracted sam header does not exist or is empty.";
		exit 1;
	fi
	
	logger_info "Apply tags filtering to '$infile'. Tags filters list: '$TAG_FILTERS'."
	[[ $rtns == 0 ]] && IS_SAM="-S"
	samtools view $IS_SAM $infile 2>${outfile}.err.tmp | grep_nh_nm_tags $NMP $NHP >>${outfile}.sam.tmp 2>>${outfile}.err.tmp
	if [[ -s ${outfile}.err.tmp ]]; then logger_warn "Some warnings occured during tag filtering steps."; logger_debug "Warnings: $(echo;cat ${outfile}.err.tmp)"; else logger_info "Tags filtering applied successfully."; fi
		
	logger_info "Rename filtered tmp sam file."
	if [[ -s ${outfile}.sam.tmp ]]; then
		mv ${outfile}.sam.tmp ${outfile}.sam 2>${outfile}.err.tmp
	else
		logger_error "Filtered sam file '${outfile}.sam.tmp' does not exist or is empty. Did you have set some filtering options. See Usage with --help option."; 
		exit 1;
	fi
	if [[ -s ${outfile}.err.tmp ]]; then logger_warn "Some warnings occured while renaming temp filtered sam file."; logger_debug "Warnings: $(echo; cat ${outfile}.err.tmp)"; else logger_info "Rename temp filtered sam file successfully."; fi

# clean
logger_debug "Clean program report and tmp output files."
## iterate over all tmp files and then remove each of them
rm ${outfile}*.tmp 2>${outfile}.err
if [[ -s ${outfile}.err ]]; then logger_warn "An error occured during tmp intermediate files removal."; logger_debug "Warnings: $(echo; cat ${outfile}.err.tmp)"; else logger_debug "Temp intermediate output files removed successfully."; fi
if [[ -e ${outfile}.err ]]; then rm ${outfile}.err; logger_debug "Remove ${outfile}.err successfully."; fi
logger_debug "Cleaning successfully."

# end program
end_msg="$(basename $0) program ends successfully."
end_program()
{
	msg="$1"
	logger_info "$msg"
}
[[ $STDOUT_APPENDER == false ]] && echo "$end_msg" || end_program "$end_msg" 2>&1

# close all appenders
appender_exists stderr && appender_close stderr
appender_exists console && appender_close console
appender_exists debugger && appender_close debugger
appender_exists debuggerF && appender_close debuggerF
