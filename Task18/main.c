/*
 * Copyright (c) 2025 Dennis Chen.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 3, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "linux/delay.h"
#include "linux/list.h"
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>

MODULE_AUTHOR("Dennis Chen <dechen@redhat.com>");
MODULE_DESCRIPTION("Little Penguin Challenge Task 17");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

#define BUF_LEN 50

#define NAME_LEN 20

struct identity {
	char name[NAME_LEN];
	int  id;
	bool busy;
	struct list_head list;
};

static LIST_HEAD(id_list);

static int flag = 0;

static int identity_create(char *name, int id)
{
	struct identity *new;

	new = kzalloc(sizeof(struct identity), GFP_KERNEL);
	if (!new) {
		return -ENOMEM;
	}
	
	snprintf(new->name, NAME_LEN, "%s", name);
	new->id = id;
	new->busy = false;
	INIT_LIST_HEAD(&new->list);


	list_add(&new->list, &id_list);
	return 0;
}

static struct identity *identity_get(void)
{
	if (list_empty(&id_list))
		return NULL;

	struct identity *next = list_first_entry(&id_list, struct identity, list);
	list_del(&next->list);
	return next;
}

static void identity_destroy_all(void) {
	struct identity *iter, *next;
	list_for_each_entry_safe(iter, next, &id_list, list) {
		list_del(&iter->list);
		kfree(iter);
	}
}

static struct task_struct *eudyptula;
DECLARE_WAIT_QUEUE_HEAD(wee_wait);
int id_counter;

static ssize_t t18_misc_write(struct file *filp, const char __user *ubuf,
			      size_t count, loff_t *offp)
{
	uint8_t databuf[BUF_LEN];
	int err;

	if (count > 19) 
		count = 19;

	err = simple_write_to_buffer(databuf, BUF_LEN, offp, ubuf, count);
	if (err < 0)
		return err;
	/* end string to prevent looking at uninit memory */
	databuf[count] = '\0';

	err = identity_create(databuf, id_counter);
	if (err < 0)
		return err;
	id_counter++;
	pr_info("t18_misc_dev: write: created identity with name=%s id=%d\n", databuf, id_counter - 1);

	flag = 1;
	wake_up_interruptible(&wee_wait);

	return count;
}

static const struct file_operations t18_misc_fops = { .owner = THIS_MODULE,
						      .write = t18_misc_write };

struct miscdevice t18_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "eudyptula",
	.mode = S_IWUGO | S_IRUGO,
	.fops = &t18_misc_fops,
};

static int thread_function(void *_)
{
	struct identity *id;
	unsigned long remainding_ms;
	pr_info("t18_misc_dev: kernel thread created!\n");
	
	while(!kthread_should_stop()) {
		wait_event_interruptible(wee_wait, flag != 0);

		id = identity_get();
		remainding_ms = msleep_interruptible(5000);
		if (remainding_ms > 0) 
			pr_info("t18_misc_dev: kthread: sleep interrupted with %lu ms remaining\n", remainding_ms);

		if (!id)
			continue;

		pr_info("t18_misc_dev: kthread: fetched identity: name=%s, id=%d\n", id->name, id->id);
		kfree(id);
		flag = 0;
	}

	return 0;
}

static int __init t18_init(void)
{
	int error;

	eudyptula = kthread_create(thread_function, NULL, "eudyptula");
	if (IS_ERR(eudyptula)) {
		pr_err("t18_misc_dev: failed to create kernel thread\n");
		return PTR_ERR(eudyptula);
	}

	wake_up_process(eudyptula);

	error = misc_register(&t18_misc);
	if (error) {
		pr_err("t18_misc_dev: misc_register failed with code: %d\n",
		       error);
		kthread_stop(eudyptula);
		return error;
	}

	printk(KERN_DEBUG "t18_misc_dev: t18_misc_dev registered!\n");
	return 0;
}

static void __exit t18_exit(void)
{
	if (eudyptula) {
		kthread_stop(eudyptula);
	}
	misc_deregister(&t18_misc);
	identity_destroy_all();
	printk(KERN_DEBUG "t18_misc_dev: t18_misc_dev deregistered!\n");
}

module_init(t18_init);
module_exit(t18_exit);
