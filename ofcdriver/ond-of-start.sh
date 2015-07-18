#!/bin/bash

pkill controller
pkill ofucmd
pkill ofldsvd
pkill httpd


export LD_LIBRARY_PATH= $(pwd)/lib
echo "$(pwd)/lib/" >> /etc/ld.so.conf

echo 1 > /proc/sys/net/ipv4/ip_forward

ldconfig
echo "Starting Controller..."
rm -rf /var/log/oflog*
cd bin


#./controller -coremask -numberofthreads -portnumber(default is 6653)
#./controller 0,1,2,3 8 -f &
./controller 0 8 -f &

echo "Starting LDSV..."
./ofldsvd -f &
sleep 2
echo "Starting ofucmd..."
./ofucmd &
echo "Starting httpd..."
./httpd &
ps -aef|grep controller
ps -aef|grep ucmd
ps -aef|grep ofldsvd
ps -aef|grep httpd

