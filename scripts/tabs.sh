#!/bin/bash

# Change your output.cgi so it sleeps for 10 seconds ( sleep 10 )
cd ..

echo "dynamic or blockflush? (choose an option by number)"
select yn in "dynamic" "blockflush"; do
    case $yn in
        dynamic )     osascript -e 'tell application "Terminal" to do script "/Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/server 2002 4 2 dynamic 3; exec bash -i"';   break;;
        blockflush )  osascript -e 'tell application "Terminal" to do script "/Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/server 2002 3 3 bf ; exec bash -i"'; osascript -e 'tell application "Terminal" to do script "/Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client /Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files 2002 output.cgi?1; exec bash -i"'; break;;
    esac
done

sleep 1

osascript -e 'tell application "Terminal" to do script "/Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client /Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/public 2002 output.cgi?1; exec bash -i"'
osascript -e 'tell application "Terminal" to do script "/Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/client /Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/public 2002 output.cgi?1; exec bash -i"'

case $yn in
    dynamic )     osascript -e 'tell application "Terminal" to do script "/Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/scripts/dynamic.sh;exec bash -i"';   ;;
    blockflush )  osascript -e 'tell application "Terminal" to do script "/Users/yagelmaimon/Desktop/technion/operatingsystem/my_hws/hw3/wet/webserver-files/scripts/blockflush.sh;exec bash -i"';    ;;
esac
