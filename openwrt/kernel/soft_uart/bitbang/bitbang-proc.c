/**************************************************************************
$Id$
***************************************************************************/

#include "kdbg.h"

#include "bitbang.h"

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

//-------------------------------------------------------------------------
static int bitbang_proc_show(struct seq_file *m, void *v)
{
	struct bitbang_priv *priv = m->private;
	seq_printf(m, "Total TX: %d\n"
		"Total RX: %d\n"
		"RX errors:\n"
		"start %d:\n"
		"stop  %d:\n"
		"parity  %d:\n", 
		priv->stats.tx_counter, priv->stats.rx_counter,
		priv->stats.rx_errs_start, priv->stats.rx_errs_stop, priv->stats.rx_errs_parity);
	return 0;
}
//-------------------------------------------------------------------------
static int bitbang_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, bitbang_proc_show, pde_data(inode));
}
//-------------------------------------------------------------------------
static const struct proc_ops bitbang_proc_ops = {
	.proc_open    = bitbang_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};
//-------------------------------------------------------------------------
int bitbang_proc_init(struct bitbang_priv *priv)
{
	int ret = 0;

	priv->proc_entry = proc_create_data(DEVICE, 0444, NULL, &bitbang_proc_ops, priv);
	if (!priv->proc_entry) {
		WARNING("Proc entry create failed");
		//ret = -ENOMEM;
	}	
	return ret;
}
//-------------------------------------------------------------------------
void bitbang_proc_exit(struct bitbang_priv *priv)
{
	if(priv->proc_entry) 
		proc_remove(priv->proc_entry);
}
//-------------------------------------------------------------------------
