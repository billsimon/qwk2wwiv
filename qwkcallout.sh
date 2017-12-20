#!/bin/bash
#
# Set the following variables according to your environment. Some must match what you 
# set in your INI file.
WWIVDIR=/home/wwiv   # wwiv home dir
INIDIR=$WWIVDIR/qwk2wwiv  # location of qwk2wwiv and ini file
QWKDIR=$WWIVDIR/qwk # location for QWK packets and REPlies
DOVEDIR=$WWIVDIR/nets/dovenet  # location of your two-node WWIVNET used for QWK
DOVENODE=vert  # hub node, used in name of qwk and rep packets
DOVENODEID=2  # hub node's fake WWIVNET ID
OURNODEID=1  # our fake WWIVNET ID
WWIVNET=".1"  # our network identifier for the network* programs
HOST=dove.synchro.net  # where to ftp to get/send QWK and REP: dove.synchro.net for Dove-Net
FTPUSER=mybbs  # our user (QWK ID) on the hub node
FTPPASS=mybbspassword # password on the hub node

# Comment out the echo lines below if you don't want to get reports from your cron job.
# Or just redirect everything to /dev/null

# First, prepare any replies

if [[ -f $QWKDIR/$DOVENODE.rep ]] ; then
	# The old rep file wasn't sent or deleted. Notify.
	echo "$DOVENODE.rep is still waiting to be sent. Waiting for manual intervention."
	exit 1
fi

UPLOAD=""

if [[ -f $DOVEDIR/s$DOVENODEID.net ]] ; then
	echo Found outgoing packet to process.
	cd $INIDIR
	./qwk2wwiv w2r $DOVENODE.ini

	if [[ -f $QWKDIR/$DOVENODE.rep ]] ; then
		/bin/rm $DOVEDIR/s$DOVENODEID.net
		UPLOAD="put $DOVENODE.rep"
	fi
fi

if [[ -f $QWKDIR/$DOVENODE.qwk ]] ; then
	# A QWK file is still waiting to be processed. Don't download another one yet.
	echo There is already an incoming QWK file waiting to be processed. Not doing FTP.
	exit 1
fi


# Ready to download QWK and, optionally, upload replies
cd $QWKDIR
FTPLOG=$( /usr/bin/ftp -inv $HOST <<EOF 
user $FTPUSER $FTPPASS
$UPLOAD
get $DOVENODE.qwk
bye
EOF
)

if [[ $FTPLOG =~ "226 Upload complete" ]] ; then
	echo Successfully uploaded reply packet.
	/bin/rm $QWKDIR/$DOVENODE.rep
fi

if [[ $FTPLOG =~ "550 No QWK packet" ]] ; then
	echo There was no QWK packet to download.
	exit 0
fi

# Process incoming QWK
echo Processing incoming QWK packet.
cd $INIDIR
./qwk2wwiv q2w $DOVENODE.ini

if [[ -f $DOVEDIR/p$OURNODEID.net ]] ; then
	/bin/rm $QWKDIR/$DOVENODE.qwk

	cd $WWIVDIR
	./network1 $WWIVNET
	./network2 $WWIVNET
else
	echo There was an error converting the QWK packet.
fi

exit

