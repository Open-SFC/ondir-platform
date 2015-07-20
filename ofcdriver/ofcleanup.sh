#!/bin/sh
echo "Reload"
echo "1. Openstack Controller"
echo "2. Openflow Controller"
read -p "Option: " TYPE
if [ "$TYPE" = "1" ]; then
CRD_PID=`ps -ef | grep -v grep | grep nscs-server | awk '{print $2}'`
echo "Killing CRD-SERVER..."
kill -9 $CRD_PID

CRD_PID=`ps -ef | grep -v grep | grep nscs-relay-agent | awk '{print $2}'`
echo "Killing CRD-RELAY-AGENT..."
kill -9 $CRD_PID

echo "Re-creating CRD Database..."

mysql -uroot -proot <<EOF
drop database crd;
create database crd;
EOF

ovs-vsctl --db=tcp:10.78.87.47:8090 del-br br-int
ovs-vsctl --db=tcp:10.78.87.47:8090 del-br br-tun
ovs-vsctl --db=tcp:10.78.87.49:8090 del-br br-int
ovs-vsctl --db=tcp:10.78.87.49:8090 del-br br-tun


echo "Starting CRD-SERVER..."
nohup nscs-server --config-file /usr/local/etc/crd/nscs.conf > /tmp/crd.log 2>&1 &
sleep 30
#nohup nscs-relay-agent --config-file /usr/local/etc/crd/nscs.conf > /tmp/relay.log 2>&1 &
#sleep 5

echo "Creating Cluster..."
nscs create-cluster --ca_cert_path /home/intoto/cluster/CRD-CA/Cluser-CA/rootCA.pem  --private_key_path /home/intoto/cluster/CRD-CA/Cluser-CA/rootCA.key  DefaultCluster

sleep 3
echo "Creating Controller"
cluster_id=`mysql -uroot -proot crd -ss -e "select id from crd_openflow_cluster;"`
nscs create-controller --ip_address 10.78.87.1 --port 6653 --cluster_id $cluster_id --name DefaultController --cell RegionOne


echo "Creating Dummy NetworkFunction and Config Handle for Testing"
tenant_id=`mysql -uroot -proot keystone -ss -e "select id from project where name='admin';"`
networkfunction_id='f109a439-e78f-4e73-a29e-155cafefccc8'
config_handle_id='f109b439-e78f-4e73-a29e-155cafefccc8'
`mysql -uroot -proot crd -ss -e "insert into ns_networkfunctions (id,tenant_id,name,description,shared) values ('$networkfunction_id','$tenant_id','Dummy','Dummy Description',0);"`
`mysql -uroot -proot crd -ss -e "insert into ns_config_handles (id,tenant_id,name,networkfunction_id,status,slug,config_mode) values ('$config_handle_id','$tenant_id','DummyConfig','$networkfunction_id',1,'dummy','NFV');"`

sleep 3

tail -f /tmp/crd.log
elif [ "$TYPE" = "2" ]; then
OCAS_PID=`ps -ef | grep -v grep | grep nscsas-api | awk '{print $2}'`
CONSUMER_PID=`ps -ef | grep -v grep | grep nscs-agent | awk '{print $2}'`
echo "Killing NSCSAS API..."
kill -9 $OCAS_PID
echo "Killing CRD Consumer..."
kill -9 $CONSUMER_PID
echo "Restarting Openflow Controller..."
/usr/local/ondir_platform/2.0/ond-of-stop.sh; /usr/local/ondir_platform/2.0/ond-of-start.sh
echo "Re-creating NSCSAS Database..."
mysql -uroot -proot <<EOF
drop database nscsas;
create database nscsas;
EOF
echo "Starting NSCSAS-API..."
nohup nscsas-api --config-file /usr/local/etc/nscsas/nscsas.conf --log-file /var/log/nscsas.log --verbose &
mkdir -p /var/log/crd
sleep 10
#mysql -uroot <<EOF
#use nscsas;
#insert into nscsas_domains (id,name,subject) values ('a209d26a-73a2-4b18-90c3-f1d8c7102ebe','FC_DOMAIN','fc_domain@abc.com');
#EOF
echo "Starting CRD Consumer..."
nohup nscs-agent --log-file /var/log/crd/consumer.log --config-file /usr/local/etc/nscsas/consumer.conf --config-file /usr/local/etc/nscsas/nscs.conf &
fi
echo "Cleanup Success!!!"

