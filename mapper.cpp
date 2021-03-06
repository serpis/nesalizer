#include "common.h"

#include "mapper.h"
#include "rom.h"

// NROM
void mapper_0_init();
void mapper_0_write(uint8_t, uint16_t);

// SxROM, all of which use the Nintendo MMC1
void mapper_1_init();
void mapper_1_write(uint8_t, uint16_t);

// Most common configuration of the UxROM boardset
void mapper_2_init();
void mapper_2_write(uint8_t, uint16_t);

// CNROM board and a very similar board used for Panesian games
void mapper_3_init();
void mapper_3_write(uint8_t, uint16_t);

// "iNES Mapper 004 is a wide abstraction that can represent boards using the
// Nintendo MMC3, Nintendo MMC6, or functional clones of any of the above. Most
// games utilizing TxROM, DxROM, and HKROM boards use this designation."
void mapper_4_init();
void mapper_4_write(uint8_t, uint16_t);
void mmc3_ppu_tick_callback();

// AxROM - Rare games often use this one
void mapper_7_init();
void mapper_7_write(uint8_t, uint16_t);

// MMC2 - only used by Punch-Out!!
void mapper_9_init();
void mapper_9_write(uint8_t, uint16_t);
void mmc2_ppu_tick_callback();

// Color Dreams
void mapper_11_init();
void mapper_11_write(uint8_t, uint16_t);

// Mapper-2-ish
void mapper_71_init();
void mapper_71_write(uint8_t, uint16_t);

// Camerica/Capcom mapper used by the Quattro * games

static void nop_write(uint8_t, uint16_t) {}
static void nop_ppu_tick_callback() {}

// Implicitly zero-initialized
Mapper_fns mapper_functions[256];

write_fn *write_mapper;
ppu_tick_callback_fn *ppu_tick_callback;

void init_mappers() {
    #define MAPPER(n, write_fn, _ppu_tick_callback)               \
      mapper_functions[n].init  = mapper_##n##_init;              \
      mapper_functions[n].write = write_fn;                       \
      mapper_functions[n].ppu_tick_callback = _ppu_tick_callback;

    MAPPER(0 , nop_write      , nop_ppu_tick_callback )
    MAPPER(1 , mapper_1_write , nop_ppu_tick_callback )
    MAPPER(2 , mapper_2_write , nop_ppu_tick_callback )
    MAPPER(3 , mapper_3_write , nop_ppu_tick_callback )
    MAPPER(4 , mapper_4_write , mmc3_ppu_tick_callback)
    MAPPER(7 , mapper_7_write , nop_ppu_tick_callback )
    MAPPER(9 , mapper_9_write , mmc2_ppu_tick_callback)
    MAPPER(11, mapper_11_write, nop_ppu_tick_callback )
    MAPPER(71, mapper_71_write, nop_ppu_tick_callback )

    #undef MAPPER
}

//
// Memory mapping
//

// Each PRG page is 8 KB to account for the finest granularity switched by any
// mapper
uint8_t *prg_pages[4];
// Each 1 KB big
uint8_t *chr_pages[8];

uint8_t *prg_ram;

void set_prg_32k_bank(unsigned bank) {
    if (prg_16k_banks == 1) {
        // The only configuration for a single 16k PRG bank is to be mirrored
        // in $8000-$BFFF and $C000-$FFFF
        prg_pages[0] = prg_pages[2] = prg_base;
        prg_pages[1] = prg_pages[3] = prg_base + 0x2000;
    }
    else {
        uint8_t *const bank_ptr = prg_base + 0x8000*(bank & (prg_16k_banks/2 - 1));
        for (unsigned i = 0; i < 4; ++i)
            prg_pages[i] = bank_ptr + 0x2000*i;
    }
}

void set_prg_16k_bank(unsigned n, unsigned bank) {
    uint8_t *const bank_ptr = prg_base + 0x4000*(bank & (prg_16k_banks - 1));
    for (unsigned i = 0; i < 2; ++i)
        prg_pages[2*n + i] = bank_ptr + 0x2000*i;
}

void set_prg_8k_bank(unsigned n, unsigned bank) {
    prg_pages[n] = prg_base + 0x2000*(bank & (2*prg_16k_banks - 1));
}

void set_chr_8k_bank(unsigned bank) {
    uint8_t *const bank_ptr = chr_base + 0x2000*(bank & (chr_8k_banks - 1));
    for (unsigned i = 0; i < 8; ++i)
        chr_pages[i] = bank_ptr + 0x400*i;
}

void set_chr_4k_bank(unsigned n, unsigned bank) {
    uint8_t *const bank_ptr = chr_base + 0x1000*(bank & (2*chr_8k_banks - 1));
    for (unsigned i = 0; i < 4; ++i)
        chr_pages[4*n + i] = bank_ptr + 0x400*i;
}

void set_chr_2k_bank(unsigned n, unsigned bank) {
    uint8_t *const bank_ptr = chr_base + 0x800*(bank & (4*chr_8k_banks - 1));
    for (unsigned i = 0; i < 2; ++i)
        chr_pages[2*n + i] = bank_ptr + 0x400*i;
}

void set_chr_1k_bank(unsigned n, unsigned bank) {
    chr_pages[n] = chr_base + 0x400*(bank & (8*chr_8k_banks - 1));
}

//
// Mirroring
//

Mirroring mirroring;

void set_mirroring(Mirroring m) {
    // In four-screen mode, the cart is assumed to be wired so that the mapper
    // can't influence mirroring
    if (mirroring != FOUR_SCREEN)
        mirroring = m;
}
