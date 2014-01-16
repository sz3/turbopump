#!/bin/bash

LICTEXT="/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */"

for f in $@
do
	echo "$LICTEXT"|cat - $f > /tmp/out && mv /tmp/out $f
done

