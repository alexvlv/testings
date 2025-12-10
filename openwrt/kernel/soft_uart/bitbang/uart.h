/**************************************************************************
$Id$
***************************************************************************/

#pragma once

struct bitbang_priv;

int bitbang_uart_init(struct bitbang_priv *);
void bitbang_uart_exit(struct bitbang_priv *);

void bitbang_uart_set_period(struct bitbang_priv *);

int bitbang_uart_open(struct bitbang_priv *);
void bitbang_uart_close(struct bitbang_priv *);

bool bitbang_uart_tx_buffer_full(struct bitbang_priv *);
bool bitbang_uart_rx_buffer_empty(struct bitbang_priv *);

ssize_t bitbang_tx(struct bitbang_priv *priv, const void __user *from, size_t count);
ssize_t bitbang_rx(struct bitbang_priv *priv, void __user *to, size_t count);
