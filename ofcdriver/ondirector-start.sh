#!/bin/bash

pkill controller
pkill ofucmd
pkill ofldsvd
pkill httpd
# light httpd is packaged as part of SDK, we do not need it
pkill lighttpd

export LD_LIBRARY_PATH=/ondirector/lib
echo "/ondirector/lib/" >> /etc/ld.so.conf
echo "/ondirector/lib/libnfct" >> /etc/ld.so.conf

echo 1 > /proc/sys/net/ipv4/ip_forward

ldconfig
echo "Starting Controller..."
rm -rf /var/log/oflog*
cd /ondirector/bin
#controller command:- 

#./controller -coremask -numberofthreads -portnumber(default is 6653)
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

