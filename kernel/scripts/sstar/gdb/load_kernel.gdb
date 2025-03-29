restore ./arch/arm/boot/Image binary 0x20008000
restore ../project/image/output/images/rootfs.sqfs binary 0x27000000
#restore ../project/image/output/images/miservice.sqfs binary 0x27400000
#restore ../project/image/output/images/customer.jffs2 binary 0x27800000
set $pc=0x20008000
set $sp=0x21000000
