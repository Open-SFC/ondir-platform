#!/bin/sh
echo "Reload"
OCAS_PID=`ps -ef | grep -v grep | grep nscsas-api | awk '{print $2}'`
CONSUMER_PID=`ps -ef | grep -v grep | grep crd-consumer | awk '{print $2}'`
echo "Killing OCAS API..."
kill -9 $OCAS_PID
echo "Killing CRD Consumer..."
kill -9 $CONSUMER_PID
echo "Restarting Openflow Controller..."
./ond-of-stop.sh;
sleep 3
./ond-of-start.sh;
echo "Re-creating NSCSAS Database..."
mysql -uroot -proot <<EOF
drop database nscsas;
create database nscsas;
EOF
echo "Starting NSCSAS-API..."
nohup nscsas-api --config-file /usr/local/etc/nscsas/nscsas.conf --log-file /var/log/nscsas.log --verbose &
mkdir -p /var/log/crd
sleep 10
#mysql -uroot -proot<<EOF
#use nscsas;
#insert into nscsas_domains (id,name,subject) values ('a209d26a-73a2-4b18-90c3-f1d8c7102ebe','FC_DOMAIN','fc_domain@abc.com');
#EOF
echo "Starting CRD Consumer..."
nohup crd-consumer --log-file /var/log/crd/consumer.log --config-file /usr/local/etc/nscsas/consumer.conf --config-file /usr/local/etc/nscsas/crd.conf &

