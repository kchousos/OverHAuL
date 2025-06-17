set terminal png;
set output "test.png";
set title "Simple Moving Average vs Exponetial Moving Average";
set xlabel "Time";
set ylabel "Value";

set datafile separator ','
plot \
    "test.csv" using 1:2 with lines title 'Input' linecolor rgb "grey",\
    "test.csv" using 1:3 with lines title 'Simple Moving Average' linecolor rgb "green",\
    "test.csv" using 1:4 with lines title 'Exponential Moving Average'  linecolor rgb "blue"
