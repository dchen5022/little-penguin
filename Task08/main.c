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

#include "linux/uaccess.h"
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/module.h>

MODULE_AUTHOR("Dennis Chen <dechen@redhat.com>");
MODULE_DESCRIPTION("little penguin Task8");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

#define MODULE_NAME "eudyptula"
#define ID "682c83e55b77"
#define BUF_LEN 50

static char *foo_buf;
static DECLARE_RWSEM(foo_rwlock);

static ssize_t id_read(struct file *filp, char __user *ubuf, size_t count,
                       loff_t *offp) {
  char buf[BUF_LEN];
  snprintf(buf, BUF_LEN, "%s\n", ID);
  return simple_read_from_buffer(ubuf, count, offp, buf, BUF_LEN);
}

static ssize_t id_write(struct file *filp, const char __user *ubuf,
                        size_t count, loff_t *offp) {
  char buf[BUF_LEN];
  int err = simple_write_to_buffer(buf, BUF_LEN, offp, ubuf, count);
  if (err < 0)
    return err;
  if (strncmp(ID, buf, strlen(ID)) != 0 || strlen(ID) != strlen(buf))
    return -EINVAL;
  return count;
}

static const struct file_operations id_fops = {.read = id_read,
                                               .write = id_write};

static ssize_t jf_read(struct file *filp, char __user *ubuf, size_t count,
                       loff_t *offp) {
  unsigned long jiffies = get_jiffies_64();
  char buf[BUF_LEN];
  snprintf(buf, BUF_LEN, "%lu\n", jiffies);
  return simple_read_from_buffer(ubuf, count, offp, buf, BUF_LEN);
}

static const struct file_operations jf_fops = {.read = jf_read};

static ssize_t foo_read(struct file *filp, char __user *ubuf, size_t count,
                        loff_t *offp) {
  if (*offp >= PAGE_SIZE)
    return 0;

  down_read(&foo_rwlock);
  size_t len = (count > PAGE_SIZE) ? PAGE_SIZE : count;

  ssize_t ncopied = copy_to_user(ubuf, foo_buf, len);

  if (ncopied < 0)
    return ncopied;

  *offp += len;
  up_read(&foo_rwlock);

  return len;
}

static ssize_t foo_write(struct file *filp, const char __user *ubuf,
                         size_t count, loff_t *offp) {
  // err if user tries to read more than a page
  if (*offp >= PAGE_SIZE)
    return -ENOMEM;

  down_write(&foo_rwlock);

  size_t len = (count > PAGE_SIZE) ? PAGE_SIZE : count;
  ssize_t ncopied = copy_from_user(foo_buf, ubuf, len);

  if (ncopied < 0)
    return ncopied;

  *offp += len;

  up_write(&foo_rwlock);
  return len;
}

static const struct file_operations foo_fops = {.read = foo_read,
                                                .write = foo_write};

struct dentry *dir;
struct dentry *id_file;
struct dentry *jf_file;
struct dentry *foo_file;

static int __init t8_init(void) {
  printk(KERN_INFO "Task 8: Hello World!\n");

  dir = debugfs_create_dir(MODULE_NAME, NULL);
  if (IS_ERR_VALUE(dir)) {
    pr_err("Task 8: Error creating debugfs directory\n");
    return -ENODEV;
  }

  id_file = debugfs_create_file("id", 0666, dir, NULL, &id_fops);
  if (IS_ERR_VALUE(id_file)) {
    pr_err("Task 8: Error creating debugfs id file\n");
    return -ENODEV;
  }

  jf_file = debugfs_create_file("jiffies", 0444, dir, NULL, &jf_fops);
  if (IS_ERR_VALUE(jf_file)) {
    pr_err("Task 8: Error creating debugfs jiffies file\n");
    return -ENODEV;
  }

  foo_buf = (char *)kzalloc(PAGE_SIZE, GFP_KERNEL);
  if (!foo_buf) {
    return -ENOMEM;
  }

  foo_file = debugfs_create_file("foo", 0644, dir, NULL, &foo_fops);
  if (IS_ERR_VALUE(foo_file)) {
    pr_err("Task 8: Error creating debugfs jiffies file\n");
    kfree(foo_buf);
    return -ENODEV;
  }
  return 0;
}

static void __exit t8_exit(void) {
  printk(KERN_INFO "Task 8: Good bye!\n");
  kfree(foo_buf);
  debugfs_remove_recursive(dir);
}

module_init(t8_init);
module_exit(t8_exit);
