// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
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

static void * versions[MAX_CONTEXT] = {NULL};

static int version_show(struct seq_file *m, void *v)
{
	int index = current_drv_ns_cell_index();
	if(versions[index]){
		seq_printf(m, versions[index]);
		//pr_warn("version\n");
		//printk((char*)versions[index]);
	}

	return 0;
}

static int version_open(struct inode *inode, struct file *file)
{
	return single_open(file, version_show, NULL);
}

static ssize_t version_write(struct file *file, const char __user *buf,
				  size_t count, loff_t *ppos)
{
	int index = current_drv_ns_cell_index();
	if(index <= 0)
		return 0;

	if (count >= 8192)
		return 0;

	if (!versions[index])
		versions[index] = vmalloc(8192);

	memset(versions[index], 0, 8192);

	if (copy_from_user(versions[index], buf, count)) {
		pr_warn("version_write copy_from_user fail\n");
		return 0;
	}

	return count;
}

const struct proc_ops version_proc_fops = {
	.proc_open	= version_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
	.proc_write	= version_write,
};

static char VERSION[8192] = "Linux version 5.10.157 (root@jl) (Android (8490178, based on r450784d) clang version 14.0.6 (https://android.googlesource.com/toolchain/llvm-project 4c603efb0cca074e9238af8b4106c30add4418f6), LLD 14.0.6) #56 SMP PREEMPT Wed Jul 19 19:41:19 CST 2023";

static int __init proc_version_init(void)
{
	versions[0] = VERSION;
	proc_create("version", 0, NULL, &version_proc_fops);
	return 0;
}
fs_initcall(proc_version_init);
