#!/bin/bash

# Change your output.cgi so it sleeps for 10 seconds ( sleep 10 )
> errors.txt
> prevstate.txt
> empty.txt
> expected.out

Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client localhost 2002 home.html  > trash.out 2> errors.txt #should get error and size changes by 1 (goes up to 3 now)
sleep 0.5
echo "Rio_readlineb error: Connection reset by peer" > expected.out

if ! cmp -s errors.txt expected.out || (! cmp -s errors.txt prevstate.txt || ! cmp -s trash.out empty.txt); then
    echo "failed"
    echo "1"
    exec bash -i
fi

Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client localhost 2002 home.html  > trash.out 2> errors.txt #this should work
sleep 0.5
if cmp -s trash.out empty.txt && ! cmp -s errors.txt empty.txt; then
    echo "failed"
    echo "2"
    exec bash -i
fi

sleep 0.1
cat errors.txt > prevstate.txt
sleep 0.1

osascript -e 'tell application "Terminal" to do script "Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client localhost 2002 output.cgi?1; exec bash -i"'
sleep 1

Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client localhost 2002 home.html > trash.out  2> errors.txt #should get error 
echo "Rio_readlineb error: Connection reset by peer" > expected.out
if ! cmp -s errors.txt expected.out || (! cmp -s errors.txt prevstate.txt || ! cmp -s trash.out empty.txt); then
    echo "failed"
    echo "3"
    exec bash -i
fi

sleep 0.1
cat errors.txt > prevstate.txt
sleep 0.1

Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client localhost 2002 home.html > trash.out  2>> errors.txt #fail
sleep 0.5

echo "Rio_readlineb error: Connection reset by peer" >> expected.out
if ! cmp -s errors.txt expected.out || (! cmp -s errors.txt prevstate.txt || ! cmp -s trash.out empty.txt); then
    echo "failed"
    echo "4"
    exec bash -i
fi

sleep 0.1
cat errors.txt > prevstate.txt
sleep 0.1
Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client localhost 2002 home.html > trash.out  2>> errors.txt   #fail

sleep 1
echo "Rio_readlineb error: Connection reset by peer" >> expected.out
if ! cmp -s errors.txt expected.out || (! cmp -s errors.txt prevstate.txt || ! cmp -s trash.out empty.txt); then
    echo "failed"
    echo "5"
    exec bash -i
fi
sleep 0.1

cat errors.txt > prevstate.txt
sleep 0.1

if cmp -s errors.txt expected.out; then
    echo "SUCCESS"
    echo "remember to change output.c back to original before submitting"
else
    if ! cmp -s errors.txt prevstate.txt || ! cmp -s trash.out empty.txt; then
        echo "failed"
        echo "end"
        exec bash -i
    fi
    echo "SUCCESS"
    echo ""
    echo "remember to change output.c back to original before submitting"
fi

rm -f empty.txt
rm -f errors.txt
rm -f trash.out 
echo "You can close this tab"
while true; do true; done
