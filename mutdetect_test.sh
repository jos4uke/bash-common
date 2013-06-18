#! /bin/bash

#
# PIPELINE TESTS SUITE
#

#-------------------
# testFailedInArray 
#
testFailedInArray()
{
    tabix_formats=("gff" "bed" "sam" "vcf")
    if ($(in_array "$value" "${tabix_formats[@]}")); then
	echo -e "ok" >${stdoutF}
    else
	echo -e "failed" >${stderrF}
    fi

    assertTrue 'Expected output to stdout' "[ -s ${stdoutF} ]"
    assertFalse 'Unexpected output to stderr' "[ -s ${stderrF} ]"
    echo -e "stderr output:"
    cat ${stderrF}
}

#--------------------------------------
# testFailedLoadingDefaultConfigParams
#
testFailedLoadingDefaultConfigParams()
{
    declare -A PARAMETERS_TABLE
    get_mutdetect_default_parameters $PIPELINE_DEFAULT_CONFIG 2>${stderrF}
    for i in "${!PARAMETERS_TABLE[@]}"
    do
	echo -e "key:   $i"
	echo -e "value: ${PARAMETERS_TABLE[$i]}"
    done

    assertTrue 'Unexpected void Hash table' "[ ${#PARAMETERS_TABLE[@]} -ge 1 ]"
    assertFalse 'Unexpected output to stderr' "[ -s ${stderrF} ]"
}

#-----------------------------------
# testFailedLoadingUserConfigParams 
#
testFailedLoadingUserConfigParams()
{
    declare -A PARAMETERS_TABLE
    get_mutdetect_default_parameters $PIPELINE_DEFAULT_CONFIG 2>${stderrF}
    rtrn=$?
    assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
    assertFalse 'Unexpected output to stderr' "[ -s ${stderrF} ]"

    get_mutdetect_user_parameters $PIPELINE_USER_CONFIG 2>${stderrF}
    rtrn=$?
    assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
    assertTrue 'Expected output to stderr' "[ -s ${stderrF} ]"
    echo -e "stderr output:"
    cat ${stderrF}
}

#-----------------------------------------------
# testFailedDefaultCheckingConfigParamsValidity
#
testFailedDefaultCheckingConfigParamsValidity()
{
    declare -A PARAMETERS_TABLE
    get_mutdetect_default_parameters $PIPELINE_DEFAULT_CONFIG 2>${stderrF} 
    rtrn=$?
    assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
    assertFalse 'Unexpected output to stderr' "[ -s ${stderrF} ]"
    
    check_params_validity 2>${stderrF}
    rtrn=$?
    echo -e "exit status code: $rtrn"
    assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
    assertTrue 'Expected output to stderr' "[ -s ${stderrF} ]"
    echo -e "stderr output:"
    cat ${stderrF}
}

#-------------------------------------------------------
# testFailedDefaultCheckingConfigParamsIntervalValidity
#
testFailedDefaultCheckingConfigParamsIntervalValidity()
{
    declare -A PARAMETERS_TABLE
    get_mutdetect_default_parameters $PIPELINE_DEFAULT_CONFIG 2>${stderrF} 
    rtrn=$?
    assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
    assertFalse 'Unexpected output to stderr' "[ -s ${stderrF} ]"
    
    check_params_validity 2>${stderrF}
    rtrn=$?
    echo -e "exit status code: $rtrn"
    assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
    assertTrue 'Expected output to stderr' "[ -s ${stderrF} ]"

    check_params_interval_validity 2>${stderrF}
    rtrn=$?
    echo -e "exit status code: $rtrn"
    assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
    assertFalse 'Unexpected output to stderr' "[ -s ${stderrF} ]"
    echo -e "stderr output:"
    cat ${stderrF}
}

