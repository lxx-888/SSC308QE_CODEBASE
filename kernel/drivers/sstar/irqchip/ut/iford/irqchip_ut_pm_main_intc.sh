#!/bin/sh

echo "***************** pm main intc test ******************"
echo "=================== unmask test ==================="
for index in $(busybox seq 0 111);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,pm-main-intc para_case=0;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done

echo "==================== mask test ===================="
for index in $(busybox seq 0 111);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,pm-main-intc para_case=1;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done

echo "==================== no request irq force irq test ===================="
for index in $(busybox seq 0 111);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,pm-main-intc para_case=3;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done
