/**************************************************************************
$Id$
***************************************************************************/

#pragma once

#define MXC_CSPIRXDATA		0x00
#define MXC_CSPITXDATA		0x04
#define MXC_CSPICTRL		0x08
#define MXC_CSPIINT		0x0c
#define MXC_RESET		0x1c

#define MX51_ECSPI_CTRL		0x08
#define MX51_ECSPI_CTRL_ENABLE		(1 <<  0)
#define MX51_ECSPI_CTRL_XCH		(1 <<  2)
#define MX51_ECSPI_CTRL_SMC		(1 << 3)
#define MX51_ECSPI_CTRL_MODE_MASK	(0xf << 4)
#define MX51_ECSPI_CTRL_DRCTL(drctl)	((drctl) << 16)
#define MX51_ECSPI_CTRL_POSTDIV_OFFSET	8
#define MX51_ECSPI_CTRL_PREDIV_OFFSET	12
#define MX51_ECSPI_CTRL_CS(cs)		((cs) << 18)
#define MX51_ECSPI_CTRL_BL_OFFSET	20
#define MX51_ECSPI_CTRL_BL_MASK		(0xfff << 20)

#define MX51_ECSPI_CONFIG	0x0c
#define MX51_ECSPI_CONFIG_SCLKPHA(cs)	(1 << ((cs) +  0))
#define MX51_ECSPI_CONFIG_SCLKPOL(cs)	(1 << ((cs) +  4))
#define MX51_ECSPI_CONFIG_SBBCTRL(cs)	(1 << ((cs) +  8))
#define MX51_ECSPI_CONFIG_SSBPOL(cs)	(1 << ((cs) + 12))
#define MX51_ECSPI_CONFIG_SCLKCTL(cs)	(1 << ((cs) + 20))

#define MX51_ECSPI_INT		0x10
#define MX51_ECSPI_INT_TEEN		(1 <<  0)
#define MX51_ECSPI_INT_RREN		(1 <<  3)
#define MX51_ECSPI_INT_RDREN		(1 <<  4)

#define MX51_ECSPI_DMA		0x14
#define MX51_ECSPI_DMA_TX_WML(wml)	((wml) & 0x3f)
#define MX51_ECSPI_DMA_RX_WML(wml)	(((wml) & 0x3f) << 16)
#define MX51_ECSPI_DMA_RXT_WML(wml)	(((wml) & 0x3f) << 24)

#define MX51_ECSPI_DMA_TEDEN		(1 << 7)
#define MX51_ECSPI_DMA_RXDEN		(1 << 23)
#define MX51_ECSPI_DMA_RXTDEN		(1 << 31)

#define MX51_ECSPI_STAT		0x18
#define MX51_ECSPI_STAT_RR		(1 <<  3)

#define MX51_ECSPI_TESTREG	0x20
#define MX51_ECSPI_TESTREG_LBC	BIT(31)
