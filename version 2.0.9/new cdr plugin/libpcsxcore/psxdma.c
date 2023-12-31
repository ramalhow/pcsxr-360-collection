/***************************************************************************
*   Copyright (C) 2007 Ryan Schultz, PCSX-df Team, PCSX team              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.           *
***************************************************************************/

/*
* Handles PSX DMA functions.
*/

#include "psxdma.h"

// Dma0/1 in Mdec.c
// Dma3   in CdRom.c

static __inline void SPUDMA_INT(eCycle) { 
	psxRegs.interrupt |= (1 << PSXINT_SPUDMA); 
	psxRegs.intCycle[PSXINT_SPUDMA].cycle = eCycle; 
	psxRegs.intCycle[PSXINT_SPUDMA].sCycle = psxRegs.cycle;
}

static __inline void GPUOTCDMA_INT(eCycle) { 
	psxRegs.interrupt |= (1 << PSXINT_GPUOTCDMA); 
	psxRegs.intCycle[PSXINT_GPUOTCDMA].cycle = eCycle; 
	psxRegs.intCycle[PSXINT_GPUOTCDMA].sCycle = psxRegs.cycle; 
}


void spuInterrupt() {
//if(use_vm){
//	HW_DMA4_CHCR &= SWAP32(~0x01000000);
//	DMA_INTERRUPT(4);}
//else{
	HW_DMA4_CHCR_2 &= SWAP32(~0x01000000);
	DMA_INTERRUPT_2(4);//}//teste
}

void psxDma4(u32 madr, u32 bcr, u32 chcr) { // SPU
	u16 *ptr;
	u32 size;

	switch (chcr) {
		case 0x01000201: //cpu to spu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - mem2spu *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif

//if(use_vm){
//	ptr = (u16 *)PSXM(madr);}
//else{
	ptr = (u16 *)PSXM_2(madr);//}//teste
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA4 SPU - mem2spu *** NULL Pointer!!!\n");
#endif
				break;
			}
			SPU_writeDMAMem(ptr, (bcr >> 16) * (bcr & 0xffff) * 2);

			// Jungle Book - 0-0.333x DMA
			SPUDMA_INT((bcr >> 16) * (bcr & 0xffff) / 3);
			return;

		case 0x01000200: //spu to cpu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - spu2mem *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif

//if(use_vm){
//	ptr = (u16 *)PSXM(madr);}
//else{
	ptr = (u16 *)PSXM_2(madr);//}//teste
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA4 SPU - spu2mem *** NULL Pointer!!!\n");
#endif
				break;
			}
			size = (bcr >> 16) * (bcr & 0xffff) * 2;
			SPU_readDMAMem(ptr, size);
			psxCpu->Clear(madr, size);

			SPUDMA_INT((bcr >> 16) * (bcr & 0xffff) / 2);
			return;

#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA4 SPU - unknown *** %x addr = %x size = %x\n", chcr, madr, bcr);
			break;
#endif
	}
//if(use_vm){
//	HW_DMA4_CHCR &= SWAP32(~0x01000000);
//	DMA_INTERRUPT(4);}
//else{
	HW_DMA4_CHCR_2 &= SWAP32(~0x01000000);
	DMA_INTERRUPT_2(4);//}//teste
}


void psxDma6(u32 madr, u32 bcr, u32 chcr) {
	u32 size;
	u32 *mem;
//if(use_vm){
//	mem = (u32 *)PSXM(madr);}
//else{
	mem = (u32 *)PSXM_2(madr);//}//teste

#ifdef PSXDMA_LOG
	PSXDMA_LOG("*** DMA6 OT *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif

	if (chcr == 0x11000002) {
		if (mem == NULL) {
#ifdef CPU_LOG
			CPU_LOG("*** DMA6 OT *** NULL Pointer!!!\n");
#endif

//if(use_vm){
//	HW_DMA6_CHCR &= SWAP32(~0x01000000);
//	DMA_INTERRUPT(6);}
//else{
	HW_DMA6_CHCR_2 &= SWAP32(~0x01000000);
	DMA_INTERRUPT_2(6);//}//teste
			return;
		}

		// already 32-bit size
		size = bcr;

		while (bcr--) {
			*mem-- = SWAP32((madr - 4) & 0xffffff);
			madr -= 4;
		}
		mem++; 
		*mem = SWAP32(0xffffff);

		GPUOTCDMA_INT( size );
		return;
	}
#ifdef PSXDMA_LOG
	else {
		// Unknown option
		PSXDMA_LOG("*** DMA6 OT - unknown *** %x addr = %x size = %x\n", chcr, madr, bcr);
	}
#endif

//if(use_vm){
//	HW_DMA6_CHCR &= SWAP32(~0x01000000);
//	DMA_INTERRUPT(6);}
//else{
	HW_DMA6_CHCR_2 &= SWAP32(~0x01000000);
	DMA_INTERRUPT_2(6);//}//teste
}

void gpuotcInterrupt()
{
//if(use_vm){
//	HW_DMA6_CHCR &= SWAP32(~0x01000000);
//	DMA_INTERRUPT(6);}
//else{
	HW_DMA6_CHCR_2 &= SWAP32(~0x01000000);
	DMA_INTERRUPT_2(6);//}//teste
}
