/**************************************************************************
$Id$
***************************************************************************/

#define RX_RING_PACKETS 16384

#include ".git.h"

#include "spi-slave.h"
#include "spi-slave-hw.h"
#include "kdbg.h"

#include <asm/byteorder.h>

#include <linux/circ_buf.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/property.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/platform_data/dma-imx.h>

#include <linux/workqueue.h>
#include <linux/ktime.h>

#include <linux/kthread.h>
#include <linux/sched.h>

#define DRIVER_NAME DEVICE

#define PACKET_SIZE_DWORD  (PACKET_SIZE/sizeof(u32))

DEBUG_PARAM(0);

struct spi_imx_statistic {
	unsigned long long pkt_ok;
	unsigned long long pkt_lost;
	unsigned pkt_err;
	unsigned stream_err;
	unsigned overruns;
	unsigned max_ring_used;
	unsigned long long dwords;
	unsigned long long skip;
	unsigned long last_message_jiffies;
};

struct spi_imx_data {
	struct spi_bitbang bitbang;
	struct device *dev;
	struct task_struct *rx_thrd;
	
	void __iomem *base;
	unsigned long base_phys;

	struct clk *clk_per;
	struct clk *clk_ipg;
	unsigned long spi_clk;
	unsigned int spi_bus_clk;

	spinlock_t rx_lock;
	wait_queue_head_t rx_wq;
	struct circ_buf rx_ring;
	u8 *packets;
	struct spi_imx_statistic stats;
};
//-------------------------------------------------------------------------
// rx_ring utils
// rx_ring.buf contets 2 integer arrays
#define RXRING_BUFFER_SIZE (sizeof(int)*2*RX_RING_PACKETS)
#define RXRING_ALLOC(C) { (C).buf = kzalloc(RXRING_BUFFER_SIZE, GFP_KERNEL ); }
#define RXRING_FREE(C) kfree((C).buf)
#define RXRING_USED(C) CIRC_CNT((C).head,(C).tail,RX_RING_PACKETS)
#define RXRING_VACANT(C)  CIRC_SPACE((C).head,(C).tail,RX_RING_PACKETS)
#define RXRING_ADVANCE(C,end) (C).end=((C).end+1)&(RX_RING_PACKETS-1)
#define RXRING_IDX(C,end)  ((int *)(C).buf)[(C).end]
#define RXRING_HEAD_IDX(C)  RXRING_IDX(C,head)
#define RXRING_TAIL_IDX(C)  RXRING_IDX(C,tail)
#define RXRING_FLAG(C,idx) ((int *)(C).buf)[idx+RX_RING_PACKETS]
#define RXRING_REMOVE(C)  { RXRING_FLAG(C,RXRING_TAIL_IDX(C))=0; RXRING_TAIL_IDX(C)=-1; RXRING_ADVANCE(C,tail); }
#define RXRING_INSERT(C, idx)  { RXRING_HEAD_IDX(C)=idx; RXRING_FLAG(C,RXRING_HEAD_IDX(C))=1; RXRING_ADVANCE(C,head);}
#define RXRING_CLEAR(C) { (C).tail=(C).head=0;  memset((C).buf,0,RXRING_BUFFER_SIZE);}
//-------------------------------------------------------------------------
static inline int rxring_get_free_idx(struct circ_buf *crb)
{
	int idx =-1;
	int i = 0;
	while(i<RX_RING_PACKETS && RXRING_FLAG(*crb,i)) i++;
	if(i<RX_RING_PACKETS) idx = i;
	return idx;
}
//-------------------------------------------------------------------------
/*
static int paket_n_chk(u32 *packet)
{
	static unsigned last_valid;
	static u8 last;
	u8 nbeg = (packet[0]>>16)&0xFF;
	u8 nend = (packet[46]>>8)&0xFF;
	int rv = (nbeg==nend)?1:-1;
	if(rv) {
		if(last_valid) {
			rv = ++last==nbeg?1:-1;
			if(rv<0) {
				last_valid=0;
				//TRACE("0x%08X=>0x%08X 0x%02X 0x%02X => %d" ,packet[0],packet[46], nbeg, last, rv);
			}
		} else {
			last=nbeg;
			last_valid=1;
		}
	}
	//if(rv<0) TRACE("0x%08X=>0x%08X 0x%02X 0x%02X => %d" ,packet[0],packet[46], nbeg, nend, rv);
	return rv;
}*/

