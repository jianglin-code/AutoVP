// SPDX-License-Identifier: GPL-2.0
#include <linux/cpufreq.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#define MAX_CONTEXT 20

#ifdef CONFIG_DRV_NS
#include <linux/drv_namespace.h>
static inline int current_drv_ns_cell_index(void)
{
	char tag[DRV_NS_TAG_LEN];
	int i = 0;

	if(current_drv_ns() == &init_drv_ns)
		return 0;

	for(i = 1; i < MAX_CONTEXT; i++){
		sprintf(tag, "cell%d", i);
		if(strncmp(current_drv_ns()->tag, tag, DRV_NS_TAG_LEN) == 0)
			return i;
	}

	return -1;
}

#else 
static inline int current_drv_ns_cell_index(void)
{
	return 0;
}

#endif

static void * cpuinfos[MAX_CONTEXT] = {NULL};

static int cpuinfo_show(struct seq_file *m, void *v)
{
	int index = current_drv_ns_cell_index();
	if(cpuinfos[index] && index >= 0){
		seq_printf(m, cpuinfos[index]);
		//pr_warn("cpuinfo\n");
		//printk((char*)cpuinfos[index]);
	}

	return 0;
}

static int cpuinfo_open(struct inode *inode, struct file *file)
{
	return single_open(file, cpuinfo_show, NULL);
}

static ssize_t cpuinfo_write(struct file *file, const char __user *buf,
				  size_t count, loff_t *ppos)
{
	int index = current_drv_ns_cell_index();
	if(index <= 0)
		return 0;

	if (count >= 8192)
		return 0;

	if (!cpuinfos[index])
		cpuinfos[index] = vmalloc(8192);

	memset(cpuinfos[index], 0, 8192);

	if (copy_from_user(cpuinfos[index], buf, count)) {
		pr_warn("cpuinfo_write copy_from_user fail\n");
		return 0;
	}

	return count;
}

const struct proc_ops proc_cpuinfo_operations = {
	.proc_open	= cpuinfo_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
	.proc_write	= cpuinfo_write,
};

static char CPUINFOS[8192] = "Processor\t: AArch64 Processor rev 2 (aarch64)\nprocessor\t: 0\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x801\nCPU revision\t: 4\n\nprocessor\t: 1\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x801\nCPU revision\t: 4\n\nprocessor\t: 2\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x801\nCPU revision\t: 4\n\nprocessor\t: 3\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x801\nCPU revision\t: 4\n\nprocessor\t: 4\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x800\nCPU revision\t: 2\n\nprocessor\t: 5\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x800\nCPU revision\t: 2\n\nprocessor\t: 6\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x800\nCPU revision\t: 2\n\nprocessor\t: 7\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x800\nCPU revision\t: 2\n\nHardware\t: Qualcomm Technologies, Inc SDM660\n\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x801\nCPU revision\t: 4\n\nprocessor\t: 3\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x801\nCPU revision\t: 4\n\nprocessor\t: 4\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\nCPU implementer\t: 0x51\nCPU architecture: 8\nCPU variant\t: 0xa\nCPU part\t: 0x800\nCPU revision\t: 2\n\nprocessor\t: 5\nBogoMIPS\t: 38.40\nFeatures\t: fp asimd evtstrm aes pmull sha1";

static int __init proc_cpuinfo_init(void)
{
	cpuinfos[0] = CPUINFOS;
	proc_create("cpuinfo", 0, NULL, &proc_cpuinfo_operations);
	return 0;
}
fs_initcall(proc_cpuinfo_init);
