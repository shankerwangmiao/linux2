#include <linux/console.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/serial_reg.h>
#include <linux/serial.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <linux/memblock.h>

static phys_addr_t log_base;
static size_t log_size;
static size_t log_pos = 0;

static void loong_uart_putc(unsigned char c){
	*(volatile unsigned char *)TO_UNCACHE(log_base + log_pos % log_size) = c;
	log_pos += 1;
}

static void early_serial_write(struct console *con, const char *s, unsigned n){
	while(*s && n-- > 0){
		if ( *s == '\n' ){
			loong_uart_putc('\r');
		}
		loong_uart_putc(*s);
		s++;
	}
}

static struct console early_serial_console = {
	.name =		"earlyser",
	.write =	early_serial_write,
	.flags =	CON_PRINTBUFFER | CON_BOOT,
	.index =	-1,
};

static int __init setup_early_printk(char *buf)
{
	static int registered = 0;
	if (registered) {
		return 0;
	}
	if (early_serial_console.index != -1) {
		return 0;
	}

	char *arg = buf;
	unsigned long long mem_size = memparse(arg, &arg);

	if(arg == buf){
		return -EINVAL;
	}

	if (*arg != '@') {
		return -EINVAL;
	}

	if(mem_size == 0){
		return -EINVAL;
	}

	unsigned long long start_at = memparse(arg + 1, &arg);
	memblock_reserve(start_at, mem_size);

	log_size = mem_size;
	log_base = start_at;

	register_console(&early_serial_console);
	return 0;
}

early_param("earlyprintk", setup_early_printk);
