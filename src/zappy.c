/*****
 *       zappy
 *
 *  zappy.c, keyboard macros
 *     heavily based on
 *         swkeybd.c
 *     which is heavily based on idiom.c
 *     which is heavily based on usbmouse.c
 *
 *  Copyright (c) 2011 Martin Tillenius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA  02111-1307, USA.
 ****/

#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#define BUF_SIZE 1024
#define RECORD_KEYCODE 70
#define PLAYBACK_KEYCODE 41

#include <linux/module.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>         /* kmalloc() */
#include <linux/input.h>        /* usb stuff */
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/kbd_kern.h>
#include <linux/kd.h>

#include <linux/console_struct.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

MODULE_DESCRIPTION ( "Zappy Keyboard Macro" );
MODULE_AUTHOR ( "Martin Tillenius" );
MODULE_LICENSE ( "GPL" );

int kstroke_handler(struct notifier_block *, unsigned long, void *);

// notifier block for kbd_listener
static struct notifier_block nb = {
    .notifier_call = kstroke_handler
};

struct keypress {
    unsigned int keycode;
    int down;
};

struct keypress *keybuffer;
static int record_state = 0;
static int bufptr = 0;

struct swkeybd_device {
    struct input_dev *idev; /* input device, to push out input data */
} swkeybd;


struct tasklet_struct task;

void task_fn(unsigned long arg) {
    int i;
    //printk(KERN_INFO "zappy: Playback\n");
    for (i = 0; i < bufptr; i++) {
        //printk(KERN_INFO "zappy: Keycode %d %d\n",
        //        keybuffer[i].keycode, keybuffer[i].down);
        input_report_key ( swkeybd.idev, keybuffer[i].keycode, keybuffer[i].down);
        //printk(KERN_INFO "zappy: reported\n");
        //printk(KERN_INFO "zappy: synced\n");
    }
    input_sync(swkeybd.idev);
    //printk(KERN_INFO "zappy: Playback finished\n");
}



/*Called when a key is pressed.
 *Translates the keystroke into a string and stores them for writing to our log
 *file in the module's exit function.
 *
 *Parameters:
 * nb - notifier block we registered with
 * mode - keyboard mode
 * param - keyboard_notifier_param with keystroke data
 *
 *Returns:
 * zero (call me lazy)
 */
int kstroke_handler(struct notifier_block *nb,
        unsigned long mode, void *param) {

    struct keyboard_notifier_param *kbd_np = param;

    // ignore everything but KBD_KEYCODE
    if (mode != KBD_KEYCODE)
        return NOTIFY_DONE;

    if (record_state == 0) {

        // not recording

        if (kbd_np->value == PLAYBACK_KEYCODE) {

            // consume when key when its released
            if (kbd_np->down == 0)
                return NOTIFY_STOP;

            tasklet_init(&task, task_fn, 0);
            tasklet_schedule(&task);

            return NOTIFY_STOP; // consume keycode
        }

        if (kbd_np->value == RECORD_KEYCODE && kbd_np->down == 0) {
            //printk(KERN_INFO "zappy: Start Record\n");
            bufptr = 0;
            record_state = 1;
            return NOTIFY_DONE;
        }

        return NOTIFY_DONE;
    }

    // recording

    if (kbd_np->value == RECORD_KEYCODE) {
        // consume when key is pushed down (or repeated), and act on when its released
        if (kbd_np->down > 0)
            return NOTIFY_STOP;
        //printk(KERN_INFO "zappy: Stop record\n");
        // stop recording
        record_state = 0;
        return NOTIFY_DONE;
    }

    // if buffer is full, abort and stop recording
    if (bufptr == BUF_SIZE) {
        printk(KERN_INFO "zappy: Buffer full\n");
        record_state = 0;
        return NOTIFY_DONE;
    }

    keybuffer[bufptr].keycode = kbd_np->value;
    keybuffer[bufptr].down = kbd_np->down;
    ++bufptr;
//    kbd_np->value, kbd_np->down, kbd_np->shift, kbd_np->ledstate);

    return NOTIFY_DONE;
}

static int __init checkinit(void) {
    int retval = -1;
    record_state = 0;
    bufptr = 0;

    printk ( KERN_INFO "zappy: Initializing...\n" );

    keybuffer = kmalloc(BUF_SIZE, GFP_KERNEL);

    // listen to keyboard events
    retval = register_keyboard_notifier(&nb);

    /* initialize local structure */
    memset(&swkeybd, 0, sizeof(struct swkeybd_device));
    swkeybd.idev = input_allocate_device();

    if (swkeybd.idev) {
        /* set the name */
        swkeybd.idev->name = "zappykbd";
        swkeybd.idev->id.vendor = 0x00;
        swkeybd.idev->id.product = 0x00;
        swkeybd.idev->id.version = 0x00;

        /* tell the features of this input device: fake only keys */
        swkeybd.idev->evbit[0] = BIT ( EV_KEY );

        memset(swkeybd.idev->keybit, 0xff, sizeof(long)*BITS_TO_LONGS(KEY_CNT));

        if (input_register_device(swkeybd.idev)) {
            printk ( KERN_INFO
                    "zappy: Unable to register input device!\n" );
            input_free_device(swkeybd.idev);
            kfree(keybuffer);
            return -1;
        }
    }

    return retval;
}

static void __exit checkexit(void) {
    //printk ( KERN_INFO "zappy: Cleaning up.\n" );

    unregister_keyboard_notifier(&nb);

    input_unregister_device ( swkeybd.idev );
    input_free_device ( swkeybd.idev );

    kfree(keybuffer);
    printk ( KERN_INFO "zappy: module unregistered\n" );
}

module_init(checkinit);
module_exit(checkexit);