//-------------------------------------------------------------------------
static int paketizer(u32 val,u32 *packet, unsigned *pcnt)
{
	unsigned cnt = *pcnt;
	int rv = 0;
	
	packet[cnt++] = val;
	if(cnt==1) {
		if(((val>>24)&0xFF)!=0x47) {
			cnt=0;
			rv = -3;
		}
	} else if(cnt==PACKET_SIZE_DWORD) {
/*
		if((val&0xFF)==0x74 ) {
				rv = paket_n_chk(packet);
		} else {
			rv = -2;
		}
*/
		rv = 1;
		cnt=0;
	}
	*pcnt = cnt;
	return rv;
}
//-------------------------------------------------------------------------
int receive_packet(struct spi_imx_data *spi_imx)
{
	int  idx;
	u32  rxcnt, stat, used;
	u32 *pkti;
	unsigned bytes_cnt = 0;
	int rv = -1;

	spin_lock(&spi_imx->rx_lock);
	if(!RXRING_VACANT(spi_imx->rx_ring)) {
		// drop oldest unreaded packet
		spi_imx->stats.pkt_lost++;
		RXRING_REMOVE(spi_imx->rx_ring);
	}
	idx = rxring_get_free_idx(&spi_imx->rx_ring);
	//TRACE("RXRING vacant:%d rxring_get_free_idx:%d",RXRING_VACANT(spi_imx->rx_ring),idx);
	spin_unlock(&spi_imx->rx_lock);

	if(idx<0) {
		// Can not be here, but...
		ERROR("No free space for packets!");
		return -EFAULT;
	}

	if(spi_imx->stats.pkt_lost > 1024) {
		// Too RX packets lost, probably nobody read packets
		DBG("Too RX packets lost");
		return rv;
	}

	pkti = (u32 *)(spi_imx->packets + idx*PACKET_SIZE);

	while(!kthread_should_stop() && rv<=0) {
		rxcnt = mx51_ecspi_rxcnt(spi_imx->base);
		stat = mx51_ecspi_stat(spi_imx->base);
		if(stat&(1<<6))  {
			mx51_ecspi_stat_clr(spi_imx->base); // Clear bits TC and RO
			spi_imx->stats.overruns++;
		}
		while (mx51_ecspi_rx_available(spi_imx->base)) {
			rv = paketizer(mx51_ecspi_data(spi_imx->base),pkti,&bytes_cnt);
			spi_imx->stats.dwords++;
			if(rv>0) {
				spi_imx->stats.pkt_ok++;
				break;
			} else switch(rv){
				case -1:
					spi_imx->stats.pkt_err++;
					break;
				case -2:
					spi_imx->stats.stream_err++;
					break;
				case -3:
					spi_imx->stats.skip++;
					break;
			}
		}
		schedule();
	}
	spin_lock(&spi_imx->rx_lock);
	RXRING_INSERT(spi_imx->rx_ring, idx);
	used = RXRING_USED(spi_imx->rx_ring);
	if(used > spi_imx->stats.max_ring_used) spi_imx->stats.max_ring_used = used;
	spin_unlock(&spi_imx->rx_lock);
	wake_up(&spi_imx->rx_wq);
	return rv;
}
//-------------------------------------------------------------------------
static int rx_thrd_fn(void *arg)
{
	struct spi_imx_data *spi_imx = (struct spi_imx_data *)arg;

	DBG("RX thread start:%s pid:%d", current->comm, current->pid);

	memset(&spi_imx->stats,0,sizeof(spi_imx->stats));
	spin_lock(&spi_imx->rx_lock);
	RXRING_CLEAR(spi_imx->rx_ring);
	spin_unlock(&spi_imx->rx_lock);
	
	while(receive_packet(spi_imx)>0) {
		if(spi_imx->stats.pkt_ok%100000 == 0) {
			TRACE("OK:%llu Lost:%llu over: %u err: pkt:%u/stream:%u skip: %llu", 
				spi_imx->stats.pkt_ok, spi_imx->stats.pkt_lost,
				spi_imx->stats.overruns,
				spi_imx->stats.pkt_err, spi_imx->stats.stream_err,
				spi_imx->stats.skip);
		}
	}
	DBG("RX thread exit:%s pid:%d", current->comm, current->pid);
	spi_imx->rx_thrd = NULL;
	return 0;
}
//-------------------------------------------------------------------------
static void memcpy_bswap_packet(void *dst,const void *src)
{
	//__bswapdi2();
	u32 *pdsti = (u32 *)dst;
	const u32 *psrci = (const u32 *)src;
	unsigned i;
	for(i=0;i<PACKET_SIZE_DWORD;i++) {
		*pdsti++ =  swab32(*psrci++);
	}
}
//-------------------------------------------------------------------------
static int init_rx_thread(struct spi_imx_data *spi_imx)
{
	int ret;
	struct task_struct *thrd = NULL;

	thrd = kthread_create(rx_thrd_fn,spi_imx,DEVICE "-rx");
	if(IS_ERR(thrd)) {
		ERROR("RX thread create failed:%d",ret);
		ret = PTR_ERR(thrd);
		thrd = NULL;
	} else {
		// Set very low priority...
		sched_set_normal(thrd,MAX_NICE);
		//sched_set_normal(thrd,99);
		//sched_set_fifo_low(thrd);
		ret = 0;
		wake_up_process(thrd);
	}
	spi_imx->rx_thrd = thrd;
	return ret;
}
//-------------------------------------------------------------------------
static int spi_imx_prepare_message(struct spi_master *master, struct spi_message *msg)
{
	struct spi_imx_data *spi_imx = spi_master_get_devdata(master);
	int ret = 0;
	//ktime_t now =  ktime_get();
	//unsigned ns_elapsed = (unsigned)ktime_to_ns(ktime_sub(now,spi_imx->stats.last_message));
	unsigned long now_jiffies = jiffies;
	unsigned delta = now_jiffies - spi_imx->stats.last_message_jiffies;
	if(delta > 4*HZ ) { // 4s
		memset(&spi_imx->stats,0,sizeof(spi_imx->stats));
		spin_lock(&spi_imx->rx_lock);
		RXRING_CLEAR(spi_imx->rx_ring);
		spin_unlock(&spi_imx->rx_lock);
		DBG("Reset stats at %lu after %u sec (%u jiffies), HZ:%u",now_jiffies,delta/HZ,delta,HZ);
	}
	//spi_imx->stats.last_message = now;
	spi_imx->stats.last_message_jiffies = now_jiffies;
	
	if(!spi_imx->rx_thrd) {
		ret = init_rx_thread(spi_imx);
	}
	return ret;
}
//-------------------------------------------------------------------------
/* About prepare_to_wait/finish_wait refer to LDD2 6.2.5.2 Manual sleeps */
static int spi_imx_transfer(struct spi_device *spi,
				struct spi_transfer *transfer)
{
	unsigned n_ready;
	int idx = -99;
	u8 *pkt;
	struct spi_imx_data *spi_imx = spi_master_get_devdata(spi->master);

