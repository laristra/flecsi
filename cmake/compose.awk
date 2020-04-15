#! /bin/awk -f

{
    if (length($0) == 79){
        l = $0""l
    } else {
        if (l) {
            print l
        }

        l = $0
    }
} 

END {if (l) print l}
