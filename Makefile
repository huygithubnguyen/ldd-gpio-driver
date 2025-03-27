obj-m := gpio_sysfs.o

BUILD_DIR ?= /home/huynguyen/Desktop/Distribution-Package/build-openstlinuxweston-stm32mp15-disco/tmp-glibc/deploy/images/stm32mp15-disco
TARGET_IP_ADDRESS ?= 172.16.10.160
#TARGET_IP_ADDRESS ?= 192.168.1.5
TARGET_USER ?= "root"
TARGET_PASSWORD ?= "root"

KERNEL_DIR ?= "${STAGING_KERNEL_DIR}"
KERNEL_SRC ?= $(KERNEL_DIR)

# Path to the directory that contains the source file(s) to compile
PWD := $(shell pwd) 

default:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

host:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

host-clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

copy-dtb:
	scp ${BUILD_DIR}/kernel/stm32mp157f-dk2.dtb root@${TARGET_IP_ADDRESS}:/boot
	