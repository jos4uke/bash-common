#! /bin/bash

#
# PIPELINE TESTS SUITE
#

#---------------------
# testHashTableOutput 
#
testHashTableOutput()
{
    declare -A PARAMETERS_TABLE
    get_pipeline_default_parameters $PIPELINE_DEFAULT_CONFIG
    for i in "${!PARAMETERS_TABLE[@]}"
    do
	echo -e "key: $i"
	echo -e "value ${PARAMETERS_TABLE[$i]}"
    done

    assertTrue 'Unexpected void Hash table' "[ ${#PARAMETERS_TABLE[@]} -ge 1 ]"
}

#--------------------
# testPrintHashTable 
#
testPrintHashTable()
{
    declare -A PARAMETERS_TABLE
    get_pipeline_default_parameters $PIPELINE_DEFAULT_CONFIG
}

#---------------------------------------
# testFailedEquivalentXMXOtagsLineCount 
#
testFailedEquivalentXMXOtagsLineCount()
{
    XMtags=$(cat "${TEST_SAM}" | grep "XM:i:" | wc -l)
    XOtags=$(cat "${TEST_SAM}" | grep "XO:i:" | wc -l)

    echo -e "total reads count: $(cat $TEST_SAM | wc -l)"
    echo -e "XM tags count: ${XMtags}"
    echo -e "XO tags count: ${XOtags}"

    assertTrue 'unexpected XM and XO tags line counts difference, should be equal' "[ ${XMtags} -eq ${XOtags} ]"
}

#-------------------------------------------------------
# testFailedRemovingReadsWithMoreThanXIndependentEvents
#
testFailedRemovingReadsWithMoreThanXIndependentEvents()
{
    declare -A PARAMETERS_TABLE
    get_pipeline_default_parameters $PIPELINE_DEFAULT_CONFIG

    OUT=${TEST_OUTPUT_DIR}/test_mapped_MAPQ_XIE.sam
    remove_reads_with_more_than_x_independent_events "${TEST_SAM}" >${OUT} 2>${stderrF}

    echo -e "std out output:"
    head -4 "${OUT}"
    echo -e "std err output:" 
    head -4 "${stderrF}"

    assertTrue 'expected output to stdout' "[ -s ${OUT} ]"
    assertFalse 'unexpected output to stderr' "[ -s ${stderrF} ]"

    if [[ ${PARAMETERS_TABLE["nb_of_independent_event"]} -eq 2 ]]; then
    assertTrue "unexpected count for XM=2 && XO=0, should be equal $(cat ${TEST_SAM} | grep XM:i:[2][[:space:]] | grep XO:i:[0][[:space:]] | wc -l) " "[ $(cat ${OUT} | grep XM:i:[2] | grep XO:i:[0] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[2][[:space:]] | grep XO:i:[0][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=1 && XO=1, should be equal $(cat ${TEST_SAM} | grep XM:i:[1][[:space:]] | grep XO:i:[1][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[1] | grep XO:i:[1] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[1][[:space:]] | grep XO:i:[1][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=1 && XO=0, should be equal $(cat ${TEST_SAM} | grep XM:i:[1][[:space:]] | grep XO:i:[0][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[1] | grep XO:i:[0] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[1][[:space:]] | grep XO:i:[0][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=0 && XO=1, should be equal $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[1][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[0] | grep XO:i:[1] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[1][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=0 && XO=0, should be equal $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[0][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[0] | grep XO:i:[0] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[0][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=0 && XO=2, should be equal $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[2][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[0] | grep XO:i:[2] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[2][[:space:]] | wc -l) ]"
    fi
}


#
# Configuration
#

oneTimeSetUp()
{
    . ../lib/pipeline_lib.inc
    OUTPUT_DIR="${SHUNIT_TMPDIR}/OUTPUT"
    stdoutF="${OUTPUT_DIR}/stdoutF"
    stderrF="${OUTPUT_DIR}/stderrF"
    mkdir $OUTPUT_DIR
    TEST_OUTPUT_DIR="output"

    PIPELINE_DEFAULT_CONFIG="/projects/ARABIDOPSIS/SCRIPTS/PIPELINE/pipeline_default.config"
    TEST_SAM="data/test_mapped_MAPQ.sam"
}

tearDown()
{
    rm -rf $OUTPUTDIR
    echo "------------"
}

. /usr/local/share/shunit2-2.1.6/src/shunit2
