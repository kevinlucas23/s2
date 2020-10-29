#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/path.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/pagemap.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/slab.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin");
MODULE_DESCRIPTION("Print the Process Tree of Linux Project 4 part 1");

#define S2FS_MAGIC 0x19920342

#define TMPSIZE 20

static struct inode* s2fs_make_inode(struct super_block* sb, int mode, const struct file_operations* fops)
{
	struct inode* inode;
	inode = new_inode(sb);
	if (!inode) {
		return NULL;
	}
	inode->i_mode = mode;
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_fop = fops;
	inode->i_ino = get_next_ino();
	return inode;

}

int get_task_info(int pid, char* data) {
	struct task_struct* task;
	struct pid* pid_struct;
	int offset = 0;

	pid_struct = find_get_pid(pid);

	if (pid_struct == NULL) { offset += sprintf(data, "\nTask no longer exists."); goto exit; }

	task = pid_task(pid_struct, PIDTYPE_PID);

	if (task == NULL) { offset += sprintf(data, "\nTask no longer exists."); goto exit; }

	/*offset = sprintf(data, "Task name: %s \nTask State: %ld \nProcess Id: %d \nCPU Id: %u \nTGID: %d"
		"\nParent ID: %d \nStart Time: %llu \nDynamic priority: %d \nStatic Prio: %d \nNormal Priority: %d"
		"\nRT Priority: %d", task->comm, task->state, pid, task->cpu, task->tgid, task->real_parent->pid,
		task->start_time, task->prio, task->static_prio, task->normal_prio, task->rt_priority);*/

	/* null checks */
	/*if (task->active_mm == NULL) {
		offset += sprintf(data + strlen(data), "\nactive mm struct is null!"); goto exit;

	}*/

	/*offset += sprintf(data + strlen(data), "\nMemory Map Base: %lu \nNo.of vmem address %d \nTotal pages mapped: %lu"
		"\nVirtual Memory Usage: %llu \nVirtual mem space: %lu\n", task->active_mm->mmap_base, task->active_mm->map_count,
		task->active_mm->total_vm, task->acct_vm_mem1, task->active_mm->task_size);*/

	offset = sprintf(data, "Hello World!");

exit:
	return offset;

}

static int s2fs_open(struct inode* inode, struct file* filp)
{
	filp->private_data = inode->i_private;
	return 0;
}


static ssize_t s2fs_read_file(struct file* filp, char* buf, size_t count, loff_t* offset)
{
	char* tmp;
	int* pid;
	int len;

	pid = filp->private_data;
	tmp = (char*)kmalloc(500, GFP_KERNEL);
	len = get_task_info(*pid, tmp);

	if (*offset > len)
		return 0;
	if (count > len - *offset)
		count = len - *offset;
	if (copy_to_user(buf, tmp + *offset, count))
		return -EFAULT;
	*offset += count;
	return count;
	//	return simple_read_from_buffer(buf, len, offset, tmp, len);
}

static ssize_t s2fs_write_file(struct file* filp, const char* buf,
	size_t count, loff_t* offset) {
	return 0;
}


static struct file_operations s2fs_file_ops = {
	.open = s2fs_open,
	.read = s2fs_read_file,
	.write = s2fs_write_file,
};

const struct inode_operations lwfs_inode_operations = {
		.setattr = simple_setattr,
		.getattr = simple_getattr,
};

static struct dentry* s2fs_create_file(struct super_block* sb,
	struct dentry* dir, const char* name)
{
	struct dentry* dentry;
	struct inode* inode;
	int pid, ret;
	int* int_pid;

	dentry = d_alloc_name(dir, name);
	if (!dentry)
		goto out;
	inode = s2fs_make_inode(sb, S_IFREG | 0644, &s2fs_file_ops);
	if (!inode)
		goto out_dput;

	ret = kstrtoint(name, 10, &pid);

	int_pid = (int*)kmalloc(sizeof(int), GFP_KERNEL);

	*int_pid = pid;

	inode->i_private = int_pid;

	d_add(dentry, inode);
	return dentry;

out_dput:
	dput(dentry);
out:
	return 0;
}


static struct dentry* s2fs_create_dir(struct super_block* sb,
	struct dentry* parent, const char* name)
{
	struct dentry* dentry;
	struct inode* inode;
	int pid, ret;
	int* int_pid;

	dentry = d_alloc_name(parent, name);
	if (!dentry)
		goto out;

	inode = s2fs_make_inode(sb, S_IFDIR | 0755, &simple_dir_operations);
	if (!inode)
		goto out_dput;
	inode->i_op = &simple_dir_inode_operations;

	ret = kstrtoint(name, 10, &pid);

	int_pid = (int*)kmalloc(sizeof(int), GFP_KERNEL);

	*int_pid = pid;

	inode->i_private = int_pid;

	d_add(dentry, inode);
	return dentry;

out_dput:
	dput(dentry);
out:
	return 0;
}

static struct super_operations s2fs_s_ops = {
	.statfs = simple_statfs,
	.drop_inode = generic_delete_inode,
};

void traverse(struct super_block* sb, struct dentry* parent, struct task_struct* task)
{

	struct dentry* dir;
	char str_pid[6];

	struct list_head* list;
	struct task_struct* task_child;

	snprintf(str_pid, 6, "%ld", (long)task->pid);
	if (!list_empty(&task->children)) {
		dir = s2fs_create_dir(sb, parent, str_pid);
		s2fs_create_file(sb, dir, str_pid);
	}
	else
		s2fs_create_file(sb, parent, str_pid);

	list_for_each(list, &task->children) {

		task_child = list_entry(list, struct task_struct, sibling);
		if (task_child) {

			traverse(sb, dir, task_child);
		}
	}

}

static int s2fs_fill_super(struct super_block* sb, void* data, int silent)
{
	struct inode* root;
	struct dentry* root_dentry;
	struct task_struct* task;

	sb->s_blocksize = VMACACHE_SIZE;
	sb->s_blocksize_bits = VMACACHE_SIZE;
	sb->s_magic = S2FS_MAGIC;
	sb->s_op = &s2fs_s_ops;

	root = s2fs_make_inode(sb, S_IFDIR | 0755, &simple_dir_operations);
	inode_init_owner(root, NULL, S_IFDIR | 0755);
	if (!root)
		goto out;
	root->i_op = &simple_dir_inode_operations;
	//	root->i_fop = &simple_dir_operations;
	set_nlink(root, 2);
	root_dentry = d_make_root(root);
	if (!root_dentry)
		goto out_iput;

	task = &init_task;

	traverse(sb, root_dentry, task);

	sb->s_root = root_dentry;
	return 0;

out_iput:
	iput(root);
out:
	return -ENOMEM;
}


static struct dentry* s2fs_get_super(struct file_system_type* fst,
	int flags, const char* devname, void* data)
{
	return mount_nodev(fst, flags, data, s2fs_fill_super);
}

static struct file_system_type s2fs_type = {
	.owner = THIS_MODULE,
	.name = "s2fs",
	.mount = s2fs_get_super,
	.kill_sb = kill_litter_super,
};


static int __init s2fs_init(void)
{
	return register_filesystem(&s2fs_type);
}

static void __exit s2fs_exit(void)
{
	unregister_filesystem(&s2fs_type);
}

module_init(s2fs_init);
module_exit(s2fs_exit);
