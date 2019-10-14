/************************************************************************************
** File:  \\192.168.144.3\Linux_Share\12015\ics2\development\mediatek\custom\oppo77_12015\kernel\battery\battery
** VENDOR_EDIT
** Copyright (C), 2008-2012, OPPO Mobile Comm Corp., Ltd
**
** Description:
**          for tps6128xd buck
**
** Version: 1.0
** Date created: 09:03:46, 06/11/2018
** Author: lfc@BSP.CHG.Basic
**
** --------------------------- Revision History: ------------------------------------------------------------
* <version>           <date>                <author>                             <desc>
* Revision 1.0        2018-11-06        lfc@BSP.CHG.Basic          Created for tps6128xd driver
************************************************************************************************************/
#include <linux/i2c.h>
#include <linux/debugfs.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <soc/oppo/device_info.h>
#include <soc/oppo/oppo_project.h>

#define CONFIG_REGISTER		0x01
#define VOUTFLOORSET_REG	0x02
#define VOUTROOFSET_REG		0x03
#define ILIMSET_REG			0x04
#define STATUS_REG			0x05
#define E2PROMCTRL_REG		0xFF

struct tps6128xd_dev_info {
	struct i2c_client		*client;
	struct device			*dev;
};

static int tps6128xd_read(struct tps6128xd_dev_info *di, u8 reg, u8 *data)
{
	int ret;

	ret = i2c_smbus_read_byte_data(di->client, reg);
	if (ret < 0)
		return ret;

	*data = ret;
	return 0;
}

static int tps6128xd_write(struct tps6128xd_dev_info *di, u8 reg, u8 data)
{
	return i2c_smbus_write_byte_data(di->client, reg, data);
}

#if 0
static int tps6128xd_write_mask(struct tps6128xd_dev_info *di, u8 reg,
		u8 mask, u8 data)
{
	u8 v = 0;
	int ret;

	ret = tps6128xd_read(di, reg, &v);
	pr_err("%s reg:0x%x, v:0x%x\n", __func__, reg, v);
	if (ret < 0)
		return ret;

	v &= ~mask;
	v |= (data & mask);

	return tps6128xd_write(di, reg, v);
}
#endif

static void tps6128xd_hw_init(struct tps6128xd_dev_info *di)
{
	int ret = 0;
	u8 buf[5] = {0x0};

	//VSEL == 0 && VOL > 3V, bypass mode
	ret = tps6128xd_write(di, VOUTFLOORSET_REG, 0x03);
	if (ret < 0) {
		pr_err("%s write voutfloor fail, ret:%d\n", __func__, ret);
	}
	tps6128xd_read(di, VOUTFLOORSET_REG, &buf[0]);

	//VSEL == 1 && VOL > 4.3V, boost mode
	tps6128xd_write(di, VOUTROOFSET_REG, 0x1D);
	tps6128xd_read(di, VOUTROOFSET_REG, &buf[1]);

	//limit current when current > 5A
	tps6128xd_write(di, ILIMSET_REG, 0x1F);
	tps6128xd_read(di, ILIMSET_REG, &buf[2]);

	//read status reg
	tps6128xd_read(di, STATUS_REG, &buf[3]);
	
	pr_err("%s voutfloor:0x%x, voutroof:0x%x, ilim:0x%x, status:0x%x\n",
		__func__, buf[0], buf[1], buf[2], buf[3]);
}

static int tps6128xd_driver_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct tps6128xd_dev_info *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di) {
		dev_err(dev, "can't alloc tps6128xd di struct\n");
		return -ENOMEM;
	}
	i2c_set_clientdata(client, di);
	di->client = client;
	di->dev = dev;
	
	tps6128xd_hw_init(di);
	
	return 0;
}
/**********************************************************
  *
  *   [platform_driver API]
  *
  *********************************************************/


static const struct of_device_id tps6128xd_match[] = {
        { .compatible = "oppo,tps6128xd-rf"},
        { },
};

static const struct i2c_device_id tps6128xd_id[] = {
        { "tps6128xd-rf", 0},
        {},
};
MODULE_DEVICE_TABLE(i2c, tps6128xd_id);

static struct i2c_driver tps6128xd_i2c_driver = {
        .driver                = {
                .name = "tps6128xd-rf",
                .owner        = THIS_MODULE,
                .of_match_table = tps6128xd_match,
        },
        .probe                = tps6128xd_driver_probe,
        .id_table        = tps6128xd_id,
};

module_i2c_driver(tps6128xd_i2c_driver);
MODULE_DESCRIPTION("Driver for tps6128xd rf chip");
MODULE_LICENSE("GPL v2");