#--------------------------------------
# testFailedFastqcQualityFailureReport
#
testFailedFastqcQualityFailureReport()
{
    declare -A PARAMETERS_TABLE
    get_mutdetect_default_parameters $PIPELINE_DEFAULT_CONFIG
    check_params_validity
    check_params_interval_validity

    if [[ ${PARAMETERS_TABLE["bypass_fastqc_failure_report_checking"]} == "FALSE" ]]
	then
	check_fastqc_quality_failure_report ${TEST_FASTQC_1} 2>${stderrF}
	rtrn=$?
	echo -e "exit status code: $rtrn"
	assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
	assertFalse 'Unexpected output to stdout' "[ -s ${stdoutF} ]"
	echo -e "stdout output:"
	cat ${stdoutF}	
	assertTrue 'Expected output to stderr' "[ -s ${stderrF} ]"
	echo -e "stderr output:"
	cat ${stderrF}

	check_fastqc_quality_failure_report ${TEST_FASTQC_2} 2>${stderrF}
	rtrn=$?
	echo -e "exit status code: $rtrn"
	assertTrue 'Unexpected exit status code, should be equal to 0' "[ $rtrn -eq 0 ]"
	assertFalse 'Unexpected output to stdout' "[ -s ${stdoutF} ]"
	echo -e "stdout output:"
	cat ${stdoutF}
	assertTrue 'Expected output to stderr' "[ -s ${stderrF} ]"
	echo -e "stderr output:"
	cat ${stderrF}	
    fi
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
    get_mutdetect_default_parameters $PIPELINE_DEFAULT_CONFIG

    OUT=${TEST_OUTPUT_DIR}/test_mapped_MAPQ_XIE.sam
    time remove_reads_with_more_than_x_independent_events ${PARAMETERS_TABLE["nb_of_independent_event"]} "${TEST_SAM}" >${OUT} 2>${stderrF}

    echo -e "std out output:"
    head -4 "${OUT}"
    echo -e "std err output:" 
    head -4 "${stderrF}"

    assertTrue 'expected output to stdout' "[ -s ${OUT} ]"
    assertTrue 'expected output to stderr' "[ -s ${stderrF} ]"

    if [[ ${PARAMETERS_TABLE["nb_of_independent_event"]} -eq 2 ]]; then
    assertTrue "unexpected count for XM=2 && XO=0, should be equal $(cat ${TEST_SAM} | grep XM:i:[2][[:space:]] | grep XO:i:[0][[:space:]] | wc -l) " "[ $(cat ${OUT} | grep XM:i:[2] | grep XO:i:[0] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[2][[:space:]] | grep XO:i:[0][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=1 && XO=1, should be equal $(cat ${TEST_SAM} | grep XM:i:[1][[:space:]] | grep XO:i:[1][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[1] | grep XO:i:[1] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[1][[:space:]] | grep XO:i:[1][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=1 && XO=0, should be equal $(cat ${TEST_SAM} | grep XM:i:[1][[:space:]] | grep XO:i:[0][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[1] | grep XO:i:[0] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[1][[:space:]] | grep XO:i:[0][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=0 && XO=1, should be equal $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[1][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[0] | grep XO:i:[1] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[1][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=0 && XO=0, should be equal $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[0][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[0] | grep XO:i:[0] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[0][[:space:]] | wc -l) ]"
    assertTrue "unexpected count for XM=0 && XO=2, should be equal $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[2][[:space:]] | wc -l)" "[ $(cat ${OUT} | grep XM:i:[0] | grep XO:i:[2] | wc -l) -eq $(cat ${TEST_SAM} | grep XM:i:[0][[:space:]] | grep XO:i:[2][[:space:]] | wc -l) ]"
    fi
}


