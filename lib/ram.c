#include "../include/common.h"

static ram_context ctx;
static u8 backwram[0x2000];
static u8 backhram[0x80];
ram_context *ram_get_context()
{
    return &ctx;
}
u8 *ram_get_context_wram()
{
    return &(ctx.wram);
}
void save_wram()
{

    mymemcpy(&backwram, &ctx.wram, sizeof(ctx.wram));
    mymemcpy(&backhram, &ctx.hram, sizeof(ctx.hram));

    FILE *fptr;
    fptr = fopen("wram.dat", "wb");
    if (fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }
    fwrite(&backwram, sizeof(backwram), 1, fptr);
    fclose(fptr);

    fptr = fopen("hram.dat", "wb");
    if (fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }

    fwrite(&backhram, sizeof(backhram), 1, fptr);
    fclose(fptr);
    // printf("error writing file %d!\n", sizeof(backwram));
}
void read_wram()
{

    FILE *fptr;
    fptr = fopen("wram.dat", "rb");
    if (fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }
    fread(&backwram, sizeof(backwram), 1, fptr);

    fclose(fptr);

    fptr = fopen("hram.dat", "rb");
    if (fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }

    fread(&backhram, sizeof(backhram), 1, fptr);
    fclose(fptr);

    mymemcpy(&ctx.wram, &backwram, sizeof(backwram));
    mymemcpy(&ctx.hram, &backhram, sizeof(backhram));
}
u8 wram_read(u16 address)
{
    address -= 0xC000;

    if (address >= 0x2000)
    {
        printf("INVALID WRAM ADDR %08X\n", address + 0xC000);
        exit(-1);
    }
    // printf("wram read\n");
    return ctx.wram[address];
}

void wram_write(u16 address, u8 value)
{
    address -= 0xC000;
    ctx.wram[address] = value;
    // printf("wram write\n");
    // u8 test[0x2000];
    // test =ram->hram;
    // ram_context *ram;

    // ram = ram_get_context();
    // mymemcpy(&test, &ctx.wram, sizeof(ram->wram));
    // FILE *fptr;

    // // // use appropriate location if you are using MacOS or Linux
    // fptr = fopen("program2.txt", "w+");

    // if (fptr == NULL)
    // {
    //     printf("Error!");
    //     exit(1);
    // }
    // 	// fseek(fptr,0,SEEK_SET);
    // // printf("Enter num: ");
    // // scanf("%d", &num);

    // fprintf(fptr, "%d", 1);
    // fclose(fptr);
}

u8 hram_read(u16 address)
{
    address -= 0xFF80;
    // printf("hram read\n");
    return ctx.hram[address];
}

void hram_write(u16 address, u8 value)
{
    address -= 0xFF80;
    // printf("hram write\n");
    ctx.hram[address] = value;
}
