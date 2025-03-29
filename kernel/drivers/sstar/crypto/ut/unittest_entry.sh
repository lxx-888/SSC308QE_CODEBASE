# 1. Enable driver
# CONFIG_MS_CRYPTO=y
# CONFIG_SS_AESDMA_INTR ???


# 2. Use test-self procedure test Linux native crypto API
# Remove definition of CONFIG_CRYPTO_MANAGER_DISABLE_TESTS to enable it
# cat /proc/crypto
# check "selftest     : passed"

# 3. Test API for MI impl
rm unittest.report
rmmod ut_aes
insmod ut_aes.ko
echo all=1 > /sys/class/mstar/aes_auto/test_all
dmesg| grep result  | busybox tail -8 >>  unittest.report
exit



