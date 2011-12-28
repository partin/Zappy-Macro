
#define MY_WORK_QUEUE_NAME "ZappyWorkQueue"

static struct workqueue_struct *my_workqueue;
static struct work_struct TaskClr, TaskSet;

void set_led_internal(struct work_struct *work) {
    struct tty_struct *tty = vc_cons[fg_console].d->port.tty;
    struct tty_driver *driver = tty->driver;
    unsigned long ledstatus;

    ledstatus = (driver->ops->ioctl)(tty, KDGETLED, 0) & 0x7;

    //printk(KERN_INFO "zappy: leds = %08x\n", oldled);

    if (work == &TaskSet)
        ledstatus |= LED_SCR;
    else
        ledstatus &= ~LED_SCR;

    //printk(KERN_INFO "zappy: set leds = %08x\n", ledstatus);
    (driver->ops->ioctl) (tty, KDSETLED, ledstatus);
}

static DECLARE_WORK(TaskClr, set_led_internal);
static DECLARE_WORK(TaskSet, set_led_internal);

void ui_init(void) {
    my_workqueue = create_workqueue(MY_WORK_QUEUE_NAME);
}

void ui_clear(void) {
    flush_workqueue(my_workqueue);
}

void ui_set_state(int status) {
    printk(KERN_INFO "zappy: set leds = %d\n", status);
    if (status == 0)
        queue_work(my_workqueue, &TaskClr);
    else
        queue_work(my_workqueue, &TaskSet);
}
