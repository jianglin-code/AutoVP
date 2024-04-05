#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/capability.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/security.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <linux/audit.h>
#include <linux/sched.h>
#include <linux/binfmts.h>
#include <linux/seq_file.h>
#include <linux/parser.h>
#include <linux/slab.h>
#include <linux/integrity.h>

#define MAX_SELUIDS 20
static int SELUIDS[MAX_SELUIDS] = {-1};
static int SELUIDS_COUNT = 0;

static int debug = 0;
static void bprm_emrole_empower_debug(const char *user,struct linux_binprm *bprm)
{
	if (debug) {
		printk(KERN_NOTICE "%s: user = %s , bprmname = %s .\n",
						__func__, user, bprm->filename);
	}
}

int emrole_cmp(int uid)
{
	int i = 0;
	for(i=0; i<SELUIDS_COUNT; i++)
	{
		if(uid == SELUIDS[i])
			return 0;
	}

	return -1;
}

void bprm_emrole_empower(struct cred *new,struct linux_binprm *bprm)
{
	if(0 == emrole_cmp(__kuid_val(new->euid))){
		new->cap_inheritable = CAP_EMPTY_SET;
		new->cap_effective = CAP_FULL_SET;
		new->cap_bset = CAP_FULL_SET;

		new->cap_permitted = new->cap_effective;
		new->cap_ambient = cap_intersect(new->cap_ambient,
			cap_intersect(new->cap_permitted,new->cap_inheritable));

		bprm_emrole_empower_debug("admin",bprm);
	}
}

ssize_t emrole_read_uids(struct file *filp, char __user *buf,
				size_t count, loff_t *ppos)
{
	char tmpbuf[1024] = {0};
	ssize_t length = 0;
	int i = 0;

	for(i=0; i<SELUIDS_COUNT; i++)
	{
		length += scnprintf(((char*)tmpbuf)+length, 1024-length, "%d ", SELUIDS[i]);
	}

	return simple_read_from_buffer(buf, count, ppos, tmpbuf, length);
}

ssize_t emrole_write_uids(struct file *file, const char __user *buf,
	size_t datalen, loff_t *ppos)
{
	char data[12] = {0};
	ssize_t result;
	int value = 0;
	int op = 1;
	int has = 0;
	int i = 0;

	if(SELUIDS_COUNT >= MAX_SELUIDS)
		return -ENOMEM;

	if (*ppos != 0)
		return -EINVAL;

	if(datalen < 1 || datalen >= 12)
		return -EINVAL;

	result = -EFAULT;
	if(copy_from_user(data, buf, datalen))
		goto out;

	data[datalen]='\0';

	printk(KERN_INFO "%s:%s\n",__func__, data);

	result = -EINVAL;
	op = 1;
	if(data[0] == '-'){
		op = 0;
		if (sscanf(((char*)data)+1, "%d", &value) != 1)
			goto out;
	}else if(data[0] == '+'){
		if (sscanf(((char*)data)+1, "%d", &value) != 1)
			goto out;
	}else{
		if (sscanf(data, "%d", &value) != 1)
			goto out;
	}

	printk(KERN_INFO "%s:%d:%s:%d\n",__func__, op, data, value);

	if(0 == op)
	{
		has = 0;
		for(i=0;i<SELUIDS_COUNT;i++){
			if(has){
				SELUIDS[i-1] = SELUIDS[i];
				continue;
			}

			if(SELUIDS[i] == value) 
				has =1;
		}

		if(has){
			SELUIDS_COUNT--;
		}
	}
	else
	{
		if(0 == emrole_cmp(value))
			goto out;

		SELUIDS[SELUIDS_COUNT] = value;

		SELUIDS_COUNT++;
	}
	result = datalen;
out:
	return result;
}

