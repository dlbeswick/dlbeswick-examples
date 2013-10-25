SERVICE=$1

if [ -z $1 ] 
	then echo "$0: No process specified."
	exit 3
fi
 
ps ax | grep -v grep | grep $SERVICE > /dev/null
