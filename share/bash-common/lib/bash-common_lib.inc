#! /bin/bash

#
# BASH COMMON FUNCTIONS LIBRARY
#

# Copyright 2012 Joseph Tran <Joseph.Tran@versailles.inra.fr>

# This software is a collection of utils functions and scripts in bash whose purpose is to:
# - perform configuration parsing and validation, filtering operations on sequence alignments, text manipulations, 
#   quality report parsing, filtering operations on genomes list with available sequences and indexes, etc.
# - the following code is a tests suite intended to test the library functions of this software.

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

# Date: 2012-09-21

local VERSION=v0.0.1

###
### DATA SEARCH UTILS
###

#-------------------
# in_array function
#
# Check if a value exists in an array
# @param $1 mixed  Needle  
# @param $2 array  Haystack
# @return  Success (0) if value exists, Failure (1) otherwise
# Usage: in_array "$needle" "${haystack[@]}"
# See: http://fvue.nl/wiki/Bash:_Check_if_array_element_exists
in_array() {
    local hay needle=$1
    shift
    for hay; do
        [[ "$hay" == "$needle" ]] && return 0
    done
    return 1
}


###
### TEXT MANIPULATIONS
###

#------------------
# toupper function
#
toupper()
{
    local char="$*"
    out=$(echo $char | tr [:lower:] [:upper:]) 
    local retval=$? 
    echo "$out" 
    unset out char 
    return $retval
}

#------------------
# tolower function
#
tolower()
{
    local char="$*"
    out=$(echo $char | tr [:upper:] [:lower:]) 
    local retval=$? 
    echo "$out" 
    unset out char 
    return $retval
}

###
### CONFIGURATION FILE PARSING AND VALIDATION
###

#------------------------------------------
# get_mutdetect_default_parameters function
# 
get_mutdetect_default_parameters()
{
    INFILE=$1
    #declare -A PARAMETERS_TABLE
    # for line in $(cat $INFILE); do
    # 	read -a p_array <<< $(echo -e $line | tr "=" "\n")
    # 	PARAMETERS_TABLE[p_array[1]]=p_array[2]
    # 	echo -e "$p_array[1] \t $p_array[2]"
    # done 
    
    let count=0
    rtrn=0

    while read line
    do
	let count++
	key=$(echo $line | awk -F"=" '{print $1}');
	rtrn=$?
	if [[ $rtrn -ne 0 ]]; then
	    echo -e "$(date '+%Y%m%d %r') [$0] Failed Error parsing parameter key at line $count." >&2
	    return $rtrn
	else
	    if [[ ${key} =~ "#" || -z ${key} ]]
	    then
		continue
	    else
		value=$(echo $line | awk -F"=" '{print $2;}')
		rtrn=$?
		if [[ $rtrn -ne 0 ]]; then
		    echo -e "$(date '+%Y%m%d %r') [$0] Failed Error parsing parameter value at line $count." >&2
		    return $rtrn
		else
		    PARAMETERS_TABLE[${key}]=${value}
	            #echo -e "$key\t$value"
	            #echo -e "$key\t${PARAMETERS_TABLE[${key}]}"
		fi
	    fi  
	fi
    done < $INFILE

# TODO Check parameters interval and parameters type, if problems exit
    return $?
}

