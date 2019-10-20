/*************************************************************************
	> File Name: kernel/descriptor_tables.c
	  > Author: Netcan
	  > Blog: http://www.netcan666.com
	  > Mail: 1469709759@qq.com
	  > Created Time: 2018-07-09 Mon 15:38:54 CST
 ************************************************************************/

#include <descriptor_tables.h>
#include <io.h>
#include <string.h>

extern void gdt_flush(u32 gdt_ptr);
static void init_gdt();
static void gdt_set_gate(s32, u32, u32, u8, u8 );

extern void idt_flush(u32 idt_ptr);
static void init_idt();
static void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);

/** GDT表，一共5个段。
 * 分别有如下段：
 * 1. NULL段
 * 2. 代码段
 * 3. 数据段
 * 4. 用户模式代码段
 * 5. 用户模式数据段
 */
gdt_entry_t gdt_entries[5];
gdt_ptr_t   gdt_ptr;

/** IDT表，一共256个中断。
 * 若未定义某个中断而触发了，将会导致*errupt Not Handled*异常。
 */
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

/** 初始化GDT表和IDT表。
 */
void init_descriptor_tables() {
	init_gdt();
	init_idt();
}

// 初始化GDT表
static void init_gdt() {
   gdt_ptr.limit = sizeof(gdt_entries) - 1;
   gdt_ptr.base  = (u32)&gdt_entries;

   // 0xc -> 1100, 0xa -> 1010, 0x9 -> 1001
   gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
   gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
   gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
   gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
   gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

   gdt_flush((u32)&gdt_ptr);
}

// 设置GDT表中的段
static void gdt_set_gate(s32 num, u32 base, u32 limit, u8 access, u8 gran) {
   gdt_entries[num].base_low    = (base & 0xFFFF);
   gdt_entries[num].base_middle = (base >> 16) & 0xFF;
   gdt_entries[num].base_high   = (base >> 24) & 0xFF;

   gdt_entries[num].limit_low   = (limit & 0xFFFF);
   gdt_entries[num].granularity = (limit >> 16) & 0x0F;

   gdt_entries[num].granularity |= gran & 0xF0;
   gdt_entries[num].access      = access;
}

