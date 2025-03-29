// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2008 Atmel Corporation
 */
#include <common.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <flash.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>
#include <uuid.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <u-boot/crc.h>

#ifdef CONFIG_SSTAR_NOR
#include <../drivers/sstar/flash/drv_part.h>
loff_t ss_sf_env_offset = 0;
#if defined(CONFIG_ENV_OFFSET_REDUND)
loff_t ss_sf_env_redund_offset = 0;
#endif
#endif

#ifndef CONFIG_SPL_BUILD
#define INITENV
#endif

#define	OFFSET_INVALID		(~(u32)0)

#ifdef CONFIG_ENV_OFFSET_REDUND
#define ENV_OFFSET_REDUND	CONFIG_ENV_OFFSET_REDUND

#ifdef CONFIG_SSTAR_NOR
static ulong env_offset		= 0;
static ulong env_new_offset	= 0;
#else
static ulong env_offset		= CONFIG_ENV_OFFSET;
static ulong env_new_offset	= CONFIG_ENV_OFFSET_REDUND;
#endif

#else

#define ENV_OFFSET_REDUND	OFFSET_INVALID

#endif /* CONFIG_ENV_OFFSET_REDUND */

DECLARE_GLOBAL_DATA_PTR;

static int setup_flash_device(struct spi_flash **env_flash)
{
#if CONFIG_IS_ENABLED(DM_SPI_FLASH)
	struct udevice *new;
	int	ret;

	/* speed and mode will be read from DT */
	ret = spi_flash_probe_bus_cs(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				     CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE,
				     &new);
	if (ret) {
		env_set_default("spi_flash_probe_bus_cs() failed", 0);
		return ret;
	}

	*env_flash = dev_get_uclass_priv(new);
#else
	*env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				     CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!*env_flash) {
		env_set_default("spi_flash_probe() failed", 0);
		return -EIO;
	}
#endif
	return 0;
}

