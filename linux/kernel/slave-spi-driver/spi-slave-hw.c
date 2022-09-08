/**************************************************************************
$Id$
***************************************************************************/

#define CHIP_SELECT 0

#include ".git.h"
#include "kdbg.h"
#include "spi-slave.h"
#include "spi-slave-hw.h"

#include "mx51_ecspi_regs.h"

#include <linux/types.h>
#include <asm/io.h>

//-------------------------------------------------------------------------
static void mx51_ecspi_init_ctrl(void __iomem *base)
{
	u32 ctrl = MX51_ECSPI_CTRL_ENABLE;

	/* set Slave mode */
	ctrl &= ~MX51_ECSPI_CTRL_MODE_MASK;

	/* Clear BL field and set the right value */
	ctrl &= ~MX51_ECSPI_CTRL_BL_MASK;
	ctrl |= (PACKET_SIZE * 8 - 1) << MX51_ECSPI_CTRL_BL_OFFSET;

	/* set clock speed */
	ctrl &= ~(0xf << MX51_ECSPI_CTRL_POSTDIV_OFFSET |
		  0xf << MX51_ECSPI_CTRL_PREDIV_OFFSET);

	ctrl &= ~MX51_ECSPI_CTRL_SMC;
	
	/* set chip select to use */
	ctrl |= MX51_ECSPI_CTRL_CS(CHIP_SELECT);

	/*
	 * The ctrl register must be written first, with the EN bit set other
	 * registers must not be written to.
	 */
	writel(ctrl, base + MX51_ECSPI_CTRL);
}
//-------------------------------------------------------------------------
static void mx51_ecspi_init_testreg(void __iomem *base)
{
	u32 testreg = readl(base + MX51_ECSPI_TESTREG);
	testreg &= ~MX51_ECSPI_TESTREG_LBC;
	writel(testreg, base + MX51_ECSPI_TESTREG);
}
//-------------------------------------------------------------------------
static void mx51_ecspi_init_config(void __iomem *base)
{
	u32 cfg = readl(base + MX51_ECSPI_CONFIG);
	/*
	 * eCSPI burst completion by Chip Select signal in Slave mode
	 * is not functional for imx53 Soc, config SPI burst completed when
	 * BURST_LENGTH + 1 bits are received
	 */
	cfg &= ~MX51_ECSPI_CONFIG_SBBCTRL(CHIP_SELECT);

	// SPI_CPHA
	cfg |= MX51_ECSPI_CONFIG_SCLKPHA(CHIP_SELECT);

	// SPI_CPOL
	cfg |= MX51_ECSPI_CONFIG_SCLKPOL(CHIP_SELECT);
	cfg |= MX51_ECSPI_CONFIG_SCLKCTL(CHIP_SELECT);

	 // not SPI_CS_HIGH
	cfg &= ~MX51_ECSPI_CONFIG_SSBPOL(CHIP_SELECT);
	writel(cfg, base + MX51_ECSPI_CONFIG);
}
//-------------------------------------------------------------------------
void mx51_ecspi_init(void __iomem *base)
{
	mx51_ecspi_init_ctrl(base);
	mx51_ecspi_init_testreg(base);
	mx51_ecspi_init_config(base);
}
//-------------------------------------------------------------------------
void mx51_ecspi_disable(void __iomem *base)
{
	u32 ctrl;

	ctrl = readl(base + MX51_ECSPI_CTRL);
	ctrl &= ~MX51_ECSPI_CTRL_ENABLE;
	writel(ctrl, base + MX51_ECSPI_CTRL);
}
//-------------------------------------------------------------------------
inline unsigned mx51_ecspi_rxcnt(void __iomem *base)
{
	return (readl(base + MX51_ECSPI_TESTREG) >> 8)&0x7F;
}
//-------------------------------------------------------------------------
inline unsigned mx51_ecspi_stat(void __iomem *base)
{
	return readl(base + MX51_ECSPI_STAT);
}
//-------------------------------------------------------------------------
void mx51_ecspi_stat_clr(void __iomem *base)
{
	writel(0xC0, base + MX51_ECSPI_STAT); // Clear bits TC and RO
}
//-------------------------------------------------------------------------
inline unsigned mx51_ecspi_data(void __iomem *base)
{
	return readl(base + MXC_CSPIRXDATA);
}
//-------------------------------------------------------------------------
inline unsigned mx51_ecspi_rx_available(void __iomem *base)
{
	return readl(base + MX51_ECSPI_STAT) & MX51_ECSPI_STAT_RR;
}
//-------------------------------------------------------------------------
void mx51_ecspi_reset(void __iomem *base)
{
	/* drain receive buffer */
	while (mx51_ecspi_rx_available(base))
		readl(base + MXC_CSPIRXDATA);
}
//-------------------------------------------------------------------------