	spin_lock(&spi_imx->rx_lock);
	n_ready = RXRING_USED(spi_imx->rx_ring);
	while (n_ready == 0) {
		DEFINE_WAIT(wait);
		spin_unlock(&spi_imx->rx_lock);
		prepare_to_wait(&spi_imx->rx_wq,&wait, TASK_INTERRUPTIBLE);
		schedule();
		finish_wait(&spi_imx->rx_wq,&wait);
		if(signal_pending(current)) {
			return -ERESTARTSYS;
		}
		spin_lock(&spi_imx->rx_lock);
		n_ready = RXRING_USED(spi_imx->rx_ring);
	}
	if(n_ready>0) {
		idx = RXRING_TAIL_IDX(spi_imx->rx_ring);
		if(idx>=0) {
			pkt = spi_imx->packets + idx*PACKET_SIZE;
			memcpy_bswap_packet(transfer->rx_buf,pkt);
			RXRING_REMOVE(spi_imx->rx_ring);
		} else {
			ERROR("Bad RX ring: ready:%u idx:%i",n_ready,idx);
			return -EFAULT;
		}
	}
	spin_unlock(&spi_imx->rx_lock);
	{
		u32 *packet = (u32 *)transfer->rx_buf;
		TRACE("len:%u to: %px, ready:%u idx:%i 0x%08X=>0x%08X OK!", (unsigned)transfer->len, transfer->rx_buf, n_ready, idx,packet[0],packet[46]);
	}
	return transfer->len;
}
//-------------------------------------------------------------------------
static int proc_status_show(struct seq_file *m, void *v)
{
	struct spi_imx_data *spi_imx = m->private;
	seq_printf(m,"OK:%llu Lost:%llu packets, overruns:%u err: pkt:%u/stream:%u skip: %llu bytes, max buffers:%u %c\n",
		spi_imx->stats.pkt_ok, spi_imx->stats.pkt_lost,
		spi_imx->stats.overruns,
		spi_imx->stats.pkt_err, spi_imx->stats.stream_err,
		spi_imx->stats.skip,spi_imx->stats.max_ring_used,
		spi_imx->rx_thrd?'R':'-');
	spi_imx->stats.max_ring_used = 0;
	return 0;
}      
//-------------------------------------------------------------------------
static const struct of_device_id spi_imx_dt_ids[] = {
	{ .compatible = "fsl,imx8mm-ecspi"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, spi_imx_dt_ids);
//-------------------------------------------------------------------------
static int spi_imx_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = pdev->dev.of_node;
	struct spi_master *master;
	struct spi_imx_data *spi_imx;
	struct resource *res;
	int irq;
	const char *compatible = NULL;
	
	of_property_read_string(pdev->dev.of_node, "compatible", &compatible);
	INFO("Probe (GIT Rev." VERSION ") [Build: " __TIME__ " " __DATE__ "] Device:%s(%s) compat: %s",
	np->name, np->full_name, compatible);

	master = spi_alloc_slave(&pdev->dev, sizeof(struct spi_imx_data));
	if (!master)
		return -ENOMEM;
	platform_set_drvdata(pdev, master);

	master->bits_per_word_mask = SPI_BPW_RANGE_MASK(1, 32);
	master->bus_num = np ? -1 : pdev->id;
	master->use_gpio_descriptors = true;

	spi_imx = spi_master_get_devdata(master);
	spi_imx->bitbang.master = master;
	spi_imx->dev = &pdev->dev;

	spi_imx->bitbang.txrx_bufs = spi_imx_transfer;
	spi_imx->bitbang.master->prepare_message = spi_imx_prepare_message;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	spi_imx->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(spi_imx->base)) {
		ret = PTR_ERR(spi_imx->base);
		goto out_master_put;
	}
	spi_imx->base_phys = res->start;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		ret = irq;
		goto out_master_put;
	}

	spi_imx->clk_ipg = devm_clk_get(&pdev->dev, "ipg");
	if (IS_ERR(spi_imx->clk_ipg)) {
		ret = PTR_ERR(spi_imx->clk_ipg);
		goto out_master_put;
	}

	spi_imx->clk_per = devm_clk_get(&pdev->dev, "per");
	if (IS_ERR(spi_imx->clk_per)) {
		ret = PTR_ERR(spi_imx->clk_per);
		goto out_master_put;
	}

	ret = clk_prepare_enable(spi_imx->clk_per);
	if (ret)
		goto out_master_put;

	ret = clk_prepare_enable(spi_imx->clk_ipg);
	if (ret)
		goto out_put_per;

	spi_imx->spi_clk = clk_get_rate(spi_imx->clk_per);

	mx51_ecspi_reset(spi_imx->base);
	mx51_ecspi_init(spi_imx->base);
	
	TRACE("Initial stat 0x%02X rxcnt:%u",mx51_ecspi_stat(spi_imx->base),mx51_ecspi_rxcnt(spi_imx->base));
	mx51_ecspi_stat_clr(spi_imx->base);

	master->dev.of_node = pdev->dev.of_node;
	ret = spi_bitbang_start(&spi_imx->bitbang);
	if (ret) {
		dev_err_probe(&pdev->dev, ret, "bitbang start failed\n");
		goto out_bitbang_start;
	}
	
	spi_imx->packets = kzalloc(PACKET_SIZE*RX_RING_PACKETS, GFP_KERNEL );
	RXRING_ALLOC(spi_imx->rx_ring);
	spin_lock_init(&spi_imx->rx_lock);
	init_waitqueue_head(&spi_imx->rx_wq);
	
	if (!proc_create_single_data(DEVICE, S_IRUGO, NULL, proc_status_show,spi_imx)) {
		ret = -ENOMEM;  
		goto out_proc_create;
	}
	TRACE("spi_imx: %px",spi_imx);
	return ret;