#if defined(CONFIG_ENV_OFFSET_REDUND)
static int env_sf_save(void)
{
	env_t	env_new;
	char	*saved_buffer = NULL, flag = ENV_REDUND_OBSOLETE;
	u32	saved_size = 0, saved_offset = 0, sector;
	u32	sect_size = CONFIG_ENV_SECT_SIZE;
	int	ret;
	struct spi_flash *env_flash;
#ifdef CONFIG_SSTAR_NOR
	struct sstar_part sstar_part;
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
#endif

	ret = setup_flash_device(&env_flash);
	if (ret)
		return ret;

#ifdef CONFIG_SSTAR_NOR
	if (!mtdparts_init() && !find_dev_and_part(ENV_PART_NAME, &dev, &pnum, &part)) {
		CONFIG_ENV_OFFSET = (loff_t)(part->offset);
	} else if (sstar_part_get_active((u8 *)ENV_PART_NAME, &sstar_part)) {
		CONFIG_ENV_OFFSET = (loff_t)(sstar_part.offset);
	}

	if (!mtdparts_init() && !find_dev_and_part(ENV_REDUND_PART_NAME, &dev, &pnum, &part)) {
		CONFIG_ENV_OFFSET_REDUND = (loff_t)(part->offset);
	} else if (sstar_part_get_active((u8 *)ENV_REDUND_PART_NAME, &sstar_part)) {
		CONFIG_ENV_OFFSET_REDUND = (loff_t)(sstar_part.offset);
	}
#endif

	if (IS_ENABLED(CONFIG_ENV_SECT_SIZE_AUTO))
		sect_size = env_flash->mtd.erasesize;

	ret = env_export(&env_new);
	if (ret)
		return -EIO;
	env_new.flags	= ENV_REDUND_ACTIVE;

	if (gd->env_valid == ENV_VALID) {
		env_new_offset = CONFIG_ENV_OFFSET_REDUND;
		env_offset = CONFIG_ENV_OFFSET;
	} else {
		env_new_offset = CONFIG_ENV_OFFSET;
		env_offset = CONFIG_ENV_OFFSET_REDUND;
	}

	/* Is the sector larger than the env (i.e. embedded) */
	if (sect_size > CONFIG_ENV_SIZE) {
		saved_size = sect_size - CONFIG_ENV_SIZE;
		saved_offset = env_new_offset + CONFIG_ENV_SIZE;
		saved_buffer = memalign(ARCH_DMA_MINALIGN, saved_size);
		if (!saved_buffer) {
			ret = -ENOMEM;
			goto done;
		}
		ret = spi_flash_read(env_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	sector = DIV_ROUND_UP(CONFIG_ENV_SIZE, sect_size);

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(env_flash, env_new_offset,
				sector * sect_size);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");

	ret = spi_flash_write(env_flash, env_new_offset,
		CONFIG_ENV_SIZE, &env_new);
	if (ret)
		goto done;

	if (sect_size > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = spi_flash_write(env_flash, env_offset + offsetof(env_t, flags),
				sizeof(env_new.flags), &flag);
	if (ret)
		goto done;

	puts("done\n");

	gd->env_valid = gd->env_valid == ENV_REDUND ? ENV_VALID : ENV_REDUND;

	printf("Valid environment: %d\n", (int)gd->env_valid);

done:
	spi_flash_free(env_flash);

	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

static int env_sf_load(void)
{
	int ret;
	int read1_fail, read2_fail;
	env_t *tmp_env1, *tmp_env2;
	struct spi_flash *env_flash;

	tmp_env1 = (env_t *)memalign(ARCH_DMA_MINALIGN,
			CONFIG_ENV_SIZE);
	tmp_env2 = (env_t *)memalign(ARCH_DMA_MINALIGN,
			CONFIG_ENV_SIZE);
	if (!tmp_env1 || !tmp_env2) {
		env_set_default("malloc() failed", 0);
		ret = -EIO;
		goto out;
	}

	ret = setup_flash_device(&env_flash);
	if (ret)
		goto out;

#ifdef CONFIG_SSTAR_NOR
	struct sstar_part  sstar_part;

	if (!sstar_part_get_active((u8 *)ENV_PART_NAME, &sstar_part)) {
		env_set_default("MDRV_PARTS_get_active_part failed", 0);
		return 0;
	}

	printf("\n%s: offset = 0x%x size = 0x%x\r\n", ENV_PART_NAME, sstar_part.offset, sstar_part.size);
	CONFIG_ENV_OFFSET = (loff_t)(sstar_part.offset);

	if (!sstar_part_get_active((u8 *)ENV_REDUND_PART_NAME, &sstar_part)) {
		env_set_default("MDRV_PARTS_get_active_part failed", 0);
		return 0;
	}

	printf("\n%s: offset = 0x%x size = 0x%x\r\n", ENV_REDUND_PART_NAME, sstar_part.offset, sstar_part.size);
	CONFIG_ENV_OFFSET_REDUND = (loff_t)(sstar_part.offset);
#endif

	read1_fail = spi_flash_read(env_flash, CONFIG_ENV_OFFSET,
				    CONFIG_ENV_SIZE, tmp_env1);
	read2_fail = spi_flash_read(env_flash, CONFIG_ENV_OFFSET_REDUND,
				    CONFIG_ENV_SIZE, tmp_env2);

	ret = env_import_redund((char *)tmp_env1, read1_fail, (char *)tmp_env2,
				read2_fail, H_EXTERNAL);

	spi_flash_free(env_flash);
out:
	free(tmp_env1);
	free(tmp_env2);

	return ret;
}
#else
static int env_sf_save(void)
{
	u32	saved_size = 0, saved_offset = 0, sector;
	u32	sect_size = CONFIG_ENV_SECT_SIZE;
	char	*saved_buffer = NULL;
	int	ret = 1;
	env_t	env_new;
	struct spi_flash *env_flash;
#ifdef CONFIG_SSTAR_NOR
	struct sstar_part     sstar_part;
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
#endif

	ret = setup_flash_device(&env_flash);
	if (ret)
		return ret;

#ifdef CONFIG_SSTAR_NOR
	if (!mtdparts_init() && !find_dev_and_part(ENV_PART_NAME, &dev, &pnum, &part)) {
		CONFIG_ENV_OFFSET = (loff_t)(part->offset);
	} else if (sstar_part_get_active((u8 *)ENV_PART_NAME, &sstar_part)) {
		CONFIG_ENV_OFFSET = (loff_t)(sstar_part.offset);
	}
#endif

	if (IS_ENABLED(CONFIG_ENV_SECT_SIZE_AUTO))
		sect_size = env_flash->mtd.erasesize;

	/* Is the sector larger than the env (i.e. embedded) */
	if (sect_size > CONFIG_ENV_SIZE) {
		saved_size = sect_size - CONFIG_ENV_SIZE;
		saved_offset = CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer)
			goto done;

		ret = spi_flash_read(env_flash, saved_offset,
			saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = env_export(&env_new);
	if (ret)
		goto done;

	sector = DIV_ROUND_UP(CONFIG_ENV_SIZE, sect_size);

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(env_flash, CONFIG_ENV_OFFSET,
		sector * sect_size);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");
	ret = spi_flash_write(env_flash, CONFIG_ENV_OFFSET,
		CONFIG_ENV_SIZE, &env_new);
	if (ret)
		goto done;

	if (sect_size > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash, saved_offset,
			saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = 0;
	puts("done\n");

done:
	spi_flash_free(env_flash);

	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

static int env_sf_load(void)
{
	int ret;
	char *buf = NULL;
	struct spi_flash *env_flash;

	buf = (char *)memalign(ARCH_DMA_MINALIGN, CONFIG_ENV_SIZE);
	if (!buf) {
		env_set_default("malloc() failed", 0);
		return -EIO;
	}

	ret = setup_flash_device(&env_flash);
	if (ret)
		goto out;

#ifdef CONFIG_SSTAR_NOR
	struct sstar_part     sstar_part;

	if (!sstar_part_get_active((u8 *)ENV_PART_NAME, &sstar_part)) {
		env_set_default("MDRV_PARTS_get_active_part failed", 0);
		free(buf);
		return 0;
	}

	printf("\n%s: offset = 0x%x size = 0x%x\r\n", ENV_PART_NAME, sstar_part.offset, sstar_part.size);
	CONFIG_ENV_OFFSET = (loff_t)(sstar_part.offset);
#endif

	ret = spi_flash_read(env_flash,
		CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, buf);
	if (ret) {
		env_set_default("spi_flash_read() failed", 0);
		goto err_read;
	}

	ret = env_import(buf, 1, H_EXTERNAL);
	if (!ret)
		gd->env_valid = ENV_VALID;

err_read:
	spi_flash_free(env_flash);
out:
	free(buf);

	return ret;
}
#endif

static int env_sf_erase(void)
{
	int ret;
	env_t env;
	struct spi_flash *env_flash;

	ret = setup_flash_device(&env_flash);
	if (ret)
		return ret;

	memset(&env, 0, sizeof(env_t));
	ret = spi_flash_write(env_flash, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, &env);
	if (ret)
		goto done;

	if (ENV_OFFSET_REDUND != OFFSET_INVALID)
		ret = spi_flash_write(env_flash, ENV_OFFSET_REDUND, CONFIG_ENV_SIZE, &env);

done:
	spi_flash_free(env_flash);

	return ret;
}

#if CONFIG_ENV_ADDR != 0x0
__weak void *env_sf_get_env_addr(void)
{
	return (void *)CONFIG_ENV_ADDR;
}
#endif

#if defined(INITENV) && (CONFIG_ENV_ADDR != 0x0)
/*
 * check if Environment on CONFIG_ENV_ADDR is valid.
 */
static int env_sf_init_addr(void)
{
	env_t *env_ptr = (env_t *)env_sf_get_env_addr();

	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr	= (ulong)&(env_ptr->data);
		gd->env_valid	= 1;
	} else {
		gd->env_addr = (ulong)&default_environment[0];
		gd->env_valid = 1;
	}

	return 0;
}
#endif

#if defined(CONFIG_ENV_SPI_EARLY)
/*
 * early load environment from SPI flash (before relocation)
 * and check if it is valid.
 */
static int env_sf_init_early(void)
{
	int ret;
	int read1_fail;
	int read2_fail;
	int crc1_ok;
	env_t *tmp_env2 = NULL;
	env_t *tmp_env1;
	struct spi_flash *env_flash;

	/*
	 * if malloc is not ready yet, we cannot use
	 * this part yet.
	 */
	if (!gd->malloc_limit)
		return -ENOENT;

	tmp_env1 = (env_t *)memalign(ARCH_DMA_MINALIGN,
			CONFIG_ENV_SIZE);
	if (IS_ENABLED(CONFIG_SYS_REDUNDAND_ENVIRONMENT))
		tmp_env2 = (env_t *)memalign(ARCH_DMA_MINALIGN,
					     CONFIG_ENV_SIZE);

	if (!tmp_env1 || !tmp_env2)
		goto out;

	ret = setup_flash_device(&env_flash);
	if (ret)
		goto out;

	read1_fail = spi_flash_read(env_flash, CONFIG_ENV_OFFSET,
				    CONFIG_ENV_SIZE, tmp_env1);

	if (IS_ENABLED(CONFIG_SYS_REDUNDAND_ENVIRONMENT)) {
		read2_fail = spi_flash_read(env_flash,
					    CONFIG_ENV_OFFSET_REDUND,
					    CONFIG_ENV_SIZE, tmp_env2);
		ret = env_check_redund((char *)tmp_env1, read1_fail,
				       (char *)tmp_env2, read2_fail);

		if (ret < 0)
			goto err_read;

		if (gd->env_valid == ENV_VALID)
			gd->env_addr = (unsigned long)&tmp_env1->data;
		else
			gd->env_addr = (unsigned long)&tmp_env2->data;
	} else {
		if (read1_fail)
			goto err_read;

		crc1_ok = crc32(0, tmp_env1->data, ENV_SIZE) ==
				tmp_env1->crc;
		if (!crc1_ok)
			goto err_read;

		/* if valid -> this is our env */
		gd->env_valid = ENV_VALID;
		gd->env_addr = (unsigned long)&tmp_env1->data;
	}

	spi_flash_free(env_flash);

	return 0;
err_read:
	spi_flash_free(env_flash);

	free(tmp_env1);
	if (IS_ENABLED(CONFIG_SYS_REDUNDAND_ENVIRONMENT))
		free(tmp_env2);
out:
	/* env is not valid. always return 0 */
	gd->env_valid = ENV_INVALID;
	return 0;
}
#endif

static int env_sf_init(void)
{
#if defined(INITENV) && (CONFIG_ENV_ADDR != 0x0)
	return env_sf_init_addr();
#elif defined(CONFIG_ENV_SPI_EARLY)
	return env_sf_init_early();
#endif
	/*
	 * return here -ENOENT, so env_init()
	 * can set the init bit and later if no
	 * other Environment storage is defined
	 * can set the default environment
	 */
	return -ENOENT;
}

U_BOOT_ENV_LOCATION(sf) = {
	.location	= ENVL_SPI_FLASH,
	ENV_NAME("SPIFlash")
	.load		= env_sf_load,
	.save		= ENV_SAVE_PTR(env_sf_save),
	.erase		= ENV_ERASE_PTR(env_sf_erase),
	.init		= env_sf_init,
};
