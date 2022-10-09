#include "../include/common.h"


cpu_context ctx = {0};

#define CPU_DEBUG 0
void save_wram();
void savevideo_buffer();
void read_wram();
void ui_update();
void update_dbg_window();
void readvideo_buffer();;
void load_line_sprites();
cpu_context *cpu_get_context()
{
    return &ctx;
}

void *mymemcpy(void *dst, const void *src, size_t num)
{
    int nchunks = num / sizeof(dst); /*按CPU位寬拷貝*/
    int slice = num % sizeof(dst);   /*剩餘的按字節拷貝*/
    unsigned long *s = (unsigned long *)src;
    unsigned long *d = (unsigned long *)dst;
    while (nchunks--)
        *d++ = *s++;
    while (slice--)
        *((char *)d++) = *((char *)s++);
    return dst;
}

void cpu_init()
{
    ctx.regs.pc = 0x100;
    ctx.regs.sp = 0xFFFE;
    *((short *)&ctx.regs.a) = 0xB001;
    *((short *)&ctx.regs.b) = 0x1300;
    *((short *)&ctx.regs.d) = 0xD800;
    *((short *)&ctx.regs.h) = 0x4D01;
    ctx.ie_register = 0;
    ctx.int_flags = 0;
    ctx.int_master_enabled = false;
    ctx.enabling_ime = false;

    timer_get_context()->div = 0xABCC;
}

static void fetch_instruction()
{
    ctx.cur_opcode = bus_read(ctx.regs.pc++);
    ctx.cur_inst = instruction_by_opcode(ctx.cur_opcode);
}

void fetch_data();

