set ns [new Simulator]

set R1 [$ns node]
set R2 [$ns node]
$ns duplex-link $R1 $R2 1Mb 5ms DropTail

set src1 

$ns run