CC=gcc
CFLAGS=$(shell pkg-config --cflags liburing) -I/usr/include -g -Wall -mssse3
LIBS=$(shell pkg-config --libs liburing) -lpthread \
    -Wl,--as-needed -L/usr/local/lib/x86_64-linux-gnu \
    -lrte_node -lrte_graph -lrte_pipeline -lrte_table -lrte_pdump \
    -lrte_port -lrte_fib -lrte_pdcp -lrte_ipsec -lrte_vhost -lrte_stack \
    -lrte_security -lrte_sched -lrte_reorder -lrte_rib -lrte_mldev \
    -lrte_regexdev -lrte_rawdev -lrte_power -lrte_pcapng -lrte_member \
    -lrte_lpm -lrte_latencystats -lrte_jobstats -lrte_ip_frag -lrte_gso \
    -lrte_gro -lrte_gpudev -lrte_dispatcher -lrte_eventdev -lrte_efd \
    -lrte_dmadev -lrte_distributor -lrte_cryptodev -lrte_compressdev \
    -lrte_cfgfile -lrte_bpf -lrte_bitratestats -lrte_bbdev -lrte_acl \
    -lrte_timer -lrte_hash -lrte_metrics -lrte_cmdline -lrte_pci \
    -lrte_ethdev -lrte_meter -lrte_net -lrte_mbuf -lrte_mempool \
    -lrte_rcu -lrte_ring -lrte_eal -lrte_telemetry -lrte_argparse \
    -lrte_kvargs -lrte_log -lbsd

SRCS=main.c ringbuf.c dpdk_rx.c vm_writer.c
OBJS=$(SRCS:.c=.o)
TARGET=dpdk_vm_demo

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -f *.o $(TARGET)