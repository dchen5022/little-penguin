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

#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_AUTHOR("Dennis Chen <dechen@redhat.com>");
MODULE_DESCRIPTION("Little Penguin Task 12");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

#define NAME_LEN 20

struct identity {
	char name[NAME_LEN];
	int id;
	bool busy;
	struct list_head list;
};

static LIST_HEAD(id_list);
static struct kmem_cache *cache;

static int identity_create(char *name, int id)
{
	struct identity *new;

	new = kmem_cache_alloc(cache, GFP_KERNEL);
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

static struct identity *identity_find(int id)
{
	struct identity *iter;
	list_for_each_entry(iter, &id_list, list) {
		if (iter->id == id)
			return iter;
	}
	return NULL;
}

static void identity_destroy(int id)
{
	struct identity *iter, *next;
	list_for_each_entry_safe(iter, next, &id_list, list) {
		if (iter->id == id) {
			list_del(&iter->list);
			kmem_cache_free(cache, iter);
		}
	}
}

static int __init t1_init(void)
{
	printk(KERN_DEBUG "Task 12: Hello World!\n");
	cache = kmem_cache_create("eudyptula", sizeof(struct identity), 0,
				  SLAB_POISON, NULL);

	struct identity *temp;

	identity_create("Alice", 1);
	identity_create("Bob", 2);
	identity_create("Dave", 3);
	identity_create("Gena", 10);

	temp = identity_find(3);
	printk(KERN_DEBUG "Task 12: id 3 = %s\n", temp->name);

	temp = identity_find(42);
	if (temp == NULL)
		pr_debug("Task 12: id 42 not found\n");

	identity_destroy(2);
	identity_destroy(1);
	identity_destroy(10);
	identity_destroy(42);
	identity_destroy(3);

	return 0;
}

static void __exit t1_exit(void)
{
	struct identity *iter, *next;
	list_for_each_entry_safe(iter, next, &id_list, list) {
		list_del(&iter->list);
		kmem_cache_free(cache, iter);
	}
	kmem_cache_destroy(cache);
	printk(KERN_DEBUG "Task 12: Good bye!\n");
}

module_init(t1_init);
module_exit(t1_exit);
