#! /bin/awk -f

BEGIN {print "Latex start";}

# Ignore skipped lines
skip == 1 { skip = 0; sum += 1; next }

# Ignore lines with less than 6 chars
length($0) < 6 { sum += 1; next }

/^\(\.\// { sum += 1; next }
/^\(\.\// { sum += 1; next }
/^\(\// { sum += 1; next }
/^Writing index file/ { sum += 1; next }
/^No complaints by nag/ { sum += 1; next }
/^AED\: / { sum += 1; next }
/^ABD\: / { sum += 1; next }
/^\[\]\[\]\[\]\[\]/ { sum += 1; next }
/<to be read again>/ { sum += 1; next }
/No file / { sum += 1; next }
/write18 enabled/ { sum += 1; next }
/This is / { sum += 1; next }
/^LaTeX2e/ { sum += 1; next }
/^entering extended mode/ { sum += 1; next }
/^Babel/ { sum += 1; next }
/^Document Class/ { sum += 1; next }
/^Package hyperref Message:/ { sum += 1; next }
/^For additional information/ { sum += 1; next }
/^Package pgfplots: loading/ { sum += 1; next }
/^\*geometry\*/ { sum += 1; next }
/^</ { sum += 1; next }
/^ </ { sum += 1; next }
/^pdfTex warning/ { sum += 1; next }
/^Transcript written/ { sum += 1; next }
/^====/ { sum += 1; next }

# Don't display location
/^Chapter / { sum += 1; next }
/^Appendix / { sum += 1; next }

# Ignore some warnings
/^Package hyperref Warning:/ { sum += 1; skip = 1; next }
/^LaTeX Warning\: Citation/ { sum += 1; next }
/^LaTeX Warning\: Marginpar on page/ { sum += 1; next }

# Ignore overfull / underfull
/^Overfull \\hbox/ { sum += 1; skip = 1; next }
/^Underfull \\hbox/ { sum += 1; skip = 1; next }
/^Overfull \\vbox/ { sum += 1; next }
/^Underfull \\vbox/ { sum += 1; next }

/./ 

END{print "Latex finish, ignored ", sum, " statements"}
