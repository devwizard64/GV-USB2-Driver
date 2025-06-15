// SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause
/*
 * Copyright (c) 2019 Isaac Lozano <109lozanoi@gmail.com>
 *
 * This code is released using a dual license strategy: BSD/GPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of 3-clause BSD License
 * Released under the terms of GNU General Public License Version 2.0
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>

#include "gvusb2.h"
#include "gvusb2-regs.h"

int gvusb2_read_reg(struct gvusb2_dev *dev, u16 reg, u8 *value)
{
	int ret;
	int pipe = usb_rcvctrlpipe(dev->udev, 0);
	u8 *buf;

	buf = kmalloc(sizeof(u8), GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	*value = 0;
	ret = usb_control_msg(dev->udev, pipe, 0x00,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		0x00, reg, buf, sizeof(u8), HZ);
	if (ret < 0) {
		pr_err("gvusb2: read failed on reg 0x%04x (%d)\n",
			reg, ret);
		kfree(buf);
		return ret;
	}

	*value = *buf;

	kfree(buf);

	return 0;
}

int gvusb2_write_reg(struct gvusb2_dev *dev, u16 reg, u8 value)
{
	int ret;
	int pipe = usb_sndctrlpipe(dev->udev, 0);

	ret = usb_control_msg(dev->udev, pipe, 0x01,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value, reg, NULL, 0, HZ);
	if (ret < 0) {
		pr_err("gvusb2: write failed on reg 0x%04x (%d)\n",
			reg, ret);
		return ret;
	}

	return 0;
}

int gvusb2_set_reg_mask(struct gvusb2_dev *dev, u16 reg, u8 mask, u8 value)
{
	int ret;
	u8 reg_val;

	ret = gvusb2_read_reg(dev, reg, &reg_val);
	if (ret < 0)
		return ret;

	ret = gvusb2_write_reg(dev, reg, (reg_val & ~mask) | value);
	if (ret < 0)
		return ret;

	return 0;
}

int gvusb2_init(struct gvusb2_dev *dev, struct usb_device *udev)
{
	dev->udev = udev;
	return 0;
}

int gvusb2_free(struct gvusb2_dev *dev)
{
	/* nothing here, yet */
	return 0;
}

int gvusb2_snd_reset_adc(struct gvusb2_dev *dev)
{
	int ret;

	/* set audio GPIO pins to output */
	ret = gvusb2_set_reg_mask(dev, 0x0002, 0x30, 0x30);
	if (ret < 0)
		return ret;
	ret = gvusb2_set_reg_mask(dev, 0x0000, 0x30, 0x10);
	if (ret < 0)
		return ret;

	/* disable AC97 interface */
	ret = gvusb2_write_reg(dev, 0x0500, 0x00);
	if (ret < 0)
		return ret;

	/* enable I2S interface */
	ret = gvusb2_write_reg(dev, 0x050c, 0x01);
	if (ret < 0)
		return ret;

	return 0;
}
