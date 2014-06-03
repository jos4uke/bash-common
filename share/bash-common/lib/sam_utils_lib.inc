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
# same conditions as regards security. 

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
	local tag=$1
	if [[ ${#tag}==2 ]]; then 
		gawk -v tag=$tag 'BEGIN {tagpat = tag ":i:([0-9]+)";} {
			# treat tags as one field
			if (NF>11) {
				for(i=12;i<=NF;i++){tags=tags" "$i}
				match(tags, tagpat, a)
				if (a[1]~"[0-9]+") { print a[1] } else {print "NA"; print tags >"/dev/stderr"}
			} else
			{
				print "Missing tags for alignment:" $0 >"/dev/stderr"
			}
		}'
	else
		echo -e "tag $tag is not a 2-character value." >&2
	fi
}


