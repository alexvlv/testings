#!/bin/bash

full_string="prefix_data:value_with_underscores_and_more"
delimiter=":"
target_symbol="_"
replacement_symbol="-"

# 1. Split the string into two parts at the first occurrence of the delimiter.
#    This gets the part before and the part after the delimiter.
prefix=${full_string%%$delimiter*}
suffix=${full_string#*$delimiter}

# 2. Perform the replacement only on the 'suffix' part.
modified_suffix=${suffix//${target_symbol}/${replacement_symbol}}

# 3. Reconstruct the full string.
result_string="${prefix}${delimiter}${modified_suffix}"

echo "Original string: $full_string"
echo "Result string:   $result_string"



echo "CONFIG_PACKAGE_kmod-cfg80211=y" | sed -E 's/^([^=]*=)(.*)/\1$(echo "\2" | sed "s/-/_/g")/'
echo "CONFIG_PACKAGE_kmod-cfg80211=y" | sed -E 's/(=.*)-/\1_/'
echo "CONFIG_PACKAGE_km-od-cfg80211=y-W" | sed -E '/=/{s/^([^=]*)-/\1_/g}'

# !!! WORKED !!!
echo 'foo-bar-baz=keep-this-part' | sed ':a; s/^\([^=]*\)-/\1_/; ta'