static void execute()
{
    IN_PROC proc = inst_get_processor(ctx.cur_inst->type);

    if (!proc)
    {
        NO_IMPL
    }

    proc(&ctx);
}
int count = 0;
u8 *wram;
u8 *hram;
ram_context *ram2;
u64 save_ticks;
bool cpu_step()
{

    if (!ctx.halted)
    {
        u16 pc;
        // char flags[16];
        // restart:
        pc = ctx.regs.pc;
        // sprintf(flags, "%c%c%c%c",
        //         ctx.regs.f & (1 << 7) ? 'Z' : '-',
        //         ctx.regs.f & (1 << 6) ? 'N' : '-',
        //         ctx.regs.f & (1 << 5) ? 'H' : '-',
        //         ctx.regs.f & (1 << 4) ? 'C' : '-');
        fetch_instruction();
        emu_cycles(1);
        //   if(pc==256)
        //        ctx.regs.a=0;
        // fprintf(fp, "(%02X %02X %02X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
        //        ctx.cur_opcode,
        //             bus_read(pc + 1), bus_read(pc + 2), ctx.regs.a, flags, ctx.regs.b, ctx.regs.c,
        //             ctx.regs.d, ctx.regs.e, ctx.regs.h, ctx.regs.l);
        fetch_data();

        // #if CPU_DEBUG == 1
        // #endif

        if (ctx.cur_inst == NULL)
        {
            printf("Unknown Instruction! %02X\n", ctx.cur_opcode);
            exit(-7);
        }
        count++;

        // dbg_update();
        // dbg_print();

        // char inst[16];
        // inst_to_str(&ctx, inst);
        // if(count>1000000){
        // // printf("%08lX - %X: %-12s (%02X %02X %02X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
        // //     emu_get_context()->ticks,
        // //     pc, inst, ctx.cur_opcode,
        // //     bus_read(pc + 1), bus_read(pc + 2), ctx.regs.a, flags, ctx.regs.b, ctx.regs.c,
        // //     ctx.regs.d, ctx.regs.e, ctx.regs.h, ctx.regs.l);
        // //     count =0;
        // printf("%X\n", pc);
        //     }
        //       printf("count %d\n",count);
        execute();
        // if (emu_get_context()->ticks - save_ticks >= 2000)
        //     if (cpu_get_context()->save_game && emu_get_context()->too == false)
        //     {
        //         save_ticks = emu_get_context()->ticks;
        //         emu_get_context()->too = true;
        //         save_wram();

        //         FILE *outfile;
        //         outfile = fopen("cpu.dat", "w");
        //         if (outfile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fwrite(&ctx, sizeof(cpu_context), 1, outfile);
        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");
        //         fclose(outfile);

        //         outfile = fopen("cart.dat", "wb");
        //         if (outfile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fwrite(cart_get_context(), sizeof(cart_context), 1, outfile);
        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");
        //         fclose(outfile);

        //         outfile = fopen("dma.dat", "wb");
        //         if (outfile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fwrite(dma_get_context(), sizeof(dma_context), 1, outfile);
        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");
        //         fclose(outfile);

        //         outfile = fopen("lcd.dat", "wb");
        //         if (outfile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fwrite(lcd_get_context(), sizeof(lcd_context), 1, outfile);
        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");
        //         fclose(outfile);

        //         outfile = fopen("ppu.dat", "wb");
        //         if (outfile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         savevideo_buffer();
        //         fwrite(ppu_get_context(), sizeof(ppu_context), 1, outfile);
        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");
        //         fclose(outfile);

        //         outfile = fopen("timer.dat", "wb");
        //         if (outfile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fwrite(timer_get_context(), sizeof(timer_context), 1, outfile);
        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");
        //         fclose(outfile);

        //         outfile = fopen("emu.dat", "wb");
        //         if (outfile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fwrite(emu_get_context(), sizeof(emu_context), 1, outfile);
        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");
        //         fclose(outfile);

        //         outfile = fopen("bank.dat", "wb");
        //         if (outfile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fwrite(cart_get_context()->ram_bank, 0x2000, 1, outfile);
        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");
        //         fclose(outfile);

        //         for (int i = 0; i < 16; i++)
        //         {
        //             char fn[1048];
        //             sprintf(fn, "bank%d.dat", i);
        //             outfile = fopen(fn, "wb");
        //             if (outfile == NULL)
        //             {
        //                 fprintf(stderr, "\nError opened file\n");
        //                 exit(1);
        //             }
        //             if (!cart_get_context()->ram_banks[i])
        //             {
        //                 fclose(outfile);
        //                 continue;
        //                 // return;
        //             }
        //             fwrite(cart_get_context()->ram_banks[i], 0x2000, 1, outfile);
        //             if (fwrite != 0)
        //                 printf("contents to file written successfully%d !\n", i);
        //             else
        //                 printf("error writing file !\n");
        //             fclose(outfile);
        //         }
        //         // FILE *fp;
        //         // fp = fopen("regist.txt", "w");
        //         printf("(%02X", ctx.regs.pc);
        //         printf("(%02X %02X %02X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
        //                ctx.cur_opcode,
        //                bus_read(ctx.regs.pc + 1), bus_read(ctx.regs.pc + 2), ctx.regs.a, flags, ctx.regs.b, ctx.regs.c,
        //                ctx.regs.d, ctx.regs.e, ctx.regs.h, ctx.regs.l);
        //         //    printf("%08lX - %d: %-12s (%02X %02X %02X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
        //         //     emu_get_context()->ticks,
        //         // printf("%08lX\n", emu_get_context()->ticks);
        //         inst_to_str(&ctx, inst);
        //         printf("%-12s\n", inst);
        //         // close(fp);
        //         // return false;
        //         emu_get_context()->too = false;
        //         cpu_get_context()->save_game = false;
        //     }
        // if (emu_get_context()->ticks - save_ticks >= 2000)
        //     if (cpu_get_context()->load_game && emu_get_context()->too == false)
        //     {
        //         save_ticks = emu_get_context()->ticks;
        //         emu_get_context()->too = true;
        //         // count ++ ;
        //         // timer_init();
        //         // cpu_init();
        //         // ppu_init();
        //         FILE *infile;
        //         cpu_context input;
        //         fprintf(stderr, "qw=-e=qw-e=qw-e=-wq%d\n", count);

        //         infile = fopen("cpu.dat", "rb");
        //         if (infile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fread(&ctx, sizeof(cpu_context), 1, infile);

        //         fclose(infile);
        //         // cart_context *tmp6 = cart_get_context();
        //         // cart_context input2;
        //         // infile = fopen("cart.dat", "rb");
        //         // if (infile == NULL)
        //         // {
        //         //     fprintf(stderr, "\nError opened file\n");
        //         //     exit(1);
        //         // }
        //         // fread(&input2, sizeof(cart_context), 1, infile);
        //         // // // while (fread(&input2, sizeof(cart_context), 1, infile))
        //         // // // {

        //         // // //     *tmp6 = input2;
        //         // // //     memset(tmp6->rom_bank_x, 0, 0x2000);
        //         //     // memcpy(tmp6->rom_bank_x, &input2.rom_bank_x, 0x2000);
        //         // // //     // tmp6->rom_bank_x = input2.rom_bank_x;
        //         // // // }
        //         // fclose(infile);

        //         dma_context *tmp = dma_get_context();
        //         dma_context input3;
        //         infile = fopen("dma.dat", "rb");
        //         if (infile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fread(dma_get_context(), sizeof(dma_context), 1, infile);
        //         fclose(infile);

        //         lcd_context *tmp2 = lcd_get_context();
        //         lcd_context input4;
        //         infile = fopen("lcd.dat", "rb");
        //         if (infile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         fread(lcd_get_context(), sizeof(lcd_context), 1, infile);
        //         fclose(infile);

        //         ppu_context *tmp3 = ppu_get_context();
        //         ppu_context input5;
        //         infile = fopen("ppu.dat", "rb");
        //         if (infile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }

        //         // memset((tmp3->fetched_entries), sizeof(oam_entry) * 3);
        //         // memset(tmp3->pfc.fetch_entry_data, sizeof(u8) * 6);
        //         // memset((tmp3->line_entry_array), sizeof(oam_line_entry) * 10);
        //         // memcpy((tmp3->fetched_entries), &(input5.fetched_entries), sizeof(oam_entry) * 3);
        //         // memcpy(tmp3->pfc.fetch_entry_data, &(input5.pfc.fetch_entry_data), sizeof(u8) * 6);
        //         // memcpy((tmp3->line_entry_array), &(input5.line_entry_array), sizeof(oam_line_entry) * 10);
        //         tmp3->pfc = input5.pfc;

        //         for (int i = 0; i < 3; i++)
        //         {
        //             tmp3->fetched_entries[i] = input5.fetched_entries[i];
        //         }

        //         tmp3->line_sprite_count = input5.line_sprite_count;
        //         tmp3->window_line = input5.window_line;

        //         tmp3->current_frame = input5.current_frame;
        //         tmp3->line_ticks = input5.line_ticks;
        //         tmp3->fetched_entry_count = input5.fetched_entry_count;

        //         readvideo_buffer();
        //         load_line_sprites();
        //         update_dbg_window();
        //         ui_update();
        //         read_wram();

        //         fprintf(stderr, "qw=-e=qw-e=qw-e=-wq22222222222%d\n", count);

        //         timer_context *tmp5 = timer_get_context();
        //         timer_context input7;
        //         infile = fopen("timer.dat", "rb");
        //         if (infile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         while (fread(&input7, sizeof(timer_context), 1, infile))
        //         {

        //             *tmp5 = input7;
        //         }
        //         fclose(infile);

        //         emu_context *tmp9 = emu_get_context();
        //         emu_context input9;
        //         infile = fopen("emu.dat", "rb");
        //         if (infile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }
        //         while (fread(&input9, sizeof(emu_context), 1, infile))
        //         {
        //             *tmp9 = input9;
        //         }

        //         fclose(infile);

        //         infile = fopen("bank.dat", "rb");
        //         if (infile == NULL)
        //         {
        //             fprintf(stderr, "\nError opened file\n");
        //             exit(1);
        //         }

        //         // memset(cart_get_context()->ram_bank, 0, 0x2000);
        //         fread(cart_get_context()->ram_bank, 0x2000, 1, infile);

        //         if (fwrite != 0)
        //             printf("contents to file written successfully !\n");
        //         else
        //             printf("error writing file !\n");

        //         fclose(infile);

        //         for (int i = 0; i < 16; i++)
        //         {
        //             char fn[1048];
        //             sprintf(fn, "bank%d.dat", i);
        //             infile = fopen(fn, "rb");
        //             if (infile == NULL)
        //             {
        //                 fprintf(stderr, "\nError opened file\n");
        //                 exit(1);
        //             }
        //             if (!cart_get_context()->ram_banks[i])
        //             {
        //                 fclose(infile);
        //                 continue;
        //                 // return;
        //             }
        //             // memset(cart_get_context()->ram_banks[i], 0, 0x2000);
        //             fread(cart_get_context()->ram_banks[i], 0x2000, 1, infile);
        //             if (fwrite != 0)
        //                 printf("contents to file written successfully%d !\n", i);
        //             else
        //                 printf("error writing file !\n");

        //             fclose(infile);
        //         }
        //         printf("(%02X", ctx.regs.pc);
        //         printf("(%02X %02X %02X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
        //                ctx.cur_opcode,
        //                bus_read(ctx.regs.pc + 1), bus_read(ctx.regs.pc + 2), ctx.regs.a, flags, ctx.regs.b, ctx.regs.c,
        //                ctx.regs.d, ctx.regs.e, ctx.regs.h, ctx.regs.l);
        //         emu_get_context()->too = false;
        //         cpu_get_context()->load_game = false;
        //     }
    }
    else
    {
        // is halted...
        emu_cycles(1);

        if (ctx.int_flags)
        {
            ctx.halted = false;
        }
    }

    if (ctx.int_master_enabled)
    {
        cpu_handle_interrupts(&ctx);
        ctx.enabling_ime = false;
    }

    if (ctx.enabling_ime)
    {
        ctx.int_master_enabled = true;
    }

    return true;
}

u8 cpu_get_ie_register()
{
    return ctx.ie_register;
}

void cpu_set_ie_register(u8 n)
{
    ctx.ie_register = n;
}

void cpu_request_interrupt(interrupt_type t)
{
    ctx.int_flags |= t;
}
