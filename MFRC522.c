#include <linux/module.h>
#include <linux/spi/spi.h>

static int MFRC522_spi_init(void);
static int MFRC522_spi_write_then_read(struct spi_device *spi, const void *txbuf, unsigned n_tx, void *rxbuf, unsigned n_rx);

static struct spi_device *MFRC522_spi_device;