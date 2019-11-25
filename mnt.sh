#!/bin/sh

set -x
sudo mkdir /n
sudo mount -t 9p -o trans=virtio,version=9p2000.L test_mount /n
cd /n
sudo ./ubuntu.sh
