#FLAGS := -O3 -Wall 
#LIBS := \
#  -lgtk-3 \
#  -lX11 \
#  -lXtst
#INCLUDES := \
#  -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ \
#  -I/usr/include/atk-1.0/ \
#  -I/usr/include/gdk-pixbuf-2.0/ \
#  -I/usr/include/cairo/ \
#  -I/usr/include/pango-1.0/ \
#  -I/usr/include/glib-2.0/ \
#  -I/usr/include/gtk-3.0
#
#zappy:
#	$(CXX) $(FLAGS) $(INCLUDES) main.cpp -pthread -o zappy $(LIBS)
#
#clean:
#	rm -rf zappy

VERSION := 1.0.0
KDIR   := /lib/modules/$(shell uname -r)/build
PWD    := $(shell pwd)
KERNEL := $(shell uname -r | sed 's,-[0-9\-]*,,g')

default:
	cd src && $(MAKE) -C $(KDIR) SUBDIRS=$(PWD)/src modules 

clean:
	cd src && make clean

install:
	cd src && $(MAKE) -C $(KDIR) SUBDIRS=$(PWD)/src modules modules_install && depmod -a

try:
	-sudo rmmod zappy
	sudo insmod src/zappy.ko
	sleep 5
	-sudo rmmod zappy
#	sudo chmod a+rwx /dev/macrokbd

#	sudo insmod src/swmouse.ko && 
#	-sudo rmmod swmouse 
#	sudo chmod a+rwx /dev/swkeybd /dev/swmouse

#check:
#	cd test && ./all.sh

#dist:
#	make clean
#	mkdir -p swinput-$(VERSION)/
#	rm -fr swinput-$(VERSION)/*
#	mkdir -p swinput-$(VERSION)/src 
#	mkdir -p swinput-$(VERSION)/test
#	mkdir -p swinput-$(VERSION)/test/data/swkeybd
#	mkdir -p swinput-$(VERSION)/test/data/swmouse
#	mkdir -p swinput-$(VERSION)/test/bc
#	cp   ChangeLog COPYING README  Makefile swinput-$(VERSION)/
#	cp   test/*.sh swinput-$(VERSION)/test/
#	cp   test/bc/*.bc swinput-$(VERSION)/test/bc/
#	cp   test/data/swkeybd/*.txt swinput-$(VERSION)/test/data/swkeybd
#	-cp   test/data/swmouse/*.txt swinput-$(VERSION)/test/data/swmouse
#	cp   src/*.c src/*.h swinput-$(VERSION)/src/
#	cp   src/Makefile swinput-$(VERSION)/src/
#	rm -f swinput-$(VERSION).tar swinput-$(VERSION).tar.gz
#	tar cvf swinput-$(VERSION).tar   swinput-$(VERSION)/
#	gzip swinput-$(VERSION).tar 
#	gpg -b swinput-$(VERSION).tar.gz

