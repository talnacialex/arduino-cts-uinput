#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/uaccess.h>

#define MODNAME "arduino_cts"

#define ABS_X_MIN	0
#define ABS_X_MAX	1024
#define ABS_Y_MIN	0
#define ABS_Y_MAX	1024

#define MAX_CONTACTS 1    // 1 finger is it

#define VERSION "1.0" 	

#define SETXCOORD 0x2F
#define SETYCOORD 0x3F
#define TOUCHDOWN 0xFF
#define TOUCHUP 0x00
#define TOUCHSYNC 0x30

static struct input_dev *virt_ts_dev;
static void handle_command(unsigned char command, unsigned char value);

// reused code from virtual_touchscreen project. all credit should go to them, not to me. I just modified the code a little bit to make it usable via communicating with an Arduino that's connected to an capacitive 4wire touchscreen, using SPI.

int err;

static int __init ts_init(void)
{
	printk("%s - %s\n", MODNAME, __FUNCTION__);

	virt_ts_dev = input_allocate_device();
	if (!virt_ts_dev)
		return -ENOMEM;

	virt_ts_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
	virt_ts_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	input_set_abs_params(virt_ts_dev, ABS_X, ABS_X_MIN, ABS_X_MAX, 0, 0);
	input_set_abs_params(virt_ts_dev, ABS_Y, ABS_Y_MIN, ABS_Y_MAX, 0, 0);

	virt_ts_dev->name = "arduino_cts";
	virt_ts_dev->phys = "arduino_cts/input0";

    	input_mt_init_slots(virt_ts_dev, MAX_CONTACTS, INPUT_MT_DIRECT);

	input_set_abs_params(virt_ts_dev, ABS_MT_POSITION_X, ABS_X_MIN, ABS_X_MAX, 0, 0);
	input_set_abs_params(virt_ts_dev, ABS_MT_POSITION_Y, ABS_Y_MIN, ABS_Y_MAX, 0, 0);

	err = input_register_device(virt_ts_dev);
	if (err) {
		input_free_device(virt_ts_dev);
		return err;
	}
	printk("%s v%s %s - init complete\n", MODNAME, VERSION, __FUNCTION__);
	return 0;
}
	
static void handle_command(unsigned char command, unsigned char value) {
    printk("%s v%s %s - received command: 0x%02x 0x%02x\n", MODNAME, VERSION, __FUNCTION__, command, value);
    // x - sets X coord, y - the same as x but for y axis
    // d - touch DOWN, u - touch UP
    // S sync MANDATORY
    // basic commands
    switch(command) {
        case SETXCOORD:
            input_report_abs(virt_ts_dev, ABS_X, (int)value);
            break;
        case SETYCOORD:
            input_report_abs(virt_ts_dev, ABS_Y, (int)value);
            break;
        case TOUCHDOWN:
            input_report_key(virt_ts_dev, BTN_TOUCH, 1);
            break;
        case TOUCHUP:
            input_report_key(virt_ts_dev, BTN_TOUCH, 0);
            break;
        case TOUCHSYNC:
	    input_sync(virt_ts_dev);
            break;
        default:
	    printk("%s v%s %s - received unknown command: 0x%02x 0x%02x\n", MODNAME, VERSION, __FUNCTION__, command, value);
            break;
    }
}

static void __exit ts_exit(void)
{
	printk("%s v%s %s - exit! goodbye.\n", MODNAME, VERSION, __FUNCTION__);
	input_unregister_device(virt_ts_dev);
}

module_init(ts_init);
module_exit(ts_exit);

MODULE_AUTHOR("Talnaci Alexandru - flafyrex@gmail.com");
MODULE_DESCRIPTION("Arduino Capacitive TouchScreen");
MODULE_LICENSE("GPL");
