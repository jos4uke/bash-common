#! /bin/bash

#
# BASH COMMON FUNCTIONS LIBRARY: SAM UTILS
#

# Copyright 2014 Joseph Tran <Joseph.Tran@versailles.inra.fr>

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
# same conditions as eegards security. 

# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license, Version 2.0 (the "License"), and that you accept its terms.

# Date: 2014-06-03

# VERSION=v0.0.2

###
### SAM FILTERS
###

#-------------------
# parseTagIntValue  
#
parseTagIntValue()
{
	if [[ ${#tag}==2 ]]; then 
		gawk -v tag=$1 '#BEGIN {tagpat = tag ":i:([0-9]+)";} 
			# treat tags as one field
			function parseTagValue(pat) {
				tags=""
				if (NF>11) {
				for(i=12;i<=NF;i++){tags=tags" "$i}
				#print tags >"/dev/stderr"
				delete a
				match(tags, pat, a) 
				if (a[1]~"[0-9]+") { print a[1] } else {print "NA"; print tags >"/dev/stderr"}
				
			} else
			{
				print "Missing tags for alignment:" $0 >"/dev/stderr"
			}
		}
		BEGIN {tagpat = tag ":i:([0-9]+)";}
		{
			parseTagValue(tagpat)
		}'
	else
		echo -e "tag $tag is not a 2-character value." >&2
	fi
}

#-------------------
# countIndels
countIndels()
{
	gawk '
		function findAllIndels(str)
		{
			count=0;
			totsize=0;
			while(match(str, /([0-9]+)[ID]/, a))
			{  
				sub("[ID]", "", a[0]);
				count++;
				totsize+=a[0]
				str = substr(str, RSTART+RLENGTH)
			}
			print $1"\t"count"\t"totsize 
		}

		BEGIN {
			print "read_id\tnumIndels\tcumInDelsSize"
		}

		{
			findAllIndels($6)	
		}
	'
}

#--------
# calcIE
calcIE()
{
	gawk -v snp=$1 '
	function parseTagValue(pat) 
	{
		tags=""
		if (NF>11) {
			for(i=12;i<=NF;i++){tags=tags" "$i}
			#print tags >"/dev/stderr"
			delete a
			match(tags, pat, a) 
			if (a[1]~"[0-9]+") { return a[1] } else { return "NA"; print tags >"/dev/stderr"}
			
		} else
		{
			print "Missing tags for alignment:" $0 >"/dev/stderr"
		}    
	}	

	function countAllIndels(str,c)
	{
		count=0;
		totsize=0;
		while(match(str, /([0-9]+)[ID]/, a))
		{  
			sub("[ID]", "", a[0]);
			count++;
			totsize+=a[0]
			str = substr(str, RSTART+RLENGTH)
		}
		c[1]=count
		c[2]=totsize
	}

	BEGIN {
		if (snp=="snp") {
			# IE=XW+numInDels and NM=XW+XV+cumInDelsSize
			print "read_id\tNM\tXW\tXV\tnumInDels\tcumInDelsSize\tIE"
		} else
		{
			# IE=NM-cumInDelsSize+numInDels
			print "read_id\tNM\tnumInDels\tcumInDelsSize\tIE"
		}
	}

	{
		# get NM
		nmpat="NM:i:([0-9]+)"
		NM=parseTagValue(nmpat)

		delete idc
		# no NM value
		if (NM=="NA") {
			if (snp=="snp") 
			{
				print $1"\tNA\tNA\tNA\tNA\tNA\tNA"
			} else
			{
				print $1"\tNA\tNA\tNA\tNA"
			}
		} else
		{
			# consider only unique alignment per read
			if ($0~/NH:i:1[[:space:]]/) {
			# without SNP
			if (length(snp) == 0) 
			{
				xvpat="XV:i:([0-9]+)"
				if (match($0,xvpat)) {
					print "XV tag is present in the alignment. Consider to use 'snp' switch." >"/dev/stderr"
					exit 1
				}
				countAllIndels($6,idc)
				IE=NM - idc[2] + idc[1]
				print $1 "\t" NM "\t" idc[1] "\t" idc[2] "\t" IE
			# with SNP
			} else
			{
				# get XW and XV
				xwpat="XW:i:([0-9]+)"
				XW=parseTagValue(xwpat)
				xvpat="XV:i:([0-9]+)"
				XV=parseTagValue(xvpat)
				countAllIndels($6,idc)
				IE=XW + idc[1]
				print $1 "\t" NM "\t" XW "\t" XV "\t" idc[1] "\t" idc[2] "\t" IE
			}
			}
		}
	}'
}

#---------------
# keepAlnForTag
keepAlnForTag()
{
	gawk -v tag=$1 -v tmax=$2 -v snp=$3 '
	function parseTagValue(pat) 
	{
		tags=""
		if (NF>11) {
			for(i=12;i<=NF;i++){tags=tags" "$i}
			#print tags >"/dev/stderr"
			delete a
			match(tags, pat, a) 
			if (a[1]~"[0-9]+") { return a[1] } else { return "NA"; print tags >"/dev/stderr"}
			
		} else
		{
			print "Missing tags for alignment:" $0 >"/dev/stderr"
		}    
	}	

	function countAllIndels(str,c)
	{
		count=0;
		totsize=0;
		while(match(str, /([0-9]+)[ID]/, a))
		{  
			sub("[ID]", "", a[0]);
			count++;
			totsize+=a[0]
			str = substr(str, RSTART+RLENGTH)
		}
		c[1]=count
		c[2]=totsize
	}

	BEGIN {
		# test tag
		if (tag!~/[I[EM]/) {print "Tag value should be IE or IM." >"/dev/stderr"; exit 1}
		# test tmax
		if (tmax!~/[0-9]+/) {print tag "max value is not a digit." >"/dev/stderr"; exit 1}

		# count alignments
		## all
		all=0
		## nh1
		nh1=0
		## le_tmax
		le_tmax=0
		## gt_tmax
		gt_tmax=0
	}

	{
		all++
		# get NM
		nmpat="NM:i:([0-9]+)"
		NM=parseTagValue(nmpat)

		delete idc
		# no NM value
		if (NM!="NA") {
			# consider only one best alignment per read
			if ($0~/NH:i:1[[:space:]]/) {
			nh1++
			# without SNP
			if (length(snp) == 0) 
			{
				xvpat="XV:i:([0-9]+)"
				if (match($0,xvpat)) {
					print "XV tag is present in the alignment. Consider to use 'snp' switch." >"/dev/stderr"
					exit 1
				}
				countAllIndels($6,idc)
				IE=NM - idc[2] + idc[1]
				IM=NM - idc[2]
				if (tag=="IE") {
					if (IE<=tmax && IE>=0) {le_tmax++; print $0} else {gt_tmax++}
				}
				if (tag=="IM") {
					if (IM<=tmax && IM>=0) {le_tmax++; print $0} else {gt_tmax++}
				}
			# with SNP
			} else
			{
				# get XW and XV
				xwpat="XW:i:([0-9]+)"
				XW=parseTagValue(xwpat)
				xvpat="XV:i:([0-9]+)"
				XV=parseTagValue(xvpat)
				countAllIndels($6,idc)
				IE=XW + idc[1]
				if (tag=="IE") {
					if (IE<=tmax && IE>=0) {le_tmax++; print $0} else {gt_tmax++}
				}
				if (tag=="IM") {
					# get XW        			
					xwpat="XW:i:([0-9]+)"
					XW=parseTagValue(xwpat)
					if (XW<=tmax) {le_tmax++; print $0} else {gt_tmax++}
				}	
			}
			}
		}
	}
	END {
		print "total input alignments: " all >"/dev/stderr"
		print "total NH:i:1 alignments: " hi1 >"/dev/stderr"
		print "total alignments lower or equal to" tag "=" tmax ": " le_tmax >"/dev/stderr"
		print "total alignments greater than" tag "=" tmax ": " gt_tmax >"/dev/stderr"	
	}'
}
