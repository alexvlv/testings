set xzeroaxis
set yzeroaxis
set format x "%X"
set format y "%X"
plot "generated.bin" binary format="%short" endian=big array=(-1) using 1 every ::::99999
