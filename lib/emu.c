#include "../include/common.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
// emcc --bind main.c ./lib/emu.c ./lib/cart.c ./lib/cart.c ./lib/cpu_proc.c ./lib/emu.c ./lib/io.c ./lib/ppu_sm.c ./lib/ui.c ./lib/cpu_util.c ./lib/gamepad.c ./lib/lcd.c ./lib/ram.c ./lib/cpu.c ./lib/dbg.c ./lib/instructions.c ./lib/ppu.c ./lib/stack.c -I ./include   -o foo.html -s USE_SDL=2 -s USE_SDL_GFX=2 --preload-file LG.gb
//  emcc --bind main.c ./lib/emu.c ./lib/cart.c ./lib/cart.c ./lib/cpu_proc.c ./lib/emu.c ./lib/io.c ./lib/ppu_sm.c ./lib/ui.c ./lib/cpu_util.c ./lib/gamepad.c ./lib/lcd.c ./lib/ram.c ./lib/cpu.c ./lib/dbg.c ./lib/instructions.c ./lib/ppu.c ./lib/stack.c -I ./include   -o foo.html -s USE_SDL=2 -s USE_SDL_GFX=2 --preload-file LG.gb
// TODO Add Windows Alternative...

/*
  Emu components:

  |Cart|
  |CPU|
  |Address Bus|
  |PPU|
  |Timer|

*/
static emu_context ctx;
u32 prev_frame = 0;
int count2 = 0;

// EMSCRIPTEN_KEEPALIVE
void handle_events()
{
    // while (ctx.running)
    // {

    // if (ctx.too)
    //     return;

    // if (ctx.paused)
    // {
    //     delay(10);
    //     return;
    // }
    // if (!cpu_step())
    // {

    //     printf("CPU Stopped\n");
    //     return ;
    // }
    // count2++;
    // if (count2 >= 300)
    // {
    //     // usleep(100);
    //     // ui_handle_events();
    //     //  printf("CPU Stopped\n");
    //     if (prev_frame != ppu_get_context()->current_frame)
    //     {
    //         //   printf("CPU Stopped\n");
    //         ui_update();
    //     }

    //     prev_frame = ppu_get_context()->current_frame;
    //     count2 = 0;
    // }
    usleep(100);
    ui_handle_events();
    ui_update();
    // printf("CPU Stopped\n");
    // continue;
}
//   return

void run_main_loop()
{
#ifdef __EMSCRIPTEN__
    printf("Cart loaded..\n");
    emscripten_set_main_loop(handle_events, 60, true);
#else
    printf("Cart loaded2..\n");
    while (ctx.running)
    {
        handle_events();
    }
#endif
}

emu_context *emu_get_context()
{
    return &ctx;
}

void *cpu_run(void *p)
{

    timer_init();
    cpu_init();
    ppu_init();

    ctx.running = true;
    ctx.paused = false;
    ctx.too = false;
    ctx.ticks = 0;
    //   cpu_set_flags(ctx,  ctx.regs.a, 1, (val & 0x0F) == 0x0F, -1);
    // fp = fopen("regist.txt", "w");
    while (ctx.running)
    {

        if (ctx.too)
            break;

        if (ctx.paused)
        {
            delay(10);
            continue;
        }
        if (!cpu_step())
        {

            printf("CPU Stopped\n");
            return 0;
        }
        // if (count2 >= 2000000 && count2 <= 4000000)
        //     continue;
    }

    return 0;
}

int emu_run()
{
    // if (argc < 2)
    // {
    //     printf("Usage: emu <rom_file>\n");
    //     return -1;
    // }
    if (!cart_load("LG.gb"))
    // if (!cart_load(argv[1]))
    {
        printf("Failed to load ROM file: %s\n", "LG.gb");
        return -2;
    }

    printf("Cart loaded..\n");

    ui_init();

    timer_init();
    cpu_init();
    ppu_init();
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    if (audio_init() < 0) {
        printf("Audio initialization failed.\n");
        SDL_Quit(); // 清理 SDL
        return -1;
    }
    ctx.running = true;
    ctx.paused = false;
    ctx.too = false;
    ctx.ticks = 0;
    // u32 prev_frame = 0;
    // int count2 = 0;
    //   cpu_set_flags(ctx,  ctx.regs.a, 1, (val & 0x0F) == 0x0F, -1);
    // fp = fopen("regist.txt", "w");

    pthread_t t1;

    if (pthread_create(&t1, NULL, cpu_run, NULL))
    {
        fprintf(stderr, "FAILED TO START MAIN CPU THREAD!\n");
        return -1;
    }

    u32 prev_frame = 0;
    run_main_loop();
    //    printf("Cart loaded2..\n");

    // while (!ctx.die)
    // {
    //     while (ctx.too)
    //         ;

    //     usleep(100);
    //     ui_handle_events();

    //     if (prev_frame != ppu_get_context()->current_frame)
    //     {
    //         ui_update();
    //     }

    //     prev_frame = ppu_get_context()->current_frame;
    //     // printf("%d\n",prev_frame);
    // }

    // while (ctx.running)
    // {
    //     handle_events();
    // }
    //     while (ctx.running)
    //     {

    //         if (ctx.too)
    //             break;

    //         if (ctx.paused)
    //         {
    //             delay(10);
    //             continue;
    //         }
    //         if (!cpu_step())
    //         {

    //             printf("CPU Stopped\n");
    //             return 0;
    //         }
    //         count2++;
    //         if (count2 >=300)
    //         {
    //         // usleep(100);
    //         // ui_handle_events();
    // //  printf("CPU Stopped\n");
    //         if (prev_frame != ppu_get_context()->current_frame)
    //         {
    //             //   printf("CPU Stopped\n");
    //             ui_update();
    //         }

    //         prev_frame = ppu_get_context()->current_frame;
    //         count2 =0;
    //         }
    //             // continue;
    //     }
    // pthread_t t1;

    // if (pthread_create(&t1, NULL, cpu_run, NULL))
    // {
    //     fprintf(stderr, "FAILED TO START MAIN CPU THREAD!\n");
    //     return -1;
    // }

    // u32 prev_frame = 0;

    // while (!ctx.die)
    // {
    //     while (ctx.too)
    //         ;

    //     usleep(100);
    //     ui_handle_events();

    //     if (prev_frame != ppu_get_context()->current_frame)
    //     {
    //         ui_update();
    //     }

    //     prev_frame = ppu_get_context()->current_frame;
    //     // printf("%d\n",prev_frame);
    // }

    return 0;
}

void emu_cycles(int cpu_cycles)
{

    for (int i = 0; i < cpu_cycles; i++)
    {
        for (int n = 0; n < 4; n++)
        {
            ctx.ticks++;
            timer_tick();
            ppu_tick();
        }

        dma_tick();
    }
}
