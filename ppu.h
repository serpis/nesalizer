void set_ppu_cold_boot_state();
void reset_ppu();
void tick_ppu();

enum Sprite_size { EIGHT_BY_EIGHT = 0, EIGHT_BY_SIXTEEN };

extern unsigned scanline, dot;

extern unsigned ppu_addr_bus;

extern uint8_t *ciram;

extern uint64_t ppu_cycle;

uint8_t read_ppu_reg(unsigned n);
void write_ppu_reg(uint8_t value, unsigned n);

void write_oam_data_reg(uint8_t value);
