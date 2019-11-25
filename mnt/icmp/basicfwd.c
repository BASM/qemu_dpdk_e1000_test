/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2015 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/// FIXED:  without SMP
//
//
#include <stdint.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include <rte_ether.h>
#include <rte_arp.h>
#include <rte_ip.h>
#include <rte_icmp.h>

#define RX_RING_SIZE 32
#define TX_RING_SIZE 512

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static const struct rte_eth_conf port_conf_default = {
	.rxmode = { .max_rx_pkt_len = ETHER_MAX_LEN }
};

/* basicfwd.c: Basic DPDK skeleton forwarding example. */
// FIXES: one port for RX and TX

/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
static inline int
port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
	struct rte_eth_conf port_conf = port_conf_default;
	const uint16_t rx_rings = 1, tx_rings = 1;
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	//uint16_t q;

	if (port >= rte_eth_dev_count()) return -1;

	/* Configure the Ethernet device. */
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if (retval != 0) return retval;

	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if (retval != 0) return retval;

	/* Allocate and set up 1 RX queue per Ethernet port. */
	retval = rte_eth_rx_queue_setup(port, 0, nb_rxd, 0, NULL, mbuf_pool);
	if (retval < 0) return retval;

	retval = rte_eth_tx_queue_setup(port, 0, nb_txd, 0, NULL);
	if (retval < 0) return retval;

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);
	if (retval < 0) return retval;

	/* Display the port MAC address. */
	struct ether_addr addr;
	rte_eth_macaddr_get(port, &addr);
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port,
			addr.addr_bytes[0], addr.addr_bytes[1],
			addr.addr_bytes[2], addr.addr_bytes[3],
			addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	rte_eth_promiscuous_enable(port);
	rte_eth_allmulticast_enable(port);
	return 0;
}
static int ether_swap(struct ether_hdr *ehdr) {
	struct ether_hdr tmp;

	memcpy(&tmp, &ehdr->d_addr, 6);
	memcpy(&ehdr->d_addr, &ehdr->s_addr, 6);
	memcpy(&ehdr->s_addr, &tmp, 6);
	return 0;
}

static int arp_reply(struct rte_mbuf **bufs, int cnt) {
	int i;

	for (i=0; i<cnt; i++) {
		uint8_t *data;
		struct ether_hdr *ehdr;
	       	data=rte_pktmbuf_mtod((struct rte_mbuf *)bufs[i], void*);
	       	ehdr=(void*)data;
		if (ehdr->ether_type==rte_cpu_to_be_16(ETHER_TYPE_ARP)) {
			printf("Its ARP\n");
			struct arp_hdr *ahdr=(void*)(ehdr+1);
			struct ether_addr tmp_hw={.addr_bytes={0x10,0xc3,0x7b,0x1,0x2,0x3}};
			uint32_t          tmp_ip;
			//Dirty hack
			ahdr->arp_op=rte_cpu_to_be_16(ARP_OP_REPLY);
			tmp_ip=ahdr->arp_data.arp_tip;

			memcpy(&ahdr->arp_data.arp_tha, &ahdr->arp_data.arp_sha, 6);
			ahdr->arp_data.arp_tip=ahdr->arp_data.arp_sip;
			memcpy(&ahdr->arp_data.arp_sha, &tmp_hw, 6);
			ahdr->arp_data.arp_sip=tmp_ip;
			
			memcpy(&ehdr->d_addr, &ahdr->arp_data.arp_tha, 6);
			memcpy(&ehdr->s_addr, &ahdr->arp_data.arp_sha, 6);
			

		}
	}
	return 0;
}

static int icmp_reply(struct rte_mbuf **bufs, int cnt) {
	int i;

	for (i=0; i<cnt; i++) {
		uint8_t *data;
		struct ether_hdr *ehdr;
	       	data=rte_pktmbuf_mtod((struct rte_mbuf *)bufs[i], void*);
	       	ehdr=(void*)data;
		if (ehdr->ether_type==rte_cpu_to_be_16(ETHER_TYPE_IPv4)) {
			struct ipv4_hdr *iphdr = (void*)&ehdr[1];
			if (iphdr->next_proto_id == 1) { //1 -- ICMP 
				int iphdrlen = (iphdr->version_ihl&0x0f)*4;
				struct icmp_hdr *icmphdr = (void*) &((uint8_t*) iphdr)[iphdrlen];
				if (icmphdr->icmp_type==IP_ICMP_ECHO_REQUEST) {
					uint32_t tmp_ip;
					uint32_t cksum;

					icmphdr->icmp_type  =IP_ICMP_ECHO_REPLY;
					cksum = ~icmphdr->icmp_cksum & 0xffff;
					cksum += ~htons(IP_ICMP_ECHO_REQUEST << 8) & 0xffff;
					cksum += htons(IP_ICMP_ECHO_REPLY << 8);
					cksum = (cksum & 0xffff) + (cksum >> 16);
					cksum = (cksum & 0xffff) + (cksum >> 16);
					icmphdr->icmp_cksum = ~cksum;

					tmp_ip=iphdr->src_addr;
					iphdr->src_addr=iphdr->dst_addr;
					iphdr->dst_addr=tmp_ip;

					iphdr->hdr_checksum=0;
					iphdr->hdr_checksum = rte_ipv4_cksum(iphdr);

					ether_swap(ehdr);
				}
			}

		}
	}
	return 0;
}

/*
 * The lcore main. This is the main thread that does the work, reading from
 * an input port and writing to an output port.
 */
static __attribute__((noreturn)) void
lcore_main(void)
{
	uint16_t portid=0;
	unsigned long int rxtxcount=0;
	unsigned long int rxtxdrop=0;

	/* Run until the application is quit or killed. */
	for (;;) {
		struct rte_mbuf *bufs[BURST_SIZE];
		uint16_t nb_rx,nb_tx;

	       	nb_rx = rte_eth_rx_burst(portid, 0, bufs, BURST_SIZE);
		if (unlikely(nb_rx == 0)) continue;

		arp_reply(bufs, nb_rx);
		icmp_reply(bufs, nb_rx);

		nb_tx = rte_eth_tx_burst(portid, 0, bufs, nb_rx);

		if (unlikely(nb_tx < nb_rx)) {
			uint16_t buf;
			printf("FORCE REMOVE: %i buffers\n", nb_rx-nb_tx);
			for (buf = nb_tx; buf < nb_rx; buf++) rte_pktmbuf_free(bufs[buf]);
			rxtxdrop+=nb_rx-nb_tx;
		}
		rxtxcount+=nb_tx;
//		if ((rxtxcount%1000)==0) 
//		printf("\rRX/TX count: %li\n", rxtxcount);
	}
}

/*
 * The main function, which does initialization and calls the per-lcore
 * functions.
 */
int
main(int argc, char *argv[])
{
	struct rte_mempool *mbuf_pool;
	unsigned nb_ports;
	uint16_t portid;

	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(argc, argv);
	if (ret < 0) rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	argc -= ret;
	argv += ret;

	/* Check that there is an even number of ports to send/receive on. */
	nb_ports = rte_eth_dev_count();
	if (nb_ports <=0 ) { rte_exit(EXIT_FAILURE, "Error: no dpdk ports\n"); }
	portid=0;

	/* Creates a new mempool in memory to hold the mbufs. */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, 0);
	if (mbuf_pool == NULL) 	rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	if (port_init(portid, mbuf_pool) != 0) rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n", portid);

	if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

	/* Call lcore_main on the master core only. */
	lcore_main();

	return 0;
}
