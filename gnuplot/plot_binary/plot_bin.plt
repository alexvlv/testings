#!/usr/bin/env gnuplot -persist
# $Id$

set xzeroaxis
set yzeroaxis

set xtics 64

set format x "%X"
set format y "%X"
set grid x2tics
set x2tics 512 format "" scale 0
#set arrow from 512, graph 0 to 512, graph 1 nohead
#set arrow from 1024, graph 0 to 1024, graph 1 nohead

do for [i = 0:254 ] {
	x = 0 + i*512
	set arrow from x, graph 0 to x, graph 1 nohead linecolor "blue"
	
}

plot "17_01_2022_17_16_34.9591-1100.es" binary format="%short" endian=big array=(-1) using 1 every ::::99999

# set arrow from x to x nohead linecolor "blue"