#---------------------------------------
# get_mutdetect_user_parameters function
# 
get_mutdetect_user_parameters()
{
    INFILE=$1

    let count=0
    rtrn=0

    while read line
    do
	let count++
	key=$(echo $line | awk -F"=" '{print $1}');
	rtrn=$?
	if [[ $rtrn -ne 0 ]]; then
	    echo -e "$(date '+%Y%m%d %r') [$0] Failed Error parsing parameter key at line $count." >&2
	    return $rtrn
	else
	    if [[ ${key} =~ "#" || -z ${key} ]]
	    then
		continue
	    else
		value=$(echo $line | awk -F"=" '{print $2;}')
		rtrn=$?
		if [[ $rtrn -ne 0 ]]; then
		    echo -e "$(date '+%Y%m%d %r') [$0] Failed Error parsing parameter value at line $count." >&2
		    return $rtrn
		else
		    if [[ -n ${PARAMETERS_TABLE[${key}]} ]]; then 
			echo -e "$(date '+%Y%m%d %r') [$0] OK Override ${key} parameter default value: ${PARAMETERS_TABLE[${key}]} by user defined value: ${value}" >&2
		    fi
		    PARAMETERS_TABLE[${key}]=${value}
	            #echo -e "$key\t$value"
	            #echo -e "$key\t${PARAMETERS_TABLE[${key}]}"
		fi
	    fi  
	fi
    done < $INFILE

    if [[ $rtrn -eq 0 ]]; then
	return $?
	else
	return $rtrn
    fi
}



#--------------------------------
# check_params_validity function
#
check_params_validity()
{
    rtrn=0
    # check parameters value type: if error occurs return exit code 5
    for key in "${!PARAMETERS_TABLE[@]}"
    do
	value=${PARAMETERS_TABLE[${key}]}

	# msgs
	check_dir_name_msg="$(date '+%Y%m%d %r') [$0] Check $key directory name value existence"	
	check_format_name_msg="$(date '+%Y%m%d %r') [$0] Check $key format name value validity"
	check_path_value_msg="$(date '+%Y%m%d %r') [$0] Check $key path value existence"
	check_prefix_value_msg="$(date '+%Y%m%d %r') [$0] Check $key prefix value validity"
	check_value_type_msg="$(date '+%Y%m%d %r') [$0] Check $key value type validity"
	## warn & err msgs
	bool_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key param with value $value: boolean value is not valid, should be TRUE or 1, or FALSE or 0"
	dir_name_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key param with value $value: directory name value does not exist or is not a directory"
	exec_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key param with value $value: command value is not executable or does not exist"
	format_name_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key param with value $value: format name value is not valid"
	not_int_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key param value is not a integer"
	not_processed_warn_msg="$(date '+%Y%m%d %r') [$0] Warning Attention $key param with value $value will not be processed"
	path_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key param with value $value: path value does not exist or is empty"
	prefix_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key param with value $value: prefix value does not exist"
	value_not_allowed_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key param with value $value: value is not allowed"
	# ok msgs
	ok_msg="$(date '+%Y%m%d %r') [$0] OK $key param with value $value is valid"

	case $key in
	    "BWA_REFERENCE_GENOME_INDEX")
		echo -e $check_prefix_value_msg
		if [[ $(ls ${value}.* | wc -l) -gt 0 ]]; then
		    echo -e $ok_msg
		    else
		    echo -e $prefix_err_msg >&2
		    rtrn=5
		fi
		;;
	    "REFERENCE_GENOME_FASTA") 
		echo -e $check_path_value_msg
		if [[ -e $value && -s $value ]]; then
		    echo -e $ok_msg
		    else
		    echo -e $path_err_msg >&2
		    rtrn=5
		fi
		;;
	    "SNPEFF_Version") 
		echo -e $not_processed_warn_msg >&2
		;;
	    "SNPEFF_PATH") 
		echo -e $check_path_value_msg
		if [[ -e $value && -x $value ]]; then
		    echo -e $ok_msg
		    else
		    echo -e $exec_err_msg >&2
		    rtrn=5
		fi
		;;
	    "bypass_fastqc_failure_report_checking") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;	    
	    "bwa_aln_t") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "bwa_aln_n") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "bwa_aln_o") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "bwa_aln_R") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "bwa_sampe_n") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "bwa_sampe_N") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "nb_of_independent_event") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "microindel_size") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "multiple_hit_allowed") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "trimmo_leading_qual_min") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "trimmo_trailing_qual_min") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "trimmo_sliding_window_size") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "trimmo_sliding_window_qual") echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "trimmo_min_length") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		fi
		;;
	    "trimmo_thread_number") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		fi
		;;
	    "MAPQ_min") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "QUAL_ENCODING") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    if [[ $value -eq 33 || $value -eq 64 ]]; then
			echo -e $ok_msg
		    else
			echo -e $value_not_allowed_err_msg >&2
		    fi
		fi
		;;
	    "snpeff_data") 
		echo -e $check_dir_name_msg
		if [[ -d "${PARAMETERS_TABLE['SNPEFF_PATH']}/data/$value" ]]; then
		    echo -e $ok_msg
		    else
		    echo -e $dir_name_err_msg >&2
		    rtrn=5
		fi
		;;
	    "snpeff_inFile_format") 
		echo -e $check_format_name_msg
		if [[ $value -eq "VCF" ]]; then
		    echo -e $ok_msg
		else
		    echo -e $format_name_err_msg >&2
		    rtrn=5
		fi
		;;
	    "snpeff_outFile_format") 
		echo -e $check_format_name_msg
		if [[ $value -eq "VCF" ]]; then
		    echo -e $ok_msg
		else
		    echo -e $format_name_err_msg >&2
		    rtrn=5
		fi
		;;
	    "nb_of_independent_event_min") 
		echo -e $check_value_type_msg
		if !(declare -i  value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "nb_of_independent_event_max") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "microindel_size_min") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "microindel_size_max") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "samtools_view_b") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "samtools_view_S") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "samtools_mpileup_B") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "samtools_mpileup_u") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "samtools_mpileup_f") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "samtools_mpileup_Q") 
		echo -e $check_value_type_msg
		if !(declare -i value); then
		    echo -e $not_int_err_msg >&2
		    rtrn=5
		    else
		    echo -e $ok_msg
		fi
		;;
	    "bcftools_view_c") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "bcftools_view_v") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "bcftools_view_g") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "bgzip_f") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "bgzip_c") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "tabix_f") 
		echo -e $check_value_type_msg
		if [[ $(toupper $value) -eq "TRUE" || $(toupper $value) -eq "FALSE" || $value -eq 1 || $value -eq 0 ]]; then
		    echo -e $ok_msg
		else
		    echo -e $bool_err_msg >&2
		    rtrn=5
		fi
		;;
	    "tabix_p") 
		echo -e $check_format_name_msg
		tabix_formats=("gff" "bed" "sam" "vcf")
		if ($(in_array "$value" "${tabix_formats[@]}")); then
		    echo -e $ok_msg
		else
		    echo -e $format_name_err_msg >&2
		    rtrn=5
		fi
		;;
	    *) 
		echo -e $not_processed_warn_msg >&2
		;;
	esac
    done

    if [[ $rtrn -eq 0 ]]; then
	return $?
	else
	return $rtrn
    fi
}

