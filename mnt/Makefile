
include env.mk
all: prepare dpdk bicmp
       #	numa


DPDK=dpdk-stable-17.11.9
TARGET=x86_64-native-linuxapp-gcc

prepare: system build_dpdk post

system: apt huge64 hugemount

apt:
	apt install libnuma-dev gcc gdb vim libelf-dev
	#apt install python-pip
	#pip install pexpect

hugemount:
	mkdir -p /huge
	mount -t hugetlbfs nodev /huge

huge64:
	echo 64 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

rebuild: rmbuild bicmp

rmbuild:
	cd ${DPDK} && ${MAKE} T=${TARGET} O=${TARGET}
	make -C icmp clean

bicmp: dpdk
	make -C icmp

br: rebuild runtest

run: runtest
runtest:
	cd icmp/build && ./icmp
	
dpdk-stable-17.11.9/Makefile:
	tar -xf dpdk-17.11.9.tar.xz
	

dpdk: build_dpdk 
build_dpdk: dpdk-stable-17.11.9/Makefile ${DPDK}/${TARGET}/kmod/igb_uio.ko

${DPDK}/${TARGET}/kmod/igb_uio.ko:
	cd ${DPDK} && make config T=${TARGET} O=${TARGET}
	cd ${DPDK} && ${MAKE} T=${TARGET} O=${TARGET}
	# dpdk-stable-17.11.9/app/test-pmd/build/app/testpmd
	#(. env.mk ; cd ${DPDK} && make app )

testpmd: build_dpdk ${DPDK}/app/test-pmd/build/app/testpmd
	cd ${DPDK}/app/test-pmd/ && make
	rm testpmd
	ln -s ${DPDK}/app/test-pmd/build/app/testpmd testpmd

post: build_dpdk
	modprobe uio
	rmmod igb_uio.ko || true
	insmod ${DPDK}/${TARGET}/kmod/igb_uio.ko
	cd ${DPDK} && ./usertools/dpdk-devbind.py --bind=igb_uio 0000:00:03.0


# Using libnuma from ubuntu arhive
build_numa:
	cd numactl-2.0.13 && ./configure --prefix=/usr && make && make install
