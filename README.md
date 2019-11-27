Quick start qemu+dpdk on the Ubuntu 18.04
=========================================

* host   -- Your PC
* target -- The emulated PC (qemu)

Setup guide:
* Download qemu, dpdk and ubuntu live cd
```
 make get
```

* Build qemu:
```
 make build_qemu
```

* Start Ubuntu 18.04 on the QEMU live
```
  make
```

* After start to 'Install' select "Try Xubuntu"
* Open console Terminal Emulator from menu
* Mount device with deploy scripts:
```
 sudo mount /dev/sda /mnt
```

* Start deploy script:
```
 sudo sh /mnt/mnt.sh
```

Afther that it install gnu make, sshd, change password for root to '123' and start sshd. And it mount the '/n' shared directory.

* Connect to target from host:
```
  ssh -p 9999 root@localhost
```
 type password: 123

* Login you need build deploy dpdk:
```
 cd /n
 make

```

It:
* Insall dependences (libcap-dev, libattr1-dev, libnuma-dev)
* Build dpdk target x86_64-native-linuxapp-gcc
* Build examples, tests...
* Mount hugepagetlb, set 64 huge pages in system
* Create link testpmd to /n/testpmd
* Build icmp test

* For build application by self, update your enviretments:
```
 source /n/env.mk
```

Run test
---------

* Prepare qemu + dpdk + test, see above.
* On target (from root):
```
 cd /n
 make run
```
you see:
```
EAL: Detected 1 lcore(s)
EAL: Probing VFIO support...
EAL: WARNING: cpu flags constant_tsc=yes nonstop_tsc=no -> using unreliable clock cycles !
EAL: PCI device 0000:00:03.0 on NUMA socket -1
EAL:   Invalid NUMA socket, default to 0
EAL:   probe driver: 8086:100e net_e1000_em
EAL: PCI device 0000:00:04.0 on NUMA socket -1
EAL:   Invalid NUMA socket, default to 0
EAL:   probe driver: 1af4:1000 net_virtio
Port 0 MAC: 00 12 f8 27 74 00
```

On the host PC set ip address:
```
sudo ip addr add 1.1.1.1/24 dev tapT0
```
or
```
sudo ifconfig tapT0 1.1.1.1/24 up
```

And run ping with flood:
```
 sudo ping 1.1.1.2 -f
```

wait 1-60 sec and packs stopped.


* FIN