out_proc_create:
	kfree(spi_imx->packets);
	RXRING_FREE(spi_imx->rx_ring);
out_bitbang_start:
	clk_disable_unprepare(spi_imx->clk_ipg);
out_put_per:
	clk_disable_unprepare(spi_imx->clk_per);
out_master_put:
	spi_master_put(master);

	return ret;
}
//-------------------------------------------------------------------------
static int spi_imx_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct spi_master *master = platform_get_drvdata(pdev);
	struct spi_imx_data *spi_imx = spi_master_get_devdata(master);

	remove_proc_entry(DEVICE, NULL);
	if(spi_imx->rx_thrd) {
		kthread_stop(spi_imx->rx_thrd);
	}

	spi_bitbang_stop(&spi_imx->bitbang);
	mx51_ecspi_disable(spi_imx->base);
	spi_master_put(master);
	RXRING_FREE(spi_imx->rx_ring);
	kfree(spi_imx->packets);
	INFO("Removed (GIT Rev." VERSION ") [Build: " __TIME__ " " __DATE__ "]" );
	
	return ret;
}
//-------------------------------------------------------------------------
static struct platform_driver spi_imx_driver = {
	.driver = {
		   .name = DRIVER_NAME,
		   .of_match_table = spi_imx_dt_ids,
		 //  .pm = &imx_spi_pm,
	},
	.probe = spi_imx_probe,
	.remove = spi_imx_remove,
};
module_platform_driver(spi_imx_driver);
//-------------------------------------------------------------------------
MODULE_DESCRIPTION("IMX8 SPI slave driver");
MODULE_AUTHOR("AlexVol");
MODULE_LICENSE("GPL");

// to suppress "loading out-of-tree module taints kernel." warning
MODULE_INFO(intree,"Y");
//-------------------------------------------------------------------------

/*
Some useful shell commands:

while true; do cat /proc/spi-slave; sleep 1; done
while true; do dd if=/dev/spits1.0  bs=3008 count=128 | hexdump -e '47/1 "%02x ""\n"' | grep 47; done

*/
