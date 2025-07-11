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
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

MODULE_AUTHOR("Dennis Chen <dechen@redhat.com>");
MODULE_DESCRIPTION("Little Penguin Task 9");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

#define MODULE_NAME "eudyptula"
#define ID "682c83e55b77"

static char foo_buf[PAGE_SIZE];
static DECLARE_RWSEM(foo_rwlock);

static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr,
		       char *buf)
{
	return sysfs_emit(buf, "%s\n", ID);
}

static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	if (strncmp(ID, buf, strlen(ID)) != 0 || strlen(ID) != strlen(buf))
		return -EINVAL;

	return count;
}

static ssize_t jiffies_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf)
{
	unsigned long jiffies = get_jiffies_64();
	return sysfs_emit(buf, "%lu\n", jiffies);
}

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	int retval;

	down_read(&foo_rwlock);
	retval = sysfs_emit(buf, "%s\n", foo_buf);
	up_read(&foo_rwlock);

	return retval;
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int retval;

	down_write(&foo_rwlock);
	retval = snprintf(foo_buf, count, "%s\n", buf);
	up_write(&foo_rwlock);

	return retval;
}

static struct kobj_attribute id_attr = __ATTR(id, 0644, id_show, id_store);
static struct kobj_attribute jiffies_attr = __ATTR_RO(jiffies);
static struct kobj_attribute foo_attr = __ATTR(foo, 0644, foo_show, foo_store);

static struct attribute *attrs[] = {
	&id_attr.attr, &jiffies_attr.attr, &foo_attr.attr,
	NULL /* Need to terminate list with NULL */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

struct kobject *root;

static int __init t9_init(void)
{
	int retval;
	printk(KERN_INFO "Task 9: Hello World!\n");

	root = kobject_create_and_add("eudyptula", NULL);
	if (!root) {
		printk(KERN_ERR
		       "Task 9: Could not create eudyptula directory\n");
		return -ENOMEM;
	}
	retval = sysfs_create_group(root, &attr_group);
	if (retval) {
		printk(KERN_ERR "Task 9: Could not create attributes\n");
		kobject_put(root);
	}

	return retval;
}

static void __exit t9_exit(void)
{
	printk(KERN_INFO "Task 9: Goodbye!\n");
	kobject_put(root);
}

module_init(t9_init);
module_exit(t9_exit);
