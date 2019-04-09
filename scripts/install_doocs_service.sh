#!/bin/bash
# 
# bash script to install doocs service on local or on remote host
# dooc service starts DOOCS watchdog
# URLs:
#    https://www.tecmint.com/create-new-service-units-in-systemd/
#    https://medium.com/@benmorel/creating-a-linux-service-with-systemd-611b5c8b91d6
#

serverNameOrigin=`hostname`
serverName=`hostname`
userName=root


if [ "$#" -gt 0 ]; then
  serverName=$1
fi
echo "serverName="$serverName

if [ "$#" -gt 1 ]; then
  userName=$2
fi
echo "userName="$userName

cat > doocs.service << EOTEXT
[Unit]
Description=Service to start DOOCS watchdog
DefaultDependencies=no

# Make sure we use the IP addresses listed for
# rpcbind.socket, no matter how this unit is started.
After=rpcbind.service

[Service]
Type=forking
#EnvironmentFile=/etc/sysconfig/rpcbind
ExecStart=/export/doocs/server/doocs_w wstart

[Install]
WantedBy=multi-user.target

EOTEXT



if [ "$serverName" == "$serverNameOrigin" ]; then
	mv doocs.service /usr/lib/systemd/system/.
	systemctl enable doocs.service
else
	scp doocs.service ${userName}@${serverName}:/usr/lib/systemd/system/.
	rm doocs.service
ssh -l ${userName} -T "${serverName}" << EOSSH
	systemctl enable doocs.service
EOSSH
fi
