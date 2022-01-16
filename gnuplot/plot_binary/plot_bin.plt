set xzeroaxis
set yzeroaxis
plot "generated.bin" binary format="%short" endian=big array=(-1) using 1 every ::::99999
