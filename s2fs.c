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
MODULE_DESCRIPTION("Print the Process Tree of Linux Project 4 part 4");

void view_tree(struct task_struct* t, int pos) {
    struct task_struct* task;
    struct list_head* list;

    char offset[50];
    memset(offset, ' ', 50);
    offset[pos + 1] = '\0';

    // print tree to kernel log
    if (pos > 0) {
        printk(KERN_INFO "%s%s [%d]\n", offset, t->comm, t->pid);
    }
    else {
        printk(KERN_INFO "%s%s [%d]\n", offset, t->comm, t->pid);
    }

    // iterate through children of init process
    list_for_each(list, &t->children) {
        task = list_entry(list, struct task_struct, sibling);
        view_tree(task, pos + 2);
    }
    return;
}

static int s2fs_init(void)
{

    struct task_struct* task;
    task = &init_task;
    printk("Loading proctree Module...\n");
    view_tree(task, 0);
    return 0;
}

static void s2fs_exit(void)
{
    printk("\n\nAll tree printed above.\n\n");
}

module_init(s2fs_init);
module_exit(s2fs_exit);