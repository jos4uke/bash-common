#! /bin/bash


# Tests

testHashTableOutput()
{
    PIPELINE_DEFAULT_CONFIG="/projects/ARABIDOPSIS/SCRIPTS/PIPELINE/pipeline_default.config"
    declare -A PARAMETERS_TABLE
    get_pipeline_default_parameters $PIPELINE_DEFAULT_CONFIG
    for i in "${!PARAMETERS_TABLE[@]}"
    do
	echo -e "key: $i"
	echo -e "value ${PARAMETERS_TABLE[$i]}"
    done

    assertTrue 'Unexpected void Hash table' "[ ${#PARAMETERS_TABLE[@]} -ge 1 ]"
}


testPrintHashTable()
{
    PIPELINE_DEFAULT_CONFIG="/projects/ARABIDOPSIS/SCRIPTS/PIPELINE/pipeline_default.config"
    get_pipeline_default_parameters $PIPELINE_DEFAULT_CONFIG
}




# Configuration

oneTimeSetUp()
{
    . ../lib/pipeline_lib.inc
    OUTPUT_DIR="${SHUNIT_TMPDIR}/OUTPUT"
    mkdir $OUTPUT_DIR
}

TearDown()
{
    rm -rf $OUTPUTDIR
}

. /usr/local/share/shunit2-2.1.6/src/shunit2
