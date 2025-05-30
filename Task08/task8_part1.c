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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/debugfs.h>

MODULE_AUTHOR("Dennis Chen <dechen@redhat.com>");
MODULE_DESCRIPTION("little penguin Task8");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

#define MODULE_NAME "eudyptula"
#define ID "682c83e55b77"
#define BUF_LEN 30

static ssize_t t8_read(struct file *filp, char __user *ubuf, size_t count, loff_t *offp)
{
	char buf[BUF_LEN];
	snprintf(buf, sizeof(buf), "%s\n", ID);
	return simple_read_from_buffer(ubuf, count, offp, buf, BUF_LEN);
}

static ssize_t t8_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *offp)
{
	char buf[BUF_LEN];
	int err = simple_write_to_buffer(buf, BUF_LEN, offp, ubuf, count);
	if (err < 0)
		return err;
	if (strncmp(ID, buf, strlen(ID)) != 0 || strlen(ID) != strlen(buf))
		return -EINVAL;
	return count;

}

static const struct file_operations t8_fops = {
	.read = t8_read,
	.write = t8_write
};

struct dentry *dir;
struct dentry *id_file;

static int __init t8_init(void)
{
	printk(KERN_INFO "Task 8: Hello World!\n");

	dir = debugfs_create_dir(MODULE_NAME, NULL);
	if (IS_ERR_VALUE(dir)) {
		pr_err("Task 8: Error creating debugfs directory\n");
		return -EFAULT;
	}

	id_file = debugfs_create_file("id", 0666, dir, NULL, &t8_fops);
	if (IS_ERR_VALUE(id_file)) {
		pr_err("Task 8: Error creating debugfs id file\n");
		return -EFAULT;
	}
	return 0;
}

static void __exit t8_exit(void)
{
	printk(KERN_INFO "Task 8: Good bye!\n");
	debugfs_remove_recursive(dir);
}

module_init(t8_init);
module_exit(t8_exit);
