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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin");
MODULE_DESCRIPTION("Print the Process Tree of Linux Project 4 part 1");

void processInfo(struct task_struct* t, int level)
{
	struct task_struct* task;
	struct list_head* list;
	int i = 0;
	/*char offset[50] = "                         ";
	offset[level + 1] = '\0';*/
	for (i = 0; i < level; ++i) {
		printk(KERN_INFO " ");
	}
	// print tree to kernel log
	if (level > 0) {
		printk(KERN_INFO "[%ld]\\__ Name: %s  State: %ld  PID: %ld \n", (long)t->real_parent->pid, t->comm, t->state, (long)t->pid);
	}
	else {
		printk(KERN_INFO "-- Name: %s  State: %ld  PID: %ld \n", t->comm, t->state, (long)t->pid);
	}

	// iterate through children of init process
	// init_task is list_head not a &list_head
	// book is wrong! use '.' not '->'
	list_for_each(list, &t->children) {
		task = list_entry(list, struct task_struct, sibling);
		processInfo(task, level + 2);
	}
	//printk("Process: %s[%d], Parent: %s[%d]\n", task -> comm, task -> pid , task -> parent -> comm, task -> parent -> pid);
	return;
}

//void memAndFileInfo(void)
//{
//	bool found = false;
//	struct task_struct *task;
//	struct task_struct *t = current;
//	struct mm_struct *m;
//	struct vm_area_struct *v;
//	struct files_struct *open_files;
//	struct fdtable *files_table;
//	struct kstat *ks;
//	struct path files_path;
//	unsigned long t_size = 0;
//	rcu_read_lock();
//	for_each_process(task)
//	{
//		task_lock(task);
//		int iid = task -> pid;
//		if(iid == id)
//		{
//			t = task;
//			found = true;
//		}
//		task_unlock(task);
//
//	}
//	rcu_read_unlock();
//	if(found)
//	{
//		printk("//////VIRTUAL MEMORY INFORMATION//////\n\n");
//		v = t -> mm -> mmap;
//		if(v != NULL)
//		{
//			printk("Process: %s[%d]\n",t -> comm, t -> pid);
//		while(v -> vm_next != NULL)
//		{
//			unsigned long size = v -> vm_end - v -> vm_start;
//			t_size = t_size + size;
//			printk("Start: 0x%lx, End: 0x%lx, Block Size: 0x%lx\n", v -> vm_start, v -> vm_end, size);
//			v = v -> vm_next;
//		}
//		printk("Total size of virtual space is: 0x%lx\n",t_size);
//		printk("\n");
//		printk("//////OPEN FILES INFORMATION//////\n\n");
//		printk("Process: %s[%d]\n",t -> comm, t -> pid);
//		int i = 0;
//		open_files = t -> files;
//		files_table = files_fdtable(open_files);
//		char *path;
//		char *buf = (char*)kmalloc(10000*sizeof(char),GFP_KERNEL);
//		if(buf == NULL)
//		{
//			printk("ERROR\n");
//			return 1;
//		}
//		while(files_table -> fd[i] != NULL)
//		{
//			files_path = files_table -> fd[i] -> f_path;
//			char* name = files_table-> fd[i] -> f_path.dentry -> d_iname;
//			long long size = i_size_read(files_table-> fd[i] -> f_path.dentry -> d_inode);
//			path = d_path(&files_path,buf,10000*sizeof(char));
//			printk("Name: %s, FD: %d, Size: 0x%llx bytes, Path: %s\n", name, i, size , path);
//			i++;
//		}
//		}
//	}
//	else
//	{
//		printk("ID not found\n");
//	}
//
//}

static int s2fs_init(void)
{
	printk("\n\n//////PROCESS TREE//////\n\n");
	processInfo(&init_task, 0);
	printk("\n\n");
	// memAndFileInfo();
	return 0;
}

static void s2fs_exit(void)
{
	printk("Done\n");
}

module_init(s2fs_init);
module_exit(s2fs_exit);
