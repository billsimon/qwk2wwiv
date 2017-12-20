# qwk2wwiv
Linux QWK &lt;--> WWIV (net) tool 

## to compile:

  make

## to setup:

Firstly, you will have to create a new "network" for your QWK network. Essentially your network will be 2 nodes, you and 
your upstream QWK network packet provider. You will need to assign both a WWIV node number. You can follow the directions
given at http://docs.wwivbbs.org/en/wwiv52/usenet/ as an example for setting up a two-node "fake" network.

Edit the vert.ini. This is a sample for Dove-Net.
- Change the paths to fit your system.
- Make sure yourwwivnode and theirwwivnode equals the ones you set up earlier.

Make mail areas in Board Edit; make them network and hosted by your upstream provider; make up sub types for sub numbers. 
Also ensure that you do not set a tagline, as this program will set one for you.

Enter the sub types and subnumbers into the ini file.

## to use:

Modify the *qwkcallout.sh* script to match your environment. Add it to crontab to perform the QWK exchange on a regular
basis. Dove-Net recommends four times daily. 

Review the callout script to understand what it does. Requires `ftp`.

