set format x "%1.0te%T"
set format y "2^%g"
set grid
set xlabel "テキスト数"
set ylabel "鍵候補数"
set logscale x 2
#plot file using 1:2:5 with yerrorbars
#plot file using 1:2:6:7 with yerrorbars
plot "result_a.txt" using 1:2 t "A", "result_b.txt" using 1:2 t "B"
#plot "result_c.txt" using 1:2 t "C", "result_d.txt" using 1:2 t "D"