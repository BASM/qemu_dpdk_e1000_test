#!/bin/sh

apt update
apt install make openssh-server

sudo passwd <<_EOF_
123
123
_EOF_

sudo sh -c "echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config"
systemctl start sshd
systemctl restart sshd


