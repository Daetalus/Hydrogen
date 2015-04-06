
//
//  Debug
//


#ifndef DEBUG_H
#define DEBUG_H

#include "bytecode.h"


#define READ_BYTE() \
	(cursor++, (uint8_t) (*(cursor - 1)))

#define READ_2_BYTES() \
	(cursor += 2, ((uint16_t) *(cursor - 1) << (1 << 3)) | *(cursor - 2))

#define READ_4_BYTES()                           \
	(cursor += 4,                                \
		((uint32_t) *(cursor - 1) << (3 << 3)) | \
		((uint32_t) *(cursor - 2) << (2 << 3)) | \
		((uint32_t) *(cursor - 3) << (1 << 3)) | \
		(uint32_t) (*(cursor - 4)))

#define READ_8_BYTES()                           \
	(cursor += 8,                                \
		((uint64_t) *(cursor - 1) << (7 << 3)) | \
		((uint64_t) *(cursor - 2) << (6 << 3)) | \
		((uint64_t) *(cursor - 3) << (5 << 3)) | \
		((uint64_t) *(cursor - 4) << (4 << 3)) | \
		((uint64_t) *(cursor - 5) << (3 << 3)) | \
		((uint64_t) *(cursor - 6) << (2 << 3)) | \
		((uint64_t) *(cursor - 7) << (1 << 3)) | \
		((uint64_t) *(cursor - 8)))


// Pretty prints out the contents of a bytecode array.
void pretty_print_bytecode(Bytecode *bytecode);


#endif
