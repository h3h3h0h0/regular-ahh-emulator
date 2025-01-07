//register encoding: ooooooss sssttttt dddddaaa aaffffff
#define OMASK 0b11111100000000000000000000000000
#define FMASK 0b00000000000000000000000000111111
#define SMASK 0b00000011111000000000000000000000
#define TMASK 0b00000000000111110000000000000000
#define DMASK 0b00000000000000001111100000000000
#define AMASK 0b00000000000000000000011111000000
//how much do you need to right-shift to get the actual value?
#define OSHIFT 26
#define FSHIFT 0
#define SSHIFT 21
#define TSHIFT 16
#define DSHIFT 11
#define ASHIFT 6

//immediate encoding: ooooooss sssttttt iiiiiiii iiiiiiii
#define IMASK 0b00000000000000001111111111111111

#define ISHIFT 0

//jump encoding: ooooooii iiiiiiii iiiiiiii iiiiiiii
#define JMASK 0b00000011111111111111111111111111

#define JSHIFT 0



