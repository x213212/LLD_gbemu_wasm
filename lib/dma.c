#include "../include/common.h"

static dma_context ctx;

void dma_start(u8 start) {
    ctx.active = true;
    ctx.byte = 0;
    ctx.start_delay = 2;
    ctx.value = start;
}

void dma_tick() {
    if (!ctx.active) {
        return;
    }

    if (ctx.start_delay) {
        ctx.start_delay--;
        return;
    }

    ppu_oam_write(ctx.byte, bus_read((ctx.value * 0x100) + ctx.byte));

    ctx.byte++;

    ctx.active = ctx.byte < 0xA0;
}

bool dma_transferring() {
    return ctx.active;
}


dma_context *dma_get_context() {
    return &ctx;
}
