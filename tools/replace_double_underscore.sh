#!/bin/bash

# This grep command finds all names that end with __ (but don't start with _ to
# avoid some compiler macros).
#
# grep -r -E -o -h -I --exclude-dir=build* --exclude-dir=.git --exclude-dir=cinch '\<[^_][A-Za-z0-9_-]+__\>' .. | sort | uniq

# Find all the files we need to process, this is all files except the ones in
# any build* directory, in .git and in cinch (there are no names ending in __
# in cinch that we need to replace).

# first build exclude list of all build directories
BUILD_DIRS="../build*"

BUILD_EXCLUDE=""
for D in ${BUILD_DIRS}; do
  BUILD_EXCLUDE="${BUILD_EXCLUDE} -o -path ${D}"
done

# now build find command
FILES="find .. ( -path ../tools/replace_double_underscore.sh -o -path ../.git -o -path ../cinch ${BUILD_EXCLUDE} ) -prune -o -type f"

# now filter out binary files (using https://stackoverflow.com/a/13659891/2998298)
FILES="${FILES} -exec grep -Iq . {} ; -and -print0"

# now run sed in place replacing trailing __ with _u in words that don't start
# with _ and only contain letters, digits, and _ or -
${FILES} | xargs -0 sed -ri 's/(\<[^_][A-Za-z0-9_-]+)__\>/\1_u/g'

# replace occurrences of __flecsi*__ and __flecsi*
${FILES} | xargs -0 sed -ri 's/\<__flecsi([A-Za-z0-9_-]+)__\>/flecsi\1/g'
${FILES} | xargs -0 sed -ri 's/\<__flecsi([A-Za-z0-9_-]+)\>/flecsi\1/g'

# some files that need special treatment
sed -ri 's/\<__([A-Za-z0-9_-]*[^_])\>/flecsi_internal_\1/g' ../flecsi/utils/reflection.h

sed -ri 's/__([ft])/\1/g' ../flecsi/utils/tuple_visit.h

sed -ri 's/to__cis_to_gis/to_cis_to_gis/g' ../flecsi/topology/mesh_topology.h
