//****************************************************************************
////**
////**    Idt.c
////**		Interrupt Descriptor Table. The IDT is responsible for providing
////**	the interface for managing interrupts, installing, setting, requesting,
////**	generating, and interrupt callback managing.
////**
////****************************************************************************

#include "idt.h"
#include "../../include/string.h"
#include "../../kernel/display.h"

//! interrupt descriptor table
static struct idt_descriptor	_idt [I86_MAX_INTERRUPTS];

//! idtr structure used to help define the cpu's idtr register
static struct idtr				_idtr;

//! returns interrupt descriptor
struct idt_descriptor* i86_get_ir (uint32_t i) {

	if (i>I86_MAX_INTERRUPTS)
		return 0;

	return &_idt[i];
}

//! installs idtr into processors idtr register
static void idt_install () 
{
	asm ("lidt %0": :"m"(_idtr));
}

//! default handler to catch unhandled system interrupts.
static void i86_default_handler (void)
{

	ClrScr (0x18);
	GotoXY (0,0);
	SetColor (0x1e);
	Puts ("*** i86_default_handler: Unhandled Exception");
    	for(;;);
}

//! installs a new interrupt handler
int i86_install_ir (uint32_t i, uint8_t flags, uint16_t sel, I86_IRQ_HANDLER irq) 
{
    uint32_t    uiBase;

    if (i>I86_MAX_INTERRUPTS)
	return 0;

    if (!irq)
	return 0;

    //! get base address of interrupt handler
    uiBase = (uint32_t)&(*irq);

    //! store base address into idt
    _idt[i].baseLo		=	(uint16_t)(uiBase & 0xffff);
    _idt[i].baseHi		=	(uint16_t)((uiBase >> 16) & 0xffff);
    _idt[i].reserved		=	0;
    _idt[i].flags		=	flags;
    _idt[i].sel			=	sel;
    return	0;
}

//! initialize idt
int i86_idt_initialize (uint16_t codeSel) 
{
    int i;

    //! set up idtr for processor
    _idtr.limit = sizeof (struct idt_descriptor) * I86_MAX_INTERRUPTS -1;
    _idtr.base	= (uint32_t)&_idt[0];

    //! null out the idt
    memset ((void*)&_idt[0], 0, sizeof (struct idt_descriptor) * I86_MAX_INTERRUPTS-1);

    //! register default handlers
    for (i=0; i<I86_MAX_INTERRUPTS; i++)
    i86_install_ir (i, I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32,
    	codeSel, i86_default_handler);

    //! install our idt
    idt_install ();

    return 0;
}
