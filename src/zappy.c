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
//#include <linux/poll.h>         /* miscdevice stuff */
//#include <linux/miscdevice.h>
//#include <linux/proc_fs.h>
//#include <asm/uaccess.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/kbd_kern.h>
#include <linux/kd.h>

MODULE_DESCRIPTION ( "Zappy Keyboard Macro" );
MODULE_AUTHOR ( "Martin Tillenius" );
MODULE_LICENSE ( "GPL" );

int kstroke_handler(struct notifier_block *, unsigned long, void *);

// notifier block for kbd_listener
static struct notifier_block nb = {
    .notifier_call = kstroke_handler
};

//
////static char keycodes[512];
////void init_keycodes ( void );
//
//static unsigned int buf_keycode[BUF_SIZE];
//static int buf_down[BUF_SIZE];
//

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

////struct file_operations swkeybd_file_operations;
//
////struct miscdevice swkeybd_misc = {
//// minor:MISC_DYNAMIC_MINOR,
//// name:"zappykbd",
//// fops:&swkeybd_file_operations,
////};
//
//
//
//
//
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

    int i;
    struct keyboard_notifier_param *kbd_np = param;

    // ignore everything but KBD_KEYCODE
    if (mode != KBD_KEYCODE)
        return NOTIFY_DONE;

    if (record_state == 0) {

        // not recording

        if (kbd_np->value == PLAYBACK_KEYCODE) {
            printk(KERN_INFO "zappy: Playback\n");
            // playback
            for (i = 0; i < bufptr; i++) {
                printk(KERN_INFO "zappy: Keycode %d %d\n",
                       keybuffer[i].keycode, keybuffer[i].down);
            }
            printk(KERN_INFO "zappy: Playback finished\n");
            return NOTIFY_STOP; // consume keycode
        }

        if (kbd_np->value == RECORD_KEYCODE && kbd_np->down == 0) {
            printk(KERN_INFO "zappy: Start Record\n");
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
        printk(KERN_INFO "zappy: Stop record\n");
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

//
//
//
//
//
/////* Name:        swkeybd_read_procmem
//// * Description: invoked when reading from /proc/swkeybd
//// */
////int swkeybd_read_procmem ( char *buf, char **start, off_t offset,
////                           int count, int *eof, void *data ) {
////  static const char *internal_buf = "macrokbd";
////  const int len = 9;
////
////  printk ( KERN_INFO "macrokbd: Read\n" );
////  if (len <= count) {
////    strcpy(buf, internal_buf);
////    return len;
////  }
////  return 0;
////}
//

///* Name:        init_module
// * Description: invoked when module is loaded
// */
//int init_module ( void )
//{
//    int retval = -1;
//
//    printk ( KERN_INFO "zappy: Initializing...\n" );
//
//    // listen to keyboard events
//    register_keyboard_notifier(&nb);
//
//    /* initialize local structure */
//    memset(&swkeybd, 0, sizeof(struct swkeybd_device));
//    swkeybd.idev = input_allocate_device();
//    printk ( KERN_INFO "zappy: Initializing... %p\n", swkeybd.idev );
//
//    if (swkeybd.idev) {
//        printk ( KERN_INFO "zappy: Good... %p\n", swkeybd.idev );
//        //      retval = misc_register(&swkeybd_misc);
//        //      if (retval != 0) {
//        //    /* return if failure ... */
//        //    printk(KERN_INFO
//        //       "zappy: failed to register the macrokbd as a misc device\n" );
//        //
//        //    input_free_device(swkeybd.idev);
//        //    return retval;
//        //      }
//
//        /* set the name */
//        swkeybd.idev->name = "zappykbd";
//        swkeybd.idev->id.vendor = 0x00;
//        swkeybd.idev->id.product = 0x00;
//        swkeybd.idev->id.version = 0x00;
//
//        /* tell the features of this input device: fake only keys */
//        swkeybd.idev->evbit[0] = BIT ( EV_KEY );
//        //      set_bit ( KEY_A, swkeybd.idev->keybit );
//
//        memset(swkeybd.idev->keybit, 0xff, sizeof(long)*BITS_TO_LONGS(KEY_CNT));
//        //  unsigned long keybit[BITS_TO_LONGS(KEY_CNT)];
//
//        if (input_register_device(swkeybd.idev)) {
//            printk ( KERN_INFO
//                    "macrokbd: Unable to register input device!\n" );
//            input_free_device(swkeybd.idev);
//            return -1;
//        }
//
//        //      /* announce yourself */
//        //      create_proc_read_entry ( "macrokbd", /* filename*/
//        //                               0 /* default mode */ ,
//        //                 NULL /* parent dir */ ,
//        //                 swkeybd_read_procmem, NULL /* client data */  );
//    }
//
//    return retval;
//}

///* Name:        cleanup_module
// * Description: invoked when module is loaded
// */
//void cleanup_module ( void )
//{
//    printk ( KERN_INFO "macrokbd: Cleaning up.\n" );
//
//    //unregister kbd_listener
//    unregister_keyboard_notifier(&nb);
//
//    input_unregister_device ( swkeybd.idev );
//    input_free_device ( swkeybd.idev );
//    printk ( KERN_INFO "macrokbd: closing misc device\n" );
//    //  misc_deregister ( &swkeybd_misc );
//    //  remove_proc_entry ( "macrokbd", NULL /* parent dir */  );
//    printk ( KERN_INFO "macrokbd: module unregistered\n" );
//}

//int swkeybd_open ( struct inode *inode, struct file *filp ) { return 0; }
//int swkeybd_release ( struct inode *inode, struct file *filp ) { return 0; }
//unsigned int swkeybd_poll(struct file *filp, struct poll_table_struct *table) {
// return POLLWRNORM | POLLOUT;
//}

///* Name:        swkeybd_write
// * Description: write accepts data and converts it to mouse movement
// */
//ssize_t swkeybd_write ( struct file * filp, const char *buf, size_t count,
//                        loff_t * offp ) {
//  static char localbuf[BUF_SIZE];
//  int i;
//
//  printk ( KERN_INFO "macrokbd: Write1" );
//  if (count & 1) {
//    printk ( KERN_INFO "macrokbd: odd input!" );
//    --count;
//  }
//  printk ( KERN_INFO "macrokbd: Write2" );
//  if (count > BUF_SIZE) {
//    printk ( KERN_INFO "macrokbd: trunkating input!" );
//    count = BUF_SIZE;
//  }
//  printk ( KERN_INFO "macrokbd: Write3" );
//  if (copy_from_user(localbuf, buf, count) != 0) {
//    printk ( KERN_INFO "macrokbd: copy_from_user() failed!\n" );
//    /* silently ignore */
//    return count;
//  }
//  printk ( KERN_INFO "macrokbd: Write4" );
//  printk ( KERN_INFO "macrokbd: generating A" );
//  input_report_key(swkeybd.idev, KEY_A, 1);
//  input_report_key(swkeybd.idev, KEY_A, 0);
//  set_bit ( out, swkeybd.idev->keybit );

//  for (i = 0; i < count; i += 2) {
//    input_report_key(swkeybd.idev, buf[i], buf[i+1]);
//    printk ( KERN_INFO "macrokbd: char %d %d", KEY_A, buf[i+1] );
//  }
//  input_sync(swkeybd.idev);
//  return count;
//}

///* file operation-handlers for /dev/swkeybd */
//struct file_operations swkeybd_file_operations = {
// write:swkeybd_write,
// poll:swkeybd_poll,
// open:swkeybd_open,
// release:swkeybd_release,
//};

static int __init checkinit(void) {
    int retval = -1;
    record_state = 0;
    bufptr = 0;

    printk ( KERN_INFO "zappy: Initializing...\n" );

    keybuffer = kmalloc(BUF_SIZE, GFP_KERNEL);

    // listen to keyboard events
    retval = register_keyboard_notifier(&nb);

    //    return register_keyboard_notifier(&nb);

    /* initialize local structure */
    memset(&swkeybd, 0, sizeof(struct swkeybd_device));
    swkeybd.idev = input_allocate_device();
    printk ( KERN_INFO "zappy: Initializing... %p\n", swkeybd.idev );

    if (swkeybd.idev) {
        printk ( KERN_INFO "zappy: Good... %p\n", swkeybd.idev );
        //      retval = misc_register(&swkeybd_misc);
        //      if (retval != 0) {
        //    /* return if failure ... */
        //    printk(KERN_INFO
        //       "zappy: failed to register the macrokbd as a misc device\n" );
        //
        //    input_free_device(swkeybd.idev);
        //    return retval;
        //      }

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
                    "macrokbd: Unable to register input device!\n" );
            input_free_device(swkeybd.idev);
            kfree(keybuffer);
            return -1;
        }

        //      /* announce yourself */
        //      create_proc_read_entry ( "macrokbd", /* filename*/
        //                               0 /* default mode */ ,
        //                 NULL /* parent dir */ ,
        //                 swkeybd_read_procmem, NULL /* client data */  );
    }

    return retval;
//    return register_keyboard_notifier(&nb);
}

static void __exit checkexit(void) {
    printk ( KERN_INFO "macrokbd: Cleaning up.\n" );

    unregister_keyboard_notifier(&nb);

    input_unregister_device ( swkeybd.idev );
    input_free_device ( swkeybd.idev );

    kfree(keybuffer);
    printk ( KERN_INFO "macrokbd: module unregistered\n" );
}

module_init(checkinit);
module_exit(checkexit);
