#!/bin/bash

LICTEXT="/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */"

for f in $@
do
	if [[ $(head -1 $f) == *Mozilla\ Public\ License* ]]
	then
		continue
	fi

	echo "$LICTEXT"|cat - $f > /tmp/out && mv /tmp/out $f
done