#--------------------------
# testFailedRemovingIndels
#
testFailedRemovingIndels()
{
    declare -A PARAMETERS_TABLE
    get_mutdetect_default_parameters $PIPELINE_DEFAULT_CONFIG

    IN=${TEST_OUTPUT_DIR}/test_mapped_MAPQ_XIE.sam
    OUT=${TEST_OUTPUT_DIR}/test_mapped_MAPQ_XIE_YID.sam

    echo -e "removing reads with indels size greater than ${PARAMETERS_TABLE['microindel_size']} bases from $(wc -l ${IN} | tr ' ' " (")) reads..."
    
    time remove_reads_with_indels_size_greater_than_y_bases ${PARAMETERS_TABLE["microindel_size"]} ${IN} >${OUT} 2>${stderrF}

    echo -e "std out output:"
    head -4 "${OUT}"
    echo -e "std err output:" 
    head -4 "${stderrF}"

    assertTrue 'expected output to stdout' "[ -s ${OUT} ]"
    assertTrue 'expected output to stderr' "[ -s ${stderrF} ]"
    assertTrue 'unexpected inequality in total reads count, input should be greater than output' "[ $(cat ${IN} | wc -l) -gt $(cat ${OUT} | wc -l) ]"

    if [[ ${PARAMETERS_TABLE["microindel_size"]} -eq 5 ]]; then
	assertTrue "unexpected difference in reads count with no indels, should be equal in input and output files: $(cat ${IN} | cut -d' ' -f6 | grep -v '[ID]' | wc -l)" "[ $(cat ${IN} | cut -d' ' -f6 | grep -v '[ID]' | wc -l) -eq $(cat ${OUT} | cut -d' ' -f6 | grep -v '[ID]' | wc -l) ]"
	assertTrue "unexpected difference in reads count with no indels and no soft clipping, should be equal in input and output files:  $(cat ${IN} | cut -d' ' -f6 | grep -v '[ID]' | grep -v '[S]' | wc -l)" "[ $(cat ${IN} | cut -d' ' -f6 | grep -v '[ID]' | grep -v '[S]' | wc -l) -eq $(cat ${OUT} | cut -d' ' -f6 | grep -v '[ID]' | grep -v '[S]' | wc -l) ]"
	assertTrue "unexpected difference in reads count with no indels but having soft clipping, should be equal in input and output files: $(cat ${IN} | cut -d' ' -f6 | grep -v '[ID]' | grep '[S]' | wc -l)" "[ $(cat ${IN} | cut -d' ' -f6 | grep -v '[ID]' | grep '[S]' | wc -l) -eq $(cat ${OUT} | cut -d' ' -f6 | grep -v '[ID]' | grep '[S]' | wc -l) ]"
	assertFalse "unexpected equivalence in reads count with indels, should be different in input and output files" "[ $(cat ${IN} | cut -d' ' -f6 | grep '[ID]' | wc -l) -eq  $(cat ${OUT} | cut -d' ' -f6 | grep '[ID]' | wc -l) ]"
	assertTrue "expected equivalence in reads count with indels but no soft clipping, should be equivalent in input and output files" "[ $(cat ${IN} | cut -d' ' -f6 | grep '[ID]' | grep -v '[S]' | wc -l) -eq $(cat ${OUT} | cut -d' ' -f6 | grep '[ID]' | grep -v '[S]' | wc -l) ]"
	assertFalse "unexpected equivalence in reads count with indels and soft clipping, should be different in input and output files" "[ $(cat ${IN} | cut -d' ' -f6 | grep '[ID]' | grep '[S]' | wc -l) -eq $(cat ${OUT} | cut -d' ' -f6 | grep '[ID]' | grep '[S]' | wc -l) ]"
    fi
}

#----------------------------
# testGetMappedReads
#
testGetMappedReads()
{
	TEST_SAM_FILE="data/test.sam"
	get_mapped_reads $TEST_SAM_FILE 2>${stderrF} >${stdoutF}

	samtools_msg="[samopen] SAM header is present"
	assertTrue "expected output to standard error" "[ -e ${stderrF} ]"
	assertTrue "unexpected message from samtools" "tail -1 ${stderrF} | grep -e ^${samtools_msg}"
	assertTrue "expected output to standard output" "[ -e ${stdoutF} ]"

	echo "stderr output:"
	cat ${stderrF}

	#echo "stdout output"
	#cat ${stdoutF}
}

#================

#
# Configuration
#

oneTimeSetUp()
{
    tests_start=`date +%H:%M:%S`
    . ../share/mutdetect/lib/mutdetect_lib.inc
    
    TEST_OUTPUT_DIR="output"

    PIPELINE_DEFAULT_CONFIG="../share/mutdetect/etc/mutdetect_default.config"
    PIPELINE_USER_CONFIG="../mutdetect_user.config"
    TEST_SAM="data/test_mapped_MAPQ.sam"
    TEST_FASTQC_1="data/test_1_Qual_Raw_Reads_test1.fq_fastqc_summary.txt"
    TEST_FASTQC_2="data/test_2_Qual_Raw_Reads_test2.fq_fastqc_summary.txt"
}

setUp()
{
    OUTPUT_DIR="${SHUNIT_TMPDIR}/OUTPUT"
    stdoutF="${OUTPUT_DIR}/stdoutF"
    stderrF="${OUTPUT_DIR}/stderrF"
    mkdir $OUTPUT_DIR
}

tearDown()
{  
    echo "Test starts ${tests_start}"
    tests_end=`date  +%H:%M:%S`
    echo "Test ends ${tests_end}"
    exec_start_time=`date +%s -d ${tests_start}`
    exec_end_time=`date +%s -d ${tests_end}`
    exec_time=$[${exec_end_time}-${exec_start_time}]
    echo |awk -v time="$exec_time" '{print "execution time: " strftime("%Hh:%Mm:%Ss", time, 1)}'
    rm -rf $OUTPUT_DIR
    echo "------------"
}

. /usr/local/share/shunit2-2.1.6/src/shunit2
