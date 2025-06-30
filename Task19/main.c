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
#include <linux/ip.h>
#include <linux/textsearch.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_AUTHOR("Dennis Chen <dechen@redhat.com>");
MODULE_DESCRIPTION("Little Penguin Task 19");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

#define t19_info(fmt, ...) pr_info("t19: " fmt, ##__VA_ARGS__)
#define t19_err(fmt, ...) pr_err("t19: " fmt, ##__VA_ARGS__)

static struct ts_config *ts_cfg = NULL;

#define ID_STR "682c83e55b77"

static unsigned int t19_nf_handler(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	unsigned int offset;
	struct iphdr *iph;
	if (skb == NULL) 
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	offset = skb_find_text(skb, 0, skb->len, ts_cfg);
	if (offset != UINT_MAX)
		t19_info("Found %s\n", ID_STR);
	else
		t19_info("Not found\n");
		
	return NF_ACCEPT;
}

static struct nf_hook_ops t19_nfh_ops = {
	.hook = t19_nf_handler,
	.hooknum	= NF_INET_PRE_ROUTING,
	.pf		= NFPROTO_IPV4,
	.priority	= NF_IP_PRI_FIRST,
};

static int __init t19_init(void)
{
	t19_info("Hello World!\n");

	ts_cfg = textsearch_prepare("kmp", ID_STR, strlen(ID_STR), GFP_KERNEL, TS_AUTOLOAD);
	if (IS_ERR(ts_cfg)) {
		t19_err("Failed to initialize textsearch config\n");
		return PTR_ERR(ts_cfg);
	}

	nf_register_net_hook(&init_net, &t19_nfh_ops);
	t19_info("netfilter hook registered!\n");
	return 0;
}

static void __exit t19_exit(void)
{
	nf_unregister_net_hook(&init_net, &t19_nfh_ops);
	textsearch_destroy(ts_cfg);
	t19_info("netfilter hook unregistered!\n");
}

module_init(t19_init);
module_exit(t19_exit);
