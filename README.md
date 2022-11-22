# LLD_gbemu_wasm
run gbemu in webassambly  
# build 
```bash
emcc --bind main.c ./lib/cpu_fetch.c ./lib/interrupts.c ./lib/ppu_pipeline.c ./lib/timer.c ./lib/dma.c ./lib/bus.c ./lib/cart.c ./lib/cpu_proc.c ./lib/emu.c ./lib/io.c ./lib/ppu_sm.c ./lib/ui.c ./lib/cpu_util.c ./lib/gamepad.c ./lib/lcd.c ./lib/ram.c ./lib/cpu.c ./lib/dbg.c ./lib/instructions.c ./lib/ppu.c ./lib/stack.c -I ./include  -o foo.html -s USE_SDL=2 -s USE_SDL_GFX=2 --preload-file LG.gb   -sALLOW_MEMORY_GROWTH -s ASYNCIFY
```

![image](https://imgur.com/LApSnni.gif)
