#define TX_BUF_SIZE 512

struct tx_ring {
	u8 buf[TX_BUF_SIZE];
	unsigned int head;
	unsigned int tail;
	spinlock_t lock;
};

/* initialize the ring buffer */
static void tx_ring_init(struct tx_ring *r)
{
	r->head = 0;
	r->tail = 0;
	spin_lock_init(&r->lock);
}

/* push a byte into TX ring */
static bool tx_ring_put(struct tx_ring *r, u8 byte)
{
	unsigned long flags;
	spin_lock_irqsave(&r->lock, flags);

	unsigned int next = (r->head + 1) % TX_BUF_SIZE;
	if (next == r->tail) {
		spin_unlock_irqrestore(&r->lock, flags);
		return false; /* buffer full */
	}

	r->buf[r->head] = byte;
	r->head = next;

	spin_unlock_irqrestore(&r->lock, flags);
	return true;
}

/* pop a byte from TX ring */
static bool tx_ring_get(struct tx_ring *r, u8 *byte)
{
	unsigned long flags;
	spin_lock_irqsave(&r->lock, flags);

	if (r->head == r->tail) {
		spin_unlock_irqrestore(&r->lock, flags);
		return false; /* buffer empty */
	}

	*byte = r->buf[r->tail];
	r->tail = (r->tail + 1) % TX_BUF_SIZE;

	spin_unlock_irqrestore(&r->lock, flags);
	return true;
}

/* check how many bytes are in the buffer */
static unsigned int tx_ring_count(struct tx_ring *r)
{
	unsigned long flags;
	unsigned int count;
	spin_lock_irqsave(&r->lock, flags);
	count = (r->head + TX_BUF_SIZE - r->tail) % TX_BUF_SIZE;
	spin_unlock_irqrestore(&r->lock, flags);
	return count;
}
