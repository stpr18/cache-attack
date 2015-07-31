set format x "%1.0te%T"
set format y "2^%g"
unset key
set grid
set xlabel "テキスト数"
set ylabel "鍵候補数"
#plot file using 1:2:5 with yerrorbars
#plot file using 1:2:6:7 with yerrorbars
plot file using 1:2
