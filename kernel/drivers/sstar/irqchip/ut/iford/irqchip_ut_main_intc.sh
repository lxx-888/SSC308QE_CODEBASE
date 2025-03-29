#!/bin/sh

# ref irqs.h, let index seq from GIC_SPI_MS_IRQ_START to (GIC_SPI_MS_FIQ_END - 1)
echo "***************** main intc test ******************"
echo "=================== unmask test ==================="
for index in $(busybox seq 32 95);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,main-intc para_case=0;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done

for index in $(busybox seq 160 191);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,main-intc para_case=0;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done

echo "==================== mask test ===================="
for index in $(busybox seq 32 95);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,main-intc para_case=1;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done

for index in $(busybox seq 160 191);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,main-intc para_case=1;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done

echo "==================== no request irq force irq test ===================="
for index in $(busybox seq 32 95);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,main-intc para_case=3;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done

for index in $(busybox seq 160 191);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,main-intc para_case=3;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done
