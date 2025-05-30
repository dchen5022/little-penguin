/*
 * Copyright (c) 2016 Kamal Heib.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

MODULE_AUTHOR("author");
MODULE_DESCRIPTION("helloworld module\n");
MODULE_LICENSE("GPL");


static int hello_probe(struct usb_interface *interface,
	const struct usb_device_id *id)
{
	pr_info("HelloModule: USB keyboard probe function called\n\n");
	return 0;
}

static void hello_disconnect(struct usb_interface *interface)
{
	pr_info("HelloModule: USB keyboard disconnect function called\n\n");
}

static const struct usb_device_id hello_id_table [] = {
        { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,
        		     USB_INTERFACE_SUBCLASS_BOOT,
            		     USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{ } /* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, hello_id_table);

static struct usb_driver hello_driver = {
	.name =		"hello_driver",
	.probe = 	hello_probe,
	.disconnect = 	hello_disconnect,
	.id_table = 	hello_id_table
};


static int __init hello_init(void)
{
	int retval = 0;

	pr_info("HelloModule: Hello World!\n\n");
	retval = usb_register(&hello_driver);
	if (retval < 0) {
		pr_err("HelloModule: usb_register failed. Error number %d\n", retval);
		return -1;
	}

	return 0;
}

static void __exit hello_exit(void)
{
	pr_info("HelloModule: exit\n\n");
	return usb_deregister(&hello_driver);
}


module_init(hello_init);
module_exit(hello_exit);
