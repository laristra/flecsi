#! /bin/awk -f

BEGIN {print "BibTex start";}

/^This is BibTex/ { sum += 1; next }
/^The top-level auxiliary file/ { sum += 1; next }
/^A level-1 auxiliary file/ { sum += 1; next }
/^The style file/ { sum += 1; next }
/^Database file/ { sum += 1; next }

/./ 

END{print "BibTex finish, ignored ", sum, " statements"}