#-----------------------------------------
# check_params_interval_validity function
#
check_params_interval_validity()
{
    rtrn=0
    # check parameters value interval: if error occurs return exit code 6
    for key in "${!PARAMETERS_TABLE[@]}"
    do
	value=${PARAMETERS_TABLE[${key}]}

	# msgs
	check_interval_msg="$(date '+%Y%m%d %r') [$0] Check $key value interval validity"
	ok_msg="$(date '+%Y%m%d %r') [$0] OK Checked $key value interval validity"
	out_of_interval_err_msg="$(date '+%Y%m%d %r') [$0] Failed Error $key value is out of allowed interval"

	case $key in
	    "nb_of_independent_event") 
		echo -e $check_interval_msg
		min=${PARAMETERS_TABLE["${key}_min"]}
		max=${PARAMETERS_TABLE["${key}_max"]}
		if [[ $value -ge $min && $value -le $max ]]; then
		    echo -e $ok_msg
		    else
		    echo -e $out_of_interval_err_msg >&2
		    rtrn=6
		fi
		;;
	    "microindel_size") 
		echo -e $check_interval_msg
		min=${PARAMETERS_TABLE["${key}_min"]}
		max=${PARAMETERS_TABLE["${key}_max"]}
		if [[ $value -ge $min && $value -le $max ]]; then
		    echo -e $ok_msg
		    else
		    echo -e $out_of_interval_err_msg >&2
		    rtrn=6
		fi
		;;
	esac
    done

    if [[ $rtrn -eq 0 ]]; then
	return $?
	else
	return $rtrn
    fi
}