// 初始化IDT表。
static void init_idt() {
	idt_ptr.limit = sizeof(idt_entries) - 1;
	idt_ptr.base = (u32)&idt_entries;

	// 初始化IDT表
	memset(&idt_entries, 0, sizeof(idt_entries));

	// system error中断(中断号<32)
	idt_set_gate(0, (u32)isr0, 0x08, 0x8E);
	idt_set_gate(1, (u32)isr1, 0x08, 0x8E);
	idt_set_gate(2, (u32)isr2, 0x08, 0x8E);
	idt_set_gate(3, (u32)isr3, 0x08, 0x8E);
	idt_set_gate(4, (u32)isr4, 0x08, 0x8E);
	idt_set_gate(5, (u32)isr5, 0x08, 0x8E);
	idt_set_gate(6, (u32)isr6, 0x08, 0x8E);
	idt_set_gate(7, (u32)isr7, 0x08, 0x8E);
	idt_set_gate(8, (u32)isr8, 0x08, 0x8E);
	idt_set_gate(9, (u32)isr9, 0x08, 0x8E);
	idt_set_gate(10, (u32)isr10, 0x08, 0x8E);
	idt_set_gate(11, (u32)isr11, 0x08, 0x8E);
	idt_set_gate(12, (u32)isr12, 0x08, 0x8E);
	idt_set_gate(13, (u32)isr13, 0x08, 0x8E);
	idt_set_gate(14, (u32)isr14, 0x08, 0x8E);
	idt_set_gate(15, (u32)isr15, 0x08, 0x8E);
	idt_set_gate(16, (u32)isr16, 0x08, 0x8E);
	idt_set_gate(17, (u32)isr17, 0x08, 0x8E);
	idt_set_gate(18, (u32)isr18, 0x08, 0x8E);
	idt_set_gate(19, (u32)isr19, 0x08, 0x8E);
	idt_set_gate(20, (u32)isr20, 0x08, 0x8E);
	idt_set_gate(21, (u32)isr21, 0x08, 0x8E);
	idt_set_gate(22, (u32)isr22, 0x08, 0x8E);
	idt_set_gate(23, (u32)isr23, 0x08, 0x8E);
	idt_set_gate(24, (u32)isr24, 0x08, 0x8E);
	idt_set_gate(25, (u32)isr25, 0x08, 0x8E);
	idt_set_gate(26, (u32)isr26, 0x08, 0x8E);
	idt_set_gate(27, (u32)isr27, 0x08, 0x8E);
	idt_set_gate(28, (u32)isr28, 0x08, 0x8E);
	idt_set_gate(29, (u32)isr29, 0x08, 0x8E);
	idt_set_gate(30, (u32)isr30, 0x08, 0x8E);
	idt_set_gate(31, (u32)isr31, 0x08, 0x8E);

	// re-map PIC，将master PIC映射到0x20-0x27, slave PIC映射到0x28-0x2f
	u8 pic_master_mask, pic_slave_mask;
	pic_master_mask = port_byte_in(PIC_MASTER_DATA); // 保存master/slave pic的掩码
	pic_slave_mask = port_byte_in(PIC_SLAVE_DATA);

	// 开始初始化（级联模式）
	port_byte_out(PIC_MASTER_CONTROL, 0x11);
	port_byte_out(PIC_SLAVE_CONTROL, 0x11);

	// 重置向量表地址
	port_byte_out(PIC_MASTER_DATA, 0x20);
	port_byte_out(PIC_SLAVE_DATA, 0x28);

	port_byte_out(PIC_MASTER_DATA, 0x04);	// 告诉master pic，slave pic的位置在IRQ2(0100b)
	port_byte_out(PIC_SLAVE_DATA, 0x02);	// 告诉slave pic, 处于级联模式

	port_byte_out(PIC_MASTER_DATA, 0x01);
	port_byte_out(PIC_SLAVE_DATA, 0x01);

	// 恢复掩码
	port_byte_out(PIC_MASTER_DATA, pic_master_mask);
	port_byte_out(PIC_SLAVE_DATA, pic_slave_mask);

	// IRQ中断(中断号>=32)

	idt_set_gate(32, (u32)irq0, 0x08, 0x8E);
	idt_set_gate(33, (u32)irq1, 0x08, 0x8E);
	idt_set_gate(34, (u32)irq2, 0x08, 0x8E);
	idt_set_gate(35, (u32)irq3, 0x08, 0x8E);
	idt_set_gate(36, (u32)irq4, 0x08, 0x8E);
	idt_set_gate(37, (u32)irq5, 0x08, 0x8E);
	idt_set_gate(38, (u32)irq6, 0x08, 0x8E);
	idt_set_gate(39, (u32)irq7, 0x08, 0x8E);
	idt_set_gate(40, (u32)irq8, 0x08, 0x8E);
	idt_set_gate(41, (u32)irq9, 0x08, 0x8E);
	idt_set_gate(42, (u32)irq10, 0x08, 0x8E);
	idt_set_gate(43, (u32)irq11, 0x08, 0x8E);
	idt_set_gate(44, (u32)irq12, 0x08, 0x8E);
	idt_set_gate(45, (u32)irq13, 0x08, 0x8E);
	idt_set_gate(46, (u32)irq14, 0x08, 0x8E);
	idt_set_gate(47, (u32)irq15, 0x08, 0x8E);

	idt_flush((u32)&idt_ptr);
}

static void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags) {
   idt_entries[num].base_lo = base & 0xFFFF;
   idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

   idt_entries[num].sel     = sel;
   idt_entries[num].always0 = 0;
   // 用户模式下，取消注释
   idt_entries[num].flags   = flags /* | 0x60 */;
}

