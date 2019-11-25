

LIVEURI=https://mirror.yandex.ru/ubuntu-cdimage/xubuntu/releases/18.04/release/${LIVEPATH}
#LIVEPATH=livedvd-amd64-multilib-20160704.iso
#LIVEPATH=ubuntu-18.04.3-live-server-amd64.iso
LIVEPATH=xubuntu-18.04.3-desktop-amd64.iso

DPDKFILE=mnt/dpdk-17.11.9.tar.xz

all: get build_host qemu_run

get: get_live get_qemu get_dpdk

build_host: build_qemu

build_qemu: get_qemu qemu/x86_64-softmmu/qemu-system-x86_64 qemu/qemu-img
	
qemu/qemu-img qemu/x86_64-softmmu/qemu-system-x86_64: qemu/Makefile
	cd qemu && make -j8

qemu/Makefile: qemu/configure
	cd qemu && ./configure --target-list="x86_64-softmmu"  --datadir=`pwd`  --with-confsuffix="/pc-bios" --disable-docs --enable-virtfs --disable-libusb

get_live: ${LIVEPATH}
  
${LIVEPATH}:
	wget -c ${LIVEURI}

get_qemu: qemu/configure

qemu/configure:
	git clone https://git.qemu.org/git/qemu.git

qemu/capstone/README: qemu/configure
	cd qemu && git submodule init && git submodule update

get_dpdk: ${DPDKFILE}
	#numactl-2.0.13.tar.gz
	#wget -c https://github.com/numactl/numactl/releases/download/v2.0.13/numactl-2.0.13.tar.gz 

${DPDKFILE}:
	wget -c https://fast.dpdk.org/rel/dpdk-17.11.9.tar.xz -O ${DPDKFILE}

QEMU_OPTS+= -cpu Broadwell
QEMU_OPTS+= -accel kvm
QEMU_OPTS+= -m 2G
QEMU_OPTS+= -vga virtio
QEMU_OPTS+= -hda disk.raw -cdrom ${LIVEPATH}
QEMU_OPTS+= -device e1000,mac=00:12:f8:27:74:00,netdev=net1 -netdev tap,id=net1,script=ethup.sh,ifname=tapT0
QEMU_OPTS+= -device virtio-net,netdev=netu0 -netdev user,id=netu0,hostfwd=tcp::9999-:22

QEMU_OPTS+= -fsdev local,id=test_dev,path=`pwd`/mnt,security_model=none -device virtio-9p-pci,fsdev=test_dev,mount_tag=test_mount


#QEMU_OPTS+= -device e1000,mac=00:12:f8:27:74:01,netdev=net2 -netdev tap,id=net2,script=ethup.sh,ifname=tapM0
#,hostfwd=tcp::5555-:22
#mount -t 9p -o trans=virtio,version=9p2000.L /vmshare /home/guest-username/vmshare

qemu_run: disk.raw ${LIVEPATH}
	sudo ./qemu/x86_64-softmmu/qemu-system-x86_64 ${QEMU_OPTS}

disk.raw: ${DPDKFILE}
	mkdir -p mnt
	./qemu/qemu-img create -f raw disk.raw 1M
	mkfs.ext3 disk.raw
	sudo mount disk.raw mnt
	sudo cp mnt.sh mnt
	sudo umount mnt
	#./qemu/qemu-img convert disk.raw -f raw -O qcow2 disk.qc2
	#rm disk.raw

ubuntureq:
	apt install libcap-dev libattr1-dev

