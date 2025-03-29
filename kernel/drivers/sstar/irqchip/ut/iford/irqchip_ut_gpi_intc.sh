#!/bin/sh

echo "***************** gpi intc test ******************"
echo "=================== unmask test ==================="
for index in $(busybox seq 0 131);
do
echo "*** ${index} ***";
insmod intc.ko para_hwirq=${index} para_intc=sstar,gpi-intc para_case=0;
if [ $? -eq 0 ]; then
sleep 0.5;
rmmod intc;
fi
done