#--------------------------------------------------------------
# Function: get_config_sections config_file
# Purpose : Print the list of config sections from config file
get_config_sections()
{
	typeset config_file=$1

	egrep "^\[" ${config_file} | awk -F '[][]' '
		NF==3 && $0 ~ /^\[.*\]/ { print $2 }
		' 2>&1
	rtrn=$?
	if [[ $rtrn -ne 0 ]]; then
		echo -e "$(date '+%Y%m%d %r') [$0] Failed Error parsing configuration sections in ${config_file}." >&2
		return $rtrn
	else
		return $rtrn
	fi
}

#-------------------------------------------------------------------------
# Function : format_config_params config_file config_section [config_prefix]
# Purpose  : Format variables (prefixed by config_prefix and corresponding config_section) from config_section in config file
format_config_params()
{
	typeset config_file=$1
	typeset config_section=$2
	typeset config_prefix=$3
	typeset config_params

	[[ -n $config_prefix ]] && config_prefix=$config_prefix"_"
	config_params=$(
	awk -F= -v Config="${config_section}" -v Prefix="${config_prefix}" '
		BEGIN {
			Config = toupper(Config);
			Prefix = toupper(Prefix);
			patternConfig = "\\[" Config "]";
		}
		toupper($0)  ~ patternConfig,(/^\[/ && toupper($0) !~ patternConfig)  {
			if (/^#/ || /^\[/ || NF <2) next;
			sub(/^[[:space:]]*/, "");
			sub(/[[:space:]]*=[[:space:]]/, "=");
			print Prefix Config"_"$0;
		} ' ${config_file} 2>&1)
	rtrn=$?

	if [[ $rtrn -ne 0 ]]; then
		echo -e "$(date '+%Y%m%d %r') [set_config_params] Failed Error parsing configuration parameters in ${config_file}." >&2
		return $rtrn
	else
		echo -e "${config_params[@]}"
	fi
}

###
### COMMAND LINE UTILS
###

#-------------------------------------------------------------------------
# Function : buildCommandLineOptions command_name [prefix]
# Purpose  : build the command line options for the given command_name
buildCommandLineOptions()
{
    typeset cmd_name=$1
	typeset config_prefix=$2

    # if command_name contains spaces, replace blanks by underscores
	# example: samtools view => samtools_view
	[[ $cmd_name =~ " " ]] && cmd_name=${cmd_name// /_}
    # if config_prefix not null, suffixed it with underscore    
    [[ -n $config_prefix ]] && config_prefix=$config_prefix"_"
    set | awk -F= -v cfg=${cmd_name} -v prefix=${config_prefix} '
        BEGIN {cfg = toupper(cfg); prefix = toupper(prefix); patternConfig="^" prefix cfg;} 
        toupper($0) ~ patternConfig  { 
            val=$2; split($1, a, "_"); opt=a[length(a)]; 
            if (length(opt) == 1) {
                if (toupper(val) ~ "TRUE") val=""; 
                opt_val="-"opt" "val;
            } 
            else if (toupper(val) ~ "TRUE") opt_val="--"opt; 
            else opt_val="--"opt"="val; 
            print opt_val}'
}

###
### PROCESS MANAGEMENT
###

#-------------------------------------------------------------------------
# Function : waitalluntiltimeout $pids_array
# Purpose  : wait for all child processes to finish and log their exit status until timeout, then kill all running out time child processes
waitalluntiltimeout() {
  ## Wait for children to exit and indicate whether all exited with 0 status.
  local errors=0
  ((t = WAITALL_TIMEOUT))
  while ((t > 0)); do
    debug "Processes remaining: $*"
    for pid in "$@"; do
      shift
      if kill -0 "$pid" 2>/dev/null; then
        debug "$pid is still alive."
        set -- "$@" "$pid"
      elif wait "$pid"; then
		status=$?
        debug "$pid exited with zero exit status ($status)."
      else
		status=$?
        debug "$pid exited with non-zero exit status ($status)."
        ((++errors))
      fi
    done
    (("$#" > 0)) || break
    # TODO: how to interrupt this sleep when a child terminates?
    sleep ${WAITALL_INTERVAL:-1}
	((t -= WAITALL_INTERVAL))
  done
  if [[ -n "$@" ]]; then
  for pid in "$@"; do
    debug "$pid is running out time limit: $WAITALL_TIMEOUT."
    debug "killing process $pid ..."
    kill -s SIGTERM $pid && kill -0 $pid
	sleep ${WAITALL_DELAY:-1}
    kill -s SIGKILL $pid
    debug "process $pid killed"
  done
  fi
  unset pid status
  ((errors == 0))
}

#-------------------------------------------------------------------------
# Function : degug $message_string
# Purpose  : print a debug message to the standard error
debug() { echo "$(date '+%Y_%m_%d %T') DEBUG: $*" >&2; }

###
### EXCEPTION MANAGEMENT
###

#-------------------------------------------------------------------------
# Function : exit_on_error err msg status log 
# Purpose  : check if current process has to exit when checking for exit status, if different of 0 exits with error msg send to log with err file reference for details
exit_on_error()
{
	err=$1
	msg=$2
	status=$3
	log=$4
	tag=$5
	recipient=$6
	if [[ "$status" -ne 0 ]]; then
		echo -e "$(date '+%Y_%m_%d %T') $msg" | tee -a $err 2>&1 | [[ -n $log ]] && tee -a $log 2>&1
		echo -e "$(date '+%Y_%m_%d %T') [Pipeline error] Exits the pipeline, with error code $status." | tee -a $err 2>&1 | [[ -n $log ]] && tee -a $log 2>&1
		echo -e "$(date '+%Y_%m_%d %T') [Pipeline error] Tail error output:" | [[ -n $log ]] && tee -a $log 2>&1
		echo -e "... $(tail -n 10 $err)" 2>&1 | [[ -n $log ]] && tee -a $log 2>&1
    	echo -e "$(date '+%Y_%m_%d %T') [Pipeline error] More details can be found in $err." | [[ -n $log ]] && tee -a $log 2>&1
		# send an email
		proc_error_msg="$tag process error: $msg\nExit status: $status\nMore details can be found in $(readlink -f $err)\nLast error output:\n... $(tail -n 10 $err)"
		[[ -n $recipient ]] && sendEmailBasic $recipient "[$(basename ${0%.*})] $tag process error" "$proc_error_msg" $err
		exit $status
	fi
}

#-------------------------------------------------------------------------
# Function : sendEmailBasic recipient subject message [files_to_attach]
# Purpose  : send an email
sendEmailBasic()
{
	recipient=$1
	subject=$2
	msg=$3
	local files=()
	shift 3
	[[ -n "$@" ]] && files=("$@")
	export EMAIL="Joseph.Tran@versailles.inra.fr"
	export REPLYTO="Joseph.Tran@versailles.inra.fr"
	export realname="Pipeline Administrator"

	echo -e "$msg" | mutt -e "set realname=\"$realname\"" -s "$subject" $([[ $(echo "$subject" | gawk '{print toupper($0)}') =~ "ERROR" ]] && echo -ne "-c $EMAIL ") $([[ -n $files ]] && echo -ne "-a ${files[@]}") -- $recipient
}

###
### QUALITY REPORT PARSING
###

#----------------------------------------------
# check_fastqc_quality_failure_report function
#
check_fastqc_quality_failure_report()
{
    fastqc_summary_file=$1

    out=$(grep -E "^FAIL[[:space:]]" $fastqc_summary_file)
    rtrn=$?
    if [[ -n $out ]]
	then
	echo "$out" >&2
	return $rtrn
	
	else
	return $rtrn
    fi
}

###
### GENOMES LIST FILTERING
###

#-------------------
# get_tool_version
#
get_tool_version()
{
	CMD_NAME=$1
	version=$( $CMD_NAME 2>&1 | grep Version | awk '{gsub(/\)||\(/,"",$3);if ($3) {print $2"-"$3} else {print $2}}')

    echo $version

}

#-------------------
# Get_Genomes_Dir_List
#
get_genomes_dir_list()
{
	genomes_path=$1

	files_list=($(ls $genomes_path))
	#echo "${files_list[@]}" 1>&2
	#echo "${#files_list[@]}" 1>&2
	dir_lists=()
	for f in "${files_list[@]}"; do
		#echo $f 1>&2
		if [[ -d $genomes_path/$f ]]
		then
			#echo $f 1>&2
			dir_lists=("${dir_lists[@]}" "$f")
		fi
	done
	#echo "${dir_lists[@]}" 1>&2
    echo "${dir_lists[@]}"
}

#------------------------------------
# Get_Genomes_Dir_List_With_One_Fasta
#
get_genomes_dir_list_with_one_fasta()
{
	genomes_path=$1

	dir_list=($(get_genomes_dir_list $genomes_path))

	genomes_list_fasta=()
	for d in "${dir_list[@]}"; do
		fasta_count=$(ls $genomes_path/$d | grep -e "$d\.m*fas*$" | wc -l)
		if [[  $fasta_count -eq 1 ]]
		then
			genomes_list_fasta=("${dir_list[@]}" "$d")
		elif [[ $fasta_count -eq 0 ]]
		then
			echo "[Warning] No corresponding fasta file exists for genome directory $d in $genomes_path." 1>&2
		elif [[ $fasta_count -ge 2 ]]
		then			
			echo "[Warning] More than 2 fasta file exists for genome directory $d in $genomes_path: ambiguous situation to be fixed." 1>&2
		fi
	done

	echo "${genomes_list_fasta[@]}"
}

#--------------------------------
# get_genomes_list_with_bwa_index
#
get_genomes_list_with_bwa_index()
{
	bwa_indexes_version=$1
	shift
	genomes_arr=($@)

	#echo "${genomes_arr[@]}" 1>&2

	bwa_index_ext=("amb" "ann" "bwt" "pac" "sa")
	genomes_bwa=()
	for g in "${genomes_arr[@]}"; do
		#echo "searching index files for $bwa_indexes_version/$g" 1>&2
		if [[ -d $bwa_indexes_version/$g ]]
		then
			for e in "${bwa_index_ext[@]}"; do
				#echo "searching index file with extension: .$e" 1>&2
				bwa_index_file=$bwa_indexes_version/$g/$g.$e
				if [[ -s $bwa_index_file ]]
				then
					#echo "found index file: $bwa_index_file" 1>&2
					continue
				else
					echo "[Error] Missing bwa index file: $bwa_index_file. Should be fixed." 1>&2
				fi
			done			
			genomes_bwa=("${genomes_bwa[@]}" "$g")
		fi
	done
	#echo "${genomes_bwa[@]}" 1>&2
	echo "${genomes_bwa[@]}"
}

#--------------------------------
# get_genomes_list_with_samtools_index
#
get_genomes_list_with_samtools_index()
{
	samtools_indexes_version=$1
	shift
	genomes_arr=($@)

	#echo "${genomes_arr[@]}" 1>&2

	samtools_index_ext=("fai")
	genomes_samtools=()
	for g in "${genomes_arr[@]}"; do
		#echo "searching index files for $samtools_indexes_version/$g" 1>&2
		if [[ -d $samtools_indexes_version/$g ]]
		then
			for e in "${samtools_index_ext[@]}"; do
				#echo "searching index file with extension: .$e" 1>&2
				samtools_index_file=$samtools_indexes_version/$g/$g.$e
				if [[ -s $samtools_index_file ]]
				then
					#echo "found index file: $samtools_index_file" 1>&2
					continue
				else
					echo "[Error] Missing samtools index file: $samtools_index_file. Should be fixed." 1>&2
				fi
			done			
			genomes_samtools=("${genomes_samtools[@]}" "$g")
		fi
	done
	#echo "${genomes_samtools[@]}" 1>&2
	echo "${genomes_samtools[@]}"
}

#------------------------------------
# get_genomes_list_with_snpeff_annot
#
get_genomes_list_with_snpeff_annot()
{
	snpeff_data=$1
	shift
	genomes_arr=($@)

	#echo "${genomes_arr[@]}" 1>&2

	snpeff_annot_files=("snpEffectPredictor.bin")

	genomes_snpeff=()
	for g in "${genomes_arr[@]}"; do
		#echo "searching snpeff annot files for $snpeff_data/$g" 1>&2
		if [[ -d $snpeff_data/$g ]]
		then
			for f in "${snpeff_annot_files[@]}"; do
				#echo "searching snpeff annot file: .$f" 1>&2
				snpeff_annot_file=$snpeff_data/$g/$f
				if [[ -s $snpeff_annot_file ]]
				then
					#echo "found snp annot file: $snpeff_annot_file" 1>&2
					continue
				else
					echo "[Error] Missing snpeff annot file: $snpeff_annot_file. Should be fixed." 1>&2
				fi
			done			
			genomes_snpeff=("${genomes_snpeff[@]}" "$g")
		fi
	done
	#echo "${genomes_snpeff[@]}" 1>&2
	echo "${genomes_snpeff[@]}"
}

###
### SAM SEQUENCES ALIGNMENTS FILTERING
###

#--------------------------------------
# dispatch_two_genomes_header function
#
dispatch_two_genomes_header()
{
	hdr_file=$1
	parent1=$1
	parent2=$2
	awk -vp1="$parent1" -vp2="$parent2" '{if ($0~p1) {print} else if ($0!~p2) {print}}' $hdr_file
}

#----------------------------------
# get_no_gapped_aln function
#
get_no_gapped_aln()
{
	sam_input=$1
	grep XO:i:[0][[:space:]] $sam_input
}

#----------------------------------
# get_no_mismatched_aln function
#
get_no_mismatched_aln()
{
	sam_input=$1
	grep XM:i:[0][[:space:]] $sam_input
}

#--------------------------------
# filter_on_mapq function
#
filter_on_mapq()
{
	sam_input=$1
	mapq_min=$2
	awk -v mapq_th="${mapq_min}" '{if($5>=mapq_th) print}' $sam_input
}

#-----------------------------
# get_unique_matches function
#
get_unique_matches()
{
	sam_input=$1
	grep -e 'X0:i:1[[:space:]]' $sam_input
}

#---------------------------
# get_mapped_reads function
#
get_mapped_reads()
{
	sam_input=$1

	samtools view -S -F 4 $sam_input

}

#------------------------------------
# get_mapped_reads_w_header function
#
get_mapped_reads_w_header()
{
	sam_input=$1

	samtools view -h -S -F 4 $sam_input

}

#---------------------------
# get_total_reads_count
#
get_total_reads_count()
{
	sam_input=$1
    samtools view -S -c $sam_input | tail -1
}

#-----------------------------------------------------------
# remove_reads_with_more_than_x_independent_events function
#
remove_reads_with_more_than_x_independent_events()
{
    LIMIT=$1
    INFILE=$2
    #old=$IFS
    #IFS=$'\n'
    #for line in $(cat $INFILE)
    #while read line
    #do
	#XM_XO=$(echo $line | perl -ne 'if ($_=~m/XM:i:(\d+).*XO:i:(\d+)/) {print $1+$2;}')
        #echo -e "XM+XO=$XM_XO" 1>&2

	# if [[ $XM_XO -le ${PARAMETERS_TABLE["nb_of_independent_event"]} ]]
	#     then
	#     echo $line
	# fi
	#echo $line | gawk -v limit=${PARAMETERS_TABLE["nb_of_independent_event"]} 'match($0, /XM:i:([0-9]+).*XO:i:([0-9]+)/, a) {if ((a[1]+a[2])<=limit) {print $0}}'
    #done < $INFILE
    #IFS=$old

    gawk -v limit="${LIMIT}" 'match($0, /XM:i:([0-9]+).*XO:i:([0-9]+)|XO:i:([0-9]+).*XM:i:([0-9]+)/, a) {if ((a[1]+a[2])<=limit) {print $0} else {print $0 > "/dev/stderr"}}' $INFILE # TODO add error catching to stderr
}

#-------------------------------------------------------------
# remove_reads_with_indels_size_greater_than_y_bases function
#
remove_reads_with_indels_size_greater_than_y_bases()
{
    LIMIT=$1
    INFILE=$2
    #while read line 
    #do
	# find a way to return all occurences of the pattern in CIGAR code
	# echo '10M5I5D8M3I7M' | gawk 'function findall(str, re) { while(match(str, re, a)) {sub("I", "", a[0]); print a[0]; str = substr(str, RSTART+RLENGTH)} } { findall($0, "([0-9]+)I"); }'
	#echo $line | 
    gawk -v limit=$LIMIT \
	    'function findAllIndelsSize(str, lim)
             {
                flag=0;
                while(match(str, /([0-9]+)[ID]/, a)) 
                {
                   sub("[ID]", "", a[0]);
                   if (a[0] <= lim)
                   {
                      flag=1;
                   } else
                   {
                      flag=0;
                      break;
                   }
                   str = substr(str, RSTART+RLENGTH)
                }
                if (flag == 1)
                {
                   print $0;
                }  else
                {
                   print $0 > "/dev/stderr"
                }
             }
             {
                if ($6 ~ "^[0-9]+M$")
                {
                   print $0;
                } else if ($6 ~ "[ID]")
                {
                   findAllIndelsSize($0, limit)
                } else # reads with soft clipping
                {
                   print $0; 
                }
             }' $INFILE # TODO add error catching to stderr
    #done < $INFILE
}

###
### REPORTING
###

#-----------------------------------------------------------
# write formatted file for trimming details rnw subsection
#
rnw_trimming_details_subsection()
{
    INFILE=$1
    OUTDIR=$2
    echo "Input read pairs" $(grep "Input Read" $INFILE |awk '{print "\t"$4*2}') >> $OUTDIR/trimming_details.txt
	echo "Both surviving" $(grep "Input Read" $INFILE | awk '{print "\t"$7}') >> $OUTDIR/trimming_details.txt
	echo "Forward only surviving" $(grep "Input Read" $INFILE | awk '{print "\t"$12}') >> $OUTDIR/trimming_details.txt
	echo "Reverse only surviving"  $(grep "Input Read" $INFILE | awk '{print "\t"$17}') >> $OUTDIR/trimming_details.txt
	echo "Read pairs dropped" $(grep "Input Read" $INFILE | awk '{print "\t"$20}') >> $OUTDIR/trimming_details.txt
}

#-----------------------------------------------------------
# write formatted file for alignment filtering rnw subsection
#
rnw_alignment_filtering_subsection()
{    
    INFILE=$1
    OUTDIR=$2
    echo "Number of reads after quality filtering" $(grep "Input Read" $INFILE | awk '{print "\t"$7*2}') >> $OUTDIR/alignment_filtering.txt
	echo "Number of mapped reads" $(grep "Filter aligned reads" $INFILE  | awk '{print "\t"$NF}') >> $OUTDIR/alignment_filtering.txt
	echo "Number of reads belonging to MAPQ threshold" $(grep "Filter MAPQ" $INFILE | awk '{print "\t"$NF}') >> $OUTDIR/alignment_filtering.txt
	echo "Number of reads with no more than X independent events" $(grep -E "Filter XOXM.*mapped" $INFILE | awk '{print "\t"$NF}') >> $OUTDIR/alignment_filtering.txt
	echo "Number of reads belonging to microindel size interval" $(grep -E "Filter indels.*mapped" $INFILE | awk '{print "\t"$NF}') >> $OUTDIR/alignment_filtering.txt
}
