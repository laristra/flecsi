#! /bin/awk -f

BEGIN {print "makeindex start";}

/^This is makeindex/ { sum += 1; next }
/^Scanning input file/ { sum += 1; next }
/^Scanning style file/ { sum += 1; next }
/^Sorting entries/ { sum += 1; next }
/^Generating output file/ { sum += 1; next }
/^Output written/ { sum += 1; next }
/^Transcript written/ { sum += 1; next }

/./ 

END{print "makeindex finish, ignored ", sum, " statements"}
