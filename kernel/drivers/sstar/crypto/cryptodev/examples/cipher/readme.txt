(1) 文件说明
├── cipher_aes_async.c				------ 异步方式进行aes128(CBC, ECB, CTR)加密/解密结果验证
├── cipher_aes_speed_async.c		------ 异步方式测试aes128(CBC, ECB, CTR)的处理速度, 不对结果做校验
├── cipher_aes_speed_sync.c			------ 同步方式测试aes128(CBC, ECB, CTR)的处理速度, 不对结果做校验
├── cipher_aes_sync.c				------ 同步方式进行aes128(CBC, ECB, CTR)加密/解密结果验证
├── cipher_aes256_async.c				------ 异步方式进行aes256(CBC, ECB, CTR)加密/解密结果验证
├── cipher_aes256_speed_async.c		------ 异步方式测试aes256(CBC, ECB, CTR)的处理速度, 不对结果做校验
├── cipher_aes256_speed_sync.c			------ 同步方式测试aes256(CBC, ECB, CTR)的处理速度, 不对结果做校验
├── cipher_aes256_sync.c				------ 同步方式进行aes256(CBC, ECB, CTR)加密/解密结果验证
├── cipher_hash256_async.c			------ 暂未使用
├── cipher_hash256_speed_async.c	------ 暂未使用
├── cipher_hash256_speed_sync.c		------ 暂未使用
├── cipher_hash256_sync.c			------ 同步方式对hash256的结果做检验, 连续对比10次同一份数据产生的hash值
├── cipher_rsa_sync.c				------ rsa2048的测试demo
├── cipher_rsa4096_sync.c				------ rsa4096的测试demo
├── cipher_rsa_speed_sync.c				------ 同步方式测试rsa2048的处理速度, 不对结果做校验
├── cipher_rsa4096_speed_sync.c			------ 同步方式测试rsa4096的处理速度, 不对结果做校验
├── crypto
│   ├── cryptodev.h					------ kernel\drivers\sstar\crypto\cryptodev\crypto\cryptodev.h
│   └── mdrv_rsa.h					------ kernel\drivers\sstar\crypto\mdrv_rsa.h (有去掉一些包含, 防止编译不过)
├── fullspeed.c						------ 暂未使用
├── Makefile						------ Makefile
├── readme.txt						------ readme.txt
└── sha_speed.c						------ 暂未使用

(2) 编译说明
make clean;make     ####### 在install目录下会生成所有的bin文件