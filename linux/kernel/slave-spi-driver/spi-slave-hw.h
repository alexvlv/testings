/**************************************************************************
$Id$
***************************************************************************/

#pragma once

void mx51_ecspi_init(void __iomem *base);
void mx51_ecspi_disable(void __iomem *base);

unsigned mx51_ecspi_rxcnt(void __iomem *base);
unsigned mx51_ecspi_stat(void __iomem *base);
void mx51_ecspi_stat_clr(void __iomem *base);
unsigned mx51_ecspi_data(void __iomem *base);
unsigned mx51_ecspi_rx_available(void __iomem *base);
void mx51_ecspi_reset(void __iomem *base);

