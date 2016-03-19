# qwk2wwiv
Linux QWK &lt;--> WWIV (net) tool 

## to compile:

gcc -c main.c
gcc -c q2w.c
cd inih
gcc -c ini.c
cd ..
gcc -o qwk2wwiv main.o q2w.o inih/ini.o

(Yes I need to make a Makefile)

## to setup:

Firstly, you will have to create a new "network" for your QWK network. Essentially your network will be 2 nodes, you and 
your upstream QWK network packet provider. You will need to assign both a WWIV node number.

edit the vert.ini, this is a sample for dovenet.

change the paths to fit your system

make sure yourwwivnode and theirwwivnode equals the ones you set up earlier.

make mail areas in Board edit, make them network and hosted by your upstream provider, make up sub types for sub numbers

enter the sub types and subnumbers into the ini file.

## to use:

make a script that downloads your qwk packet from your host, then changes to the directory where your ini file is, then run

qwk2wwiv q2w vert.ini

substituting the ini file for your own.

then delete the qwk packet.

if your network you setup in init is the second network:

run ./network1 .1
run ./network2 .1

otherwise replace .1 for the correct network number

TODO: Exporting Reply Packets
