#!/bin/sh


ifname=$1
echo "ETH UP name $name "

if [ "$ifname" = "tapT0" ] ; then
  ip addr add 1.1.1.1 dev $ifname
  ip link set $ifname up
fi


