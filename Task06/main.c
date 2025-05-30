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

#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

MODULE_AUTHOR("Dennis Chen <dechen@redhat.com>");
MODULE_DESCRIPTION("little penguin Task6");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

uint8_t *id = "682c83e55b77";
#define ID_LEN 12
#define BUF_LEN 50

static ssize_t t6_misc_write(struct file *filp, const char __user *buff,
                             size_t count, loff_t *offp) {
  pr_info("t6_misc_dev: write called");
  uint8_t databuf[BUF_LEN];

  if (copy_from_user(databuf, buff, BUF_LEN)) {
    return -EFAULT;
  }

  /* end string to prevent looking at uninit memory */
  databuf[count] = '\0';

  pr_info("t6_misc_dev: count : %d\n", count);
  pr_info("t6_misc_dev: input len: %d\n", strlen(databuf));
  pr_info("t6_misc_dev: input : %s\n", databuf);
  if (strncmp(databuf, id, ID_LEN) != 0 || strlen(databuf) != ID_LEN) {
    return -EINVAL;
  }
  return count;
}

static ssize_t t6_misc_read(struct file *filp, char __user *buff, size_t count,
                            loff_t *offp) {
  pr_info("t6_misc_dev: read called");
  int error;
  error = copy_to_user(buff, id, ID_LEN + 1);
  if (error)
    return -EFAULT;

  return count;
}

static const struct file_operations t6_misc_fops = {
    .owner = THIS_MODULE, .write = t6_misc_write, .read = t6_misc_read};

struct miscdevice t6_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "eudyptula",
    .fops = &t6_misc_fops,
};

static int __init t6_init(void) {
  int error;
  error = misc_register(&t6_misc);
  if (error) {
    pr_err("t6_misc_dev: misc_register failed with code: %d\n", error);
    return error;
  }

  printk(KERN_DEBUG "t6_misc_dev: t6_misc_dev registered!\n");
  return 0;
}

static void __exit t6_exit(void) {
  misc_deregister(&t6_misc);
  printk(KERN_DEBUG "t6_misc_dev: t6_misc_dev deregistered!\n");
}

module_init(t6_init);
module_exit(t6_exit);
