// Generated from FXDRIVER FM routines by research/tools/lift_music.py.
void MusicDriver::execute(uint16_t entry){
switch(entry){
case 0x2993:goto L2993;
case 0x2a3f:goto L2a3f;
case 0x2aa1:goto L2aa1;
case 0x2af5:goto L2af5;
case 0x2b1d:goto L2b1d;
case 0x2b45:goto L2b45;
case 0x2bfa:goto L2bfa;
case 0x2c79:goto L2c79;
case 0x2cbb:goto L2cbb;
case 0x2d75:goto L2d75;
case 0x2de1:goto L2de1;
case 0x2efa:goto L2efa;
case 0x2f55:goto L2f55;
case 0x3031:goto L3031;
case 0x307d:goto L307d;
case 0x3151:goto L3151;
case 0x3176:goto L3176;
case 0x31d8:goto L31d8;
case 0x322b:goto L322b;
case 0x327e:goto L327e;
case 0x3326:goto L3326;
case 0x337e:goto L337e;
case 0x33c8:goto L33c8;
case 0x34ad:goto L34ad;
case 0x3afd:goto L3afd;
case 0x3b63:goto L3b63;
case 0x3b98:goto L3b98;
case 0x4150:goto L4150;
default:throw std::runtime_error("Unknown FM routine");
}
L2993: step(); push(bp); // 2993: push bp
step(); bp=uint16_t(sp); // 2994: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 2996: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 2997: dec sp
step(); push(ds); // 2998: push ds
step(); ax=uint16_t(5249); // 2999: mov ax, 0x1481
step(); ds=uint16_t(ax); // 299c: mov ds, ax
step(); write16(ss, uint16_t(-2+bp), 1); // 299e: mov word ptr [bp - 2], 1
step(); goto L29b4; // 29a3: jmp 0x29b4
L29a5: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 29a5: xor ax, ax
step(); push(ax); // 29a7: push ax
step(); push(read16(ss, uint16_t(-2+bp))); // 29a8: push word ptr [bp - 2]
step(); push(cs); // 29ab: push cs
step(); push(0x8f); adapter(0x2928); // 29ac: call 0x2928
step(); cx=uint16_t(pop()); // 29af: pop cx
step(); cx=uint16_t(pop()); // 29b0: pop cx
step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 29b1: inc word ptr [bp - 2]
L29b4: step(); alu(read16(ss, uint16_t(-2+bp)),245,16,Op::SUB); // 29b4: cmp word ptr [bp - 2], 0xf5
step(); if(zf||(sf!=of)){goto L29a5;} // 29b9: jle 0x29a5
step(); ax=uint16_t(6); // 29bb: mov ax, 6
step(); push(ax); // 29be: push ax
step(); ax=uint16_t(4); // 29bf: mov ax, 4
step(); push(ax); // 29c2: push ax
step(); push(cs); // 29c3: push cs
step(); push(0xa7); adapter(0x2928); // 29c4: call 0x2928
step(); cx=uint16_t(pop()); // 29c7: pop cx
step(); cx=uint16_t(pop()); // 29c8: pop cx
step(); write16(ss, uint16_t(-2+bp), 0); // 29c9: mov word ptr [bp - 2], 0
step(); goto L29ee; // 29ce: jmp 0x29ee
L29d0: step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 29d0: mov bx, word ptr [bp - 2]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 29d3: shl bx, 1
step(); write16(ds, uint16_t(472+bx), 8192); // 29d5: mov word ptr [bx + 0x1d8], 0x2000
step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 29db: mov bx, word ptr [bp - 2]
step(); write8(ds, uint16_t(337+bx), 0); // 29de: mov byte ptr [bx + 0x151], 0
step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 29e3: mov bx, word ptr [bp - 2]
step(); write8(ds, uint16_t(368+bx), 0); // 29e6: mov byte ptr [bx + 0x170], 0
step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 29eb: inc word ptr [bp - 2]
L29ee: step(); alu(read16(ss, uint16_t(-2+bp)),9,16,Op::SUB); // 29ee: cmp word ptr [bp - 2], 9
step(); if(sf!=of){goto L29d0;} // 29f2: jl 0x29d0
step(); write16(ss, uint16_t(-2+bp), 0); // 29f4: mov word ptr [bp - 2], 0
step(); goto L2a06; // 29f9: jmp 0x2a06
L29fb: step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 29fb: mov bx, word ptr [bp - 2]
step(); write8(ds, uint16_t(346+bx), 127); // 29fe: mov byte ptr [bx + 0x15a], 0x7f
step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 2a03: inc word ptr [bp - 2]
L2a06: step(); alu(read16(ss, uint16_t(-2+bp)),11,16,Op::SUB); // 2a06: cmp word ptr [bp - 2], 0xb
step(); if(sf!=of){goto L29fb;} // 2a0a: jl 0x29fb
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2a0c: xor ax, ax
step(); push(ax); // 2a0e: push ax
step(); push(cs); // 2a0f: push cs
step(); push(0xf3); execute(0x2a3f); // 2a10: call 0x2a3f
step(); ; // 2a13: nop
step(); cx=uint16_t(pop()); // 2a14: pop cx
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2a15: xor ax, ax
step(); push(ax); // 2a17: push ax
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2a18: xor ax, ax
step(); push(ax); // 2a1a: push ax
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2a1b: xor ax, ax
step(); push(ax); // 2a1d: push ax
step(); push(cs); // 2a1e: push cs
step(); push(0x102); execute(0x2b1d); // 2a1f: call 0x2b1d
step(); ; // 2a22: nop
step(); sp=uint16_t(alu(sp,6,16,Op::ADD)); // 2a23: add sp, 6
step(); ax=uint16_t(1); // 2a26: mov ax, 1
step(); push(ax); // 2a29: push ax
step(); push(cs); // 2a2a: push cs
step(); push(0x10e); execute(0x2af5); // 2a2b: call 0x2af5
step(); ; // 2a2e: nop
step(); cx=uint16_t(pop()); // 2a2f: pop cx
step(); ax=uint16_t(1); // 2a30: mov ax, 1
step(); push(ax); // 2a33: push ax
step(); push(cs); // 2a34: push cs
step(); push(0x118); execute(0x2aa1); // 2a35: call 0x2aa1
step(); ; // 2a38: nop
step(); cx=uint16_t(pop()); // 2a39: pop cx
step(); ds=uint16_t(pop()); // 2a3a: pop ds
step(); sp=uint16_t(bp); // 2a3b: mov sp, bp
step(); bp=uint16_t(pop()); // 2a3d: pop bp
step(); pop();cs=pop();return; // 2a3e: retf
L2a3f: step(); push(bp); // 2a3f: push bp
step(); bp=uint16_t(sp); // 2a40: mov bp, sp
step(); push(ds); // 2a42: push ds
step(); ax=uint16_t(5249); // 2a43: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2a46: mov ds, ax
step(); alu(read16(ss, uint16_t(6+bp)),0,16,Op::SUB); // 2a48: cmp word ptr [bp + 6], 0
step(); if(zf){goto L2a78;} // 2a4c: je 0x2a78
step(); write8(ds, uint16_t(376), 24); // 2a4e: mov byte ptr [0x178], 0x18
step(); write16(ds, uint16_t(488), 8192); // 2a53: mov word ptr [0x1e8], 0x2000
step(); ax=uint16_t(8); // 2a59: mov ax, 8
step(); push(ax); // 2a5c: push ax
step(); push(cs); // 2a5d: push cs
step(); push(0x141); execute(0x33c8); // 2a5e: call 0x33c8
step(); ; // 2a61: nop
step(); cx=uint16_t(pop()); // 2a62: pop cx
step(); write8(ds, uint16_t(375), 31); // 2a63: mov byte ptr [0x177], 0x1f
step(); write16(ds, uint16_t(486), 8192); // 2a68: mov word ptr [0x1e6], 0x2000
step(); ax=uint16_t(7); // 2a6e: mov ax, 7
step(); push(ax); // 2a71: push ax
step(); push(cs); // 2a72: push cs
step(); push(0x156); execute(0x33c8); // 2a73: call 0x33c8
step(); ; // 2a76: nop
step(); cx=uint16_t(pop()); // 2a77: pop cx
L2a78: step(); set_lo(ax,read8(ss, uint16_t(6+bp))); // 2a78: mov al, byte ptr [bp + 6]
step(); write8(ds, uint16_t(5424), lo(ax)); // 2a7b: mov byte ptr [0x1530], al
step(); alu(read16(ss, uint16_t(6+bp)),0,16,Op::SUB); // 2a7e: cmp word ptr [bp + 6], 0
step(); if(zf){goto L2a89;} // 2a82: je 0x2a89
step(); ax=uint16_t(11); // 2a84: mov ax, 0xb
step(); goto L2a8c; // 2a87: jmp 0x2a8c
L2a89: step(); ax=uint16_t(9); // 2a89: mov ax, 9
L2a8c: step(); write16(ds, uint16_t(360), ax); // 2a8c: mov word ptr [0x168], ax
step(); write8(ds, uint16_t(4910), 0); // 2a8f: mov byte ptr [0x132e], 0
step(); push(cs); // 2a94: push cs
step(); push(0x178); execute(0x2de1); // 2a95: call 0x2de1
step(); ; // 2a98: nop
step(); push(cs); // 2a99: push cs
step(); push(0x17d); execute(0x3326); // 2a9a: call 0x3326
step(); ; // 2a9d: nop
step(); ds=uint16_t(pop()); // 2a9e: pop ds
step(); bp=uint16_t(pop()); // 2a9f: pop bp
step(); pop();cs=pop();return; // 2aa0: retf
L2aa1: step(); push(bp); // 2aa1: push bp
step(); bp=uint16_t(sp); // 2aa2: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 2aa4: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 2aa5: dec sp
step(); push(ds); // 2aa6: push ds
step(); ax=uint16_t(5249); // 2aa7: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2aaa: mov ds, ax
step(); alu(read16(ss, uint16_t(6+bp)),0,16,Op::SUB); // 2aac: cmp word ptr [bp + 6], 0
step(); if(zf){goto L2ab7;} // 2ab0: je 0x2ab7
step(); ax=uint16_t(32); // 2ab2: mov ax, 0x20
step(); goto L2ab9; // 2ab5: jmp 0x2ab9
L2ab7: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2ab7: xor ax, ax
L2ab9: step(); write16(ds, uint16_t(366), ax); // 2ab9: mov word ptr [0x16e], ax
step(); write16(ss, uint16_t(-2+bp), 0); // 2abc: mov word ptr [bp - 2], 0
step(); goto L2adc; // 2ac1: jmp 0x2adc
L2ac3: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2ac3: xor ax, ax
step(); push(ax); // 2ac5: push ax
step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 2ac6: mov bx, word ptr [bp - 2]
step(); set_lo(ax,read8(ds, uint16_t(5542+bx))); // 2ac9: mov al, byte ptr [bx + 0x15a6]
step(); set_hi(ax,0); // 2acd: mov ah, 0
step(); ax=uint16_t(alu(ax,224,16,Op::ADD)); // 2acf: add ax, 0xe0
step(); push(ax); // 2ad2: push ax
step(); push(cs); // 2ad3: push cs
step(); push(0x1b7); adapter(0x2928); // 2ad4: call 0x2928
step(); cx=uint16_t(pop()); // 2ad7: pop cx
step(); cx=uint16_t(pop()); // 2ad8: pop cx
step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 2ad9: inc word ptr [bp - 2]
L2adc: step(); alu(read16(ss, uint16_t(-2+bp)),18,16,Op::SUB); // 2adc: cmp word ptr [bp - 2], 0x12
step(); if(sf!=of){goto L2ac3;} // 2ae0: jl 0x2ac3
step(); push(read16(ds, uint16_t(366))); // 2ae2: push word ptr [0x16e]
step(); ax=uint16_t(1); // 2ae6: mov ax, 1
step(); push(ax); // 2ae9: push ax
step(); push(cs); // 2aea: push cs
step(); push(0x1ce); adapter(0x2928); // 2aeb: call 0x2928
step(); cx=uint16_t(pop()); // 2aee: pop cx
step(); cx=uint16_t(pop()); // 2aef: pop cx
step(); ds=uint16_t(pop()); // 2af0: pop ds
step(); sp=uint16_t(bp); // 2af1: mov sp, bp
step(); bp=uint16_t(pop()); // 2af3: pop bp
step(); pop();cs=pop();return; // 2af4: retf
L2af5: step(); push(bp); // 2af5: push bp
step(); bp=uint16_t(sp); // 2af6: mov bp, sp
step(); push(ds); // 2af8: push ds
step(); ax=uint16_t(5249); // 2af9: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2afc: mov ds, ax
step(); alu(read16(ss, uint16_t(6+bp)),12,16,Op::SUB); // 2afe: cmp word ptr [bp + 6], 0xc
step(); if(cf||zf){goto L2b09;} // 2b02: jbe 0x2b09
step(); write16(ss, uint16_t(6+bp), 12); // 2b04: mov word ptr [bp + 6], 0xc
L2b09: step(); alu(read16(ss, uint16_t(6+bp)),1,16,Op::SUB); // 2b09: cmp word ptr [bp + 6], 1
step(); if(!cf){goto L2b14;} // 2b0d: jae 0x2b14
step(); write16(ss, uint16_t(6+bp), 1); // 2b0f: mov word ptr [bp + 6], 1
L2b14: step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2b14: mov ax, word ptr [bp + 6]
step(); write16(ds, uint16_t(4912), ax); // 2b17: mov word ptr [0x1330], ax
step(); ds=uint16_t(pop()); // 2b1a: pop ds
step(); bp=uint16_t(pop()); // 2b1b: pop bp
step(); pop();cs=pop();return; // 2b1c: retf
L2b1d: step(); push(bp); // 2b1d: push bp
step(); bp=uint16_t(sp); // 2b1e: mov bp, sp
step(); push(ds); // 2b20: push ds
step(); ax=uint16_t(5249); // 2b21: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2b24: mov ds, ax
step(); set_lo(ax,read8(ss, uint16_t(6+bp))); // 2b26: mov al, byte ptr [bp + 6]
step(); write8(ds, uint16_t(4906), lo(ax)); // 2b29: mov byte ptr [0x132a], al
step(); set_lo(ax,read8(ss, uint16_t(8+bp))); // 2b2c: mov al, byte ptr [bp + 8]
step(); write8(ds, uint16_t(4911), lo(ax)); // 2b2f: mov byte ptr [0x132f], al
step(); set_lo(ax,read8(ss, uint16_t(10+bp))); // 2b32: mov al, byte ptr [bp + 0xa]
step(); write8(ds, uint16_t(336), lo(ax)); // 2b35: mov byte ptr [0x150], al
step(); push(cs); // 2b38: push cs
step(); push(0x21c); execute(0x3326); // 2b39: call 0x3326
step(); ; // 2b3c: nop
step(); push(cs); // 2b3d: push cs
step(); push(0x221); execute(0x3151); // 2b3e: call 0x3151
step(); ; // 2b41: nop
step(); ds=uint16_t(pop()); // 2b42: pop ds
step(); bp=uint16_t(pop()); // 2b43: pop bp
step(); pop();cs=pop();return; // 2b44: retf
L2b45: step(); push(bp); // 2b45: push bp
step(); bp=uint16_t(sp); // 2b46: mov bp, sp
step(); sp=uint16_t(alu(sp,16,16,Op::SUB)); // 2b48: sub sp, 0x10
step(); push(ds); // 2b4b: push ds
step(); ax=uint16_t(5249); // 2b4c: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2b4f: mov ds, ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2b51: mov ax, word ptr [bp + 6]
step(); alu(ax,read16(ds, uint16_t(360)),16,Op::SUB); // 2b54: cmp ax, word ptr [0x168]
step(); if(cf){goto L2b5d;} // 2b58: jb 0x2b5d
step(); goto L2bf5; // 2b5a: jmp 0x2bf5
L2b5d: step(); ax=uint16_t(read16(ss, uint16_t(10+bp))); // 2b5d: mov ax, word ptr [bp + 0xa]
step(); dx=uint16_t(read16(ss, uint16_t(8+bp))); // 2b60: mov dx, word ptr [bp + 8]
step(); dx=uint16_t(alu(dx,52,16,Op::ADD)); // 2b63: add dx, 0x34
step(); write16(ss, uint16_t(-10+bp), ax); // 2b66: mov word ptr [bp - 0xa], ax
step(); write16(ss, uint16_t(-12+bp), dx); // 2b69: mov word ptr [bp - 0xc], dx
step(); {const auto location=uint16_t(-12+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2b6c: les bx, ptr [bp - 0xc]
step(); ax=uint16_t(read16(es, uint16_t(0+bx))); // 2b6f: mov ax, word ptr es:[bx]
step(); write16(ss, uint16_t(-2+bp), ax); // 2b72: mov word ptr [bp - 2], ax
step(); write16(ss, uint16_t(-12+bp), alu(read16(ss, uint16_t(-12+bp)),2,16,Op::ADD)); // 2b75: add word ptr [bp - 0xc], 2
step(); {const auto location=uint16_t(-12+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2b79: les bx, ptr [bp - 0xc]
step(); ax=uint16_t(read16(es, uint16_t(0+bx))); // 2b7c: mov ax, word ptr es:[bx]
step(); write16(ss, uint16_t(-4+bp), ax); // 2b7f: mov word ptr [bp - 4], ax
step(); ax=uint16_t(read16(ss, uint16_t(10+bp))); // 2b82: mov ax, word ptr [bp + 0xa]
step(); dx=uint16_t(read16(ss, uint16_t(8+bp))); // 2b85: mov dx, word ptr [bp + 8]
step(); dx=uint16_t(alu(dx,26,16,Op::ADD)); // 2b88: add dx, 0x1a
step(); write16(ss, uint16_t(-6+bp), ax); // 2b8b: mov word ptr [bp - 6], ax
step(); write16(ss, uint16_t(-8+bp), dx); // 2b8e: mov word ptr [bp - 8], dx
step(); alu(read8(ds, uint16_t(5424)),0,8,Op::SUB); // 2b91: cmp byte ptr [0x1530], 0
step(); if(zf){goto L2ba8;} // 2b96: je 0x2ba8
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2b98: mov ax, word ptr [bp + 6]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 2b9b: shl ax, 1
step(); ax=uint16_t(alu(ax,18,16,Op::ADD)); // 2b9d: add ax, 0x12
step(); write16(ss, uint16_t(-14+bp), ds); // 2ba0: mov word ptr [bp - 0xe], ds
step(); write16(ss, uint16_t(-16+bp), ax); // 2ba3: mov word ptr [bp - 0x10], ax
step(); goto L2bb6; // 2ba6: jmp 0x2bb6
L2ba8: step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2ba8: mov ax, word ptr [bp + 6]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 2bab: shl ax, 1
step(); ax=uint16_t(alu(ax,0,16,Op::ADD)); // 2bad: add ax, 0
step(); write16(ss, uint16_t(-14+bp), ds); // 2bb0: mov word ptr [bp - 0xe], ds
step(); write16(ss, uint16_t(-16+bp), ax); // 2bb3: mov word ptr [bp - 0x10], ax
L2bb6: step(); push(read16(ss, uint16_t(-2+bp))); // 2bb6: push word ptr [bp - 2]
step(); push(read16(ss, uint16_t(10+bp))); // 2bb9: push word ptr [bp + 0xa]
step(); push(read16(ss, uint16_t(8+bp))); // 2bbc: push word ptr [bp + 8]
step(); {const auto location=uint16_t(-16+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2bbf: les bx, ptr [bp - 0x10]
step(); set_lo(ax,read8(es, uint16_t(0+bx))); // 2bc2: mov al, byte ptr es:[bx]
step(); set_hi(ax,0); // 2bc5: mov ah, 0
step(); push(ax); // 2bc7: push ax
step(); push(cs); // 2bc8: push cs
step(); push(0x2ac); execute(0x2efa); // 2bc9: call 0x2efa
step(); ; // 2bcc: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2bcd: add sp, 8
step(); {const auto location=uint16_t(-16+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2bd0: les bx, ptr [bp - 0x10]
step(); alu(read8(es, uint16_t(1+bx)),255,8,Op::SUB); // 2bd3: cmp byte ptr es:[bx + 1], 0xff
step(); if(zf){goto L2bf5;} // 2bd8: je 0x2bf5
step(); push(read16(ss, uint16_t(-4+bp))); // 2bda: push word ptr [bp - 4]
step(); push(read16(ss, uint16_t(-6+bp))); // 2bdd: push word ptr [bp - 6]
step(); push(read16(ss, uint16_t(-8+bp))); // 2be0: push word ptr [bp - 8]
step(); {const auto location=uint16_t(-16+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2be3: les bx, ptr [bp - 0x10]
step(); set_lo(ax,read8(es, uint16_t(1+bx))); // 2be6: mov al, byte ptr es:[bx + 1]
step(); set_hi(ax,0); // 2bea: mov ah, 0
step(); push(ax); // 2bec: push ax
step(); push(cs); // 2bed: push cs
step(); push(0x2d1); execute(0x2efa); // 2bee: call 0x2efa
step(); ; // 2bf1: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2bf2: add sp, 8
L2bf5: step(); ds=uint16_t(pop()); // 2bf5: pop ds
step(); sp=uint16_t(bp); // 2bf6: mov sp, bp
step(); bp=uint16_t(pop()); // 2bf8: pop bp
step(); pop();cs=pop();return; // 2bf9: retf
L2bfa: step(); push(bp); // 2bfa: push bp
step(); bp=uint16_t(sp); // 2bfb: mov bp, sp
step(); sp=uint16_t(alu(sp,4,16,Op::SUB)); // 2bfd: sub sp, 4
step(); push(ds); // 2c00: push ds
step(); ax=uint16_t(5249); // 2c01: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2c04: mov ds, ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2c06: mov ax, word ptr [bp + 6]
step(); alu(ax,read16(ds, uint16_t(360)),16,Op::SUB); // 2c09: cmp ax, word ptr [0x168]
step(); if(cf){goto L2c11;} // 2c0d: jb 0x2c11
step(); goto L2c74; // 2c0f: jmp 0x2c74
L2c11: step(); alu(read16(ss, uint16_t(8+bp)),127,16,Op::SUB); // 2c11: cmp word ptr [bp + 8], 0x7f
step(); if(cf||zf){goto L2c1c;} // 2c15: jbe 0x2c1c
step(); write16(ss, uint16_t(8+bp), 127); // 2c17: mov word ptr [bp + 8], 0x7f
L2c1c: step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2c1c: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ss, uint16_t(8+bp))); // 2c1f: mov al, byte ptr [bp + 8]
step(); write8(ds, uint16_t(346+bx), lo(ax)); // 2c22: mov byte ptr [bx + 0x15a], al
step(); alu(read8(ds, uint16_t(5424)),0,8,Op::SUB); // 2c26: cmp byte ptr [0x1530], 0
step(); if(zf){goto L2c3d;} // 2c2b: je 0x2c3d
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2c2d: mov ax, word ptr [bp + 6]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 2c30: shl ax, 1
step(); ax=uint16_t(alu(ax,18,16,Op::ADD)); // 2c32: add ax, 0x12
step(); write16(ss, uint16_t(-2+bp), ds); // 2c35: mov word ptr [bp - 2], ds
step(); write16(ss, uint16_t(-4+bp), ax); // 2c38: mov word ptr [bp - 4], ax
step(); goto L2c4b; // 2c3b: jmp 0x2c4b
L2c3d: step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2c3d: mov ax, word ptr [bp + 6]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 2c40: shl ax, 1
step(); ax=uint16_t(alu(ax,0,16,Op::ADD)); // 2c42: add ax, 0
step(); write16(ss, uint16_t(-2+bp), ds); // 2c45: mov word ptr [bp - 2], ds
step(); write16(ss, uint16_t(-4+bp), ax); // 2c48: mov word ptr [bp - 4], ax
L2c4b: step(); {const auto location=uint16_t(-4+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2c4b: les bx, ptr [bp - 4]
step(); set_lo(ax,read8(es, uint16_t(0+bx))); // 2c4e: mov al, byte ptr es:[bx]
step(); set_hi(ax,0); // 2c51: mov ah, 0
step(); push(ax); // 2c53: push ax
step(); push(cs); // 2c54: push cs
step(); push(0x338); execute(0x307d); // 2c55: call 0x307d
step(); ; // 2c58: nop
step(); cx=uint16_t(pop()); // 2c59: pop cx
step(); {const auto location=uint16_t(-4+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2c5a: les bx, ptr [bp - 4]
step(); alu(read8(es, uint16_t(1+bx)),255,8,Op::SUB); // 2c5d: cmp byte ptr es:[bx + 1], 0xff
step(); if(zf){goto L2c74;} // 2c62: je 0x2c74
step(); {const auto location=uint16_t(-4+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2c64: les bx, ptr [bp - 4]
step(); set_lo(ax,read8(es, uint16_t(1+bx))); // 2c67: mov al, byte ptr es:[bx + 1]
step(); set_hi(ax,0); // 2c6b: mov ah, 0
step(); push(ax); // 2c6d: push ax
step(); push(cs); // 2c6e: push cs
step(); push(0x352); execute(0x307d); // 2c6f: call 0x307d
step(); ; // 2c72: nop
step(); cx=uint16_t(pop()); // 2c73: pop cx
L2c74: step(); ds=uint16_t(pop()); // 2c74: pop ds
step(); sp=uint16_t(bp); // 2c75: mov sp, bp
step(); bp=uint16_t(pop()); // 2c77: pop bp
step(); pop();cs=pop();return; // 2c78: retf
L2c79: step(); push(bp); // 2c79: push bp
step(); bp=uint16_t(sp); // 2c7a: mov bp, sp
step(); push(ds); // 2c7c: push ds
step(); ax=uint16_t(5249); // 2c7d: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2c80: mov ds, ax
step(); set_lo(ax,read8(ds, uint16_t(5424))); // 2c82: mov al, byte ptr [0x1530]
step(); set_hi(ax,0); // 2c85: mov ah, 0
step(); ax=uint16_t(alu(ax,ax,16,Op::OR)); // 2c87: or ax, ax
step(); if(!zf){goto L2c91;} // 2c89: jne 0x2c91
step(); alu(read16(ss, uint16_t(6+bp)),9,16,Op::SUB); // 2c8b: cmp word ptr [bp + 6], 9
step(); if(cf){goto L2c97;} // 2c8f: jb 0x2c97
L2c91: step(); alu(read16(ss, uint16_t(6+bp)),6,16,Op::SUB); // 2c91: cmp word ptr [bp + 6], 6
step(); if(!cf&&!zf){goto L2cb8;} // 2c95: ja 0x2cb8
L2c97: step(); alu(read16(ss, uint16_t(8+bp)),16383,16,Op::SUB); // 2c97: cmp word ptr [bp + 8], 0x3fff
step(); if(cf||zf){goto L2ca3;} // 2c9c: jbe 0x2ca3
step(); write16(ss, uint16_t(8+bp), 16383); // 2c9e: mov word ptr [bp + 8], 0x3fff
L2ca3: step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2ca3: mov bx, word ptr [bp + 6]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 2ca6: shl bx, 1
step(); ax=uint16_t(read16(ss, uint16_t(8+bp))); // 2ca8: mov ax, word ptr [bp + 8]
step(); write16(ds, uint16_t(472+bx), ax); // 2cab: mov word ptr [bx + 0x1d8], ax
step(); push(read16(ss, uint16_t(6+bp))); // 2caf: push word ptr [bp + 6]
step(); push(cs); // 2cb2: push cs
step(); push(0x396); execute(0x33c8); // 2cb3: call 0x33c8
step(); ; // 2cb6: nop
step(); cx=uint16_t(pop()); // 2cb7: pop cx
L2cb8: step(); ds=uint16_t(pop()); // 2cb8: pop ds
step(); bp=uint16_t(pop()); // 2cb9: pop bp
step(); pop();cs=pop();return; // 2cba: retf
L2cbb: step(); push(bp); // 2cbb: push bp
step(); bp=uint16_t(sp); // 2cbc: mov bp, sp
step(); push(ds); // 2cbe: push ds
step(); ax=uint16_t(5249); // 2cbf: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2cc2: mov ds, ax
step(); write16(ss, uint16_t(8+bp), alu(read16(ss, uint16_t(8+bp)),12,16,Op::SUB)); // 2cc4: sub word ptr [bp + 8], 0xc
step(); alu(read16(ss, uint16_t(8+bp)),0,16,Op::SUB); // 2cc8: cmp word ptr [bp + 8], 0
step(); if(sf==of){goto L2cd3;} // 2ccc: jge 0x2cd3
step(); write16(ss, uint16_t(8+bp), 0); // 2cce: mov word ptr [bp + 8], 0
L2cd3: step(); set_lo(ax,read8(ds, uint16_t(5424))); // 2cd3: mov al, byte ptr [0x1530]
step(); set_hi(ax,0); // 2cd6: mov ah, 0
step(); ax=uint16_t(alu(ax,ax,16,Op::OR)); // 2cd8: or ax, ax
step(); if(!zf){goto L2ce2;} // 2cda: jne 0x2ce2
step(); alu(read16(ss, uint16_t(6+bp)),9,16,Op::SUB); // 2cdc: cmp word ptr [bp + 6], 9
step(); if(cf){goto L2ce8;} // 2ce0: jb 0x2ce8
L2ce2: step(); alu(read16(ss, uint16_t(6+bp)),6,16,Op::SUB); // 2ce2: cmp word ptr [bp + 6], 6
step(); if(!cf){goto L2d05;} // 2ce6: jae 0x2d05
L2ce8: step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2ce8: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ss, uint16_t(8+bp))); // 2ceb: mov al, byte ptr [bp + 8]
step(); write8(ds, uint16_t(368+bx), lo(ax)); // 2cee: mov byte ptr [bx + 0x170], al
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2cf2: mov bx, word ptr [bp + 6]
step(); write8(ds, uint16_t(337+bx), 32); // 2cf5: mov byte ptr [bx + 0x151], 0x20
step(); push(read16(ss, uint16_t(6+bp))); // 2cfa: push word ptr [bp + 6]
step(); push(cs); // 2cfd: push cs
step(); push(0x3e1); execute(0x33c8); // 2cfe: call 0x33c8
step(); ; // 2d01: nop
step(); cx=uint16_t(pop()); // 2d02: pop cx
step(); goto L2d72; // 2d03: jmp 0x2d72
L2d05: step(); alu(read8(ds, uint16_t(5424)),0,8,Op::SUB); // 2d05: cmp byte ptr [0x1530], 0
step(); if(zf){goto L2d72;} // 2d0a: je 0x2d72
step(); alu(read16(ss, uint16_t(6+bp)),10,16,Op::SUB); // 2d0c: cmp word ptr [bp + 6], 0xa
step(); if(!cf&&!zf){goto L2d72;} // 2d10: ja 0x2d72
step(); alu(read16(ss, uint16_t(6+bp)),6,16,Op::SUB); // 2d12: cmp word ptr [bp + 6], 6
step(); if(!zf){goto L2d29;} // 2d16: jne 0x2d29
step(); set_lo(ax,read8(ss, uint16_t(8+bp))); // 2d18: mov al, byte ptr [bp + 8]
step(); write8(ds, uint16_t(374), lo(ax)); // 2d1b: mov byte ptr [0x176], al
step(); push(read16(ss, uint16_t(6+bp))); // 2d1e: push word ptr [bp + 6]
step(); push(cs); // 2d21: push cs
step(); push(0x405); execute(0x33c8); // 2d22: call 0x33c8
step(); ; // 2d25: nop
step(); cx=uint16_t(pop()); // 2d26: pop cx
step(); goto L2d4f; // 2d27: jmp 0x2d4f
L2d29: step(); alu(read16(ss, uint16_t(6+bp)),8,16,Op::SUB); // 2d29: cmp word ptr [bp + 6], 8
step(); if(zf){goto L2d35;} // 2d2d: je 0x2d35
step(); alu(read16(ss, uint16_t(6+bp)),7,16,Op::SUB); // 2d2f: cmp word ptr [bp + 6], 7
step(); if(!zf){goto L2d4f;} // 2d33: jne 0x2d4f
L2d35: step(); set_lo(ax,read8(ds, uint16_t(376))); // 2d35: mov al, byte ptr [0x178]
step(); set_hi(ax,0); // 2d38: mov ah, 0
step(); alu(ax,read16(ss, uint16_t(8+bp)),16,Op::SUB); // 2d3a: cmp ax, word ptr [bp + 8]
step(); if(zf){goto L2d4f;} // 2d3d: je 0x2d4f
step(); set_lo(ax,read8(ss, uint16_t(8+bp))); // 2d3f: mov al, byte ptr [bp + 8]
step(); write8(ds, uint16_t(376), lo(ax)); // 2d42: mov byte ptr [0x178], al
step(); ax=uint16_t(8); // 2d45: mov ax, 8
step(); push(ax); // 2d48: push ax
step(); push(cs); // 2d49: push cs
step(); push(0x42d); execute(0x33c8); // 2d4a: call 0x33c8
step(); ; // 2d4d: nop
step(); cx=uint16_t(pop()); // 2d4e: pop cx
L2d4f: step(); set_lo(ax,read8(ss, uint16_t(8+bp))); // 2d4f: mov al, byte ptr [bp + 8]
step(); write8(ds, uint16_t(375), lo(ax)); // 2d52: mov byte ptr [0x177], al
step(); ax=uint16_t(7); // 2d55: mov ax, 7
step(); push(ax); // 2d58: push ax
step(); push(cs); // 2d59: push cs
step(); push(0x43d); execute(0x33c8); // 2d5a: call 0x33c8
step(); ; // 2d5d: nop
step(); cx=uint16_t(pop()); // 2d5e: pop cx
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2d5f: mov bx, word ptr [bp + 6]
step(); bx=uint16_t(alu(bx,6,16,Op::SUB)); // 2d62: sub bx, 6
step(); set_lo(ax,read8(ds, uint16_t(5425+bx))); // 2d65: mov al, byte ptr [bx + 0x1531]
step(); write8(ds, uint16_t(4910), alu(read8(ds, uint16_t(4910)),lo(ax),8,Op::OR)); // 2d69: or byte ptr [0x132e], al
step(); push(cs); // 2d6d: push cs
step(); push(0x451); execute(0x3326); // 2d6e: call 0x3326
step(); ; // 2d71: nop
L2d72: step(); ds=uint16_t(pop()); // 2d72: pop ds
step(); bp=uint16_t(pop()); // 2d73: pop bp
step(); pop();cs=pop();return; // 2d74: retf
L2d75: step(); push(bp); // 2d75: push bp
step(); bp=uint16_t(sp); // 2d76: mov bp, sp
step(); push(ds); // 2d78: push ds
step(); ax=uint16_t(5249); // 2d79: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2d7c: mov ds, ax
step(); set_lo(ax,read8(ds, uint16_t(5424))); // 2d7e: mov al, byte ptr [0x1530]
step(); set_hi(ax,0); // 2d81: mov ah, 0
step(); ax=uint16_t(alu(ax,ax,16,Op::OR)); // 2d83: or ax, ax
step(); if(!zf){goto L2d8d;} // 2d85: jne 0x2d8d
step(); alu(read16(ss, uint16_t(6+bp)),9,16,Op::SUB); // 2d87: cmp word ptr [bp + 6], 9
step(); if(cf){goto L2d93;} // 2d8b: jb 0x2d93
L2d8d: step(); alu(read16(ss, uint16_t(6+bp)),6,16,Op::SUB); // 2d8d: cmp word ptr [bp + 6], 6
step(); if(!cf){goto L2dbc;} // 2d91: jae 0x2dbc
L2d93: step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2d93: mov bx, word ptr [bp + 6]
step(); write8(ds, uint16_t(337+bx), 0); // 2d96: mov byte ptr [bx + 0x151], 0
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2d9b: mov bx, word ptr [bp + 6]
step(); write8(ds, uint16_t(404+bx), alu(read8(ds, uint16_t(404+bx)),223,8,Op::AND)); // 2d9e: and byte ptr [bx + 0x194], 0xdf
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2da3: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(404+bx))); // 2da6: mov al, byte ptr [bx + 0x194]
step(); set_hi(ax,0); // 2daa: mov ah, 0
step(); push(ax); // 2dac: push ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2dad: mov ax, word ptr [bp + 6]
step(); ax=uint16_t(alu(ax,176,16,Op::ADD)); // 2db0: add ax, 0xb0
step(); push(ax); // 2db3: push ax
step(); push(cs); // 2db4: push cs
step(); push(0x498); adapter(0x2928); // 2db5: call 0x2928
step(); cx=uint16_t(pop()); // 2db8: pop cx
step(); cx=uint16_t(pop()); // 2db9: pop cx
step(); goto L2dde; // 2dba: jmp 0x2dde
L2dbc: step(); alu(read8(ds, uint16_t(5424)),0,8,Op::SUB); // 2dbc: cmp byte ptr [0x1530], 0
step(); if(zf){goto L2dde;} // 2dc1: je 0x2dde
step(); alu(read16(ss, uint16_t(6+bp)),10,16,Op::SUB); // 2dc3: cmp word ptr [bp + 6], 0xa
step(); if(!cf&&!zf){goto L2dde;} // 2dc7: ja 0x2dde
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 2dc9: mov bx, word ptr [bp + 6]
step(); bx=uint16_t(alu(bx,6,16,Op::SUB)); // 2dcc: sub bx, 6
step(); set_lo(ax,read8(ds, uint16_t(5425+bx))); // 2dcf: mov al, byte ptr [bx + 0x1531]
step(); set_lo(ax,~(lo(ax))); // 2dd3: not al
step(); write8(ds, uint16_t(4910), alu(read8(ds, uint16_t(4910)),lo(ax),8,Op::AND)); // 2dd5: and byte ptr [0x132e], al
step(); push(cs); // 2dd9: push cs
step(); push(0x4bd); execute(0x3326); // 2dda: call 0x3326
step(); ; // 2ddd: nop
L2dde: step(); ds=uint16_t(pop()); // 2dde: pop ds
step(); bp=uint16_t(pop()); // 2ddf: pop bp
step(); pop();cs=pop();return; // 2de0: retf
L2de1: step(); push(bp); // 2de1: push bp
step(); bp=uint16_t(sp); // 2de2: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 2de4: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 2de5: dec sp
step(); push(ds); // 2de6: push ds
step(); ax=uint16_t(5249); // 2de7: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2dea: mov ds, ax
step(); write16(ss, uint16_t(-2+bp), 0); // 2dec: mov word ptr [bp - 2], 0
step(); goto L2e2e; // 2df1: jmp 0x2e2e
L2df3: step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 2df3: mov bx, word ptr [bp - 2]
step(); alu(read8(ds, uint16_t(5560+bx)),0,8,Op::SUB); // 2df6: cmp byte ptr [bx + 0x15b8], 0
step(); if(zf){goto L2e15;} // 2dfb: je 0x2e15
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2dfd: xor ax, ax
step(); push(ax); // 2dff: push ax
step(); ax=uint16_t(5249); // 2e00: mov ax, 0x1481
step(); push(ax); // 2e03: push ax
step(); ax=uint16_t(5444); // 2e04: mov ax, 0x1544
step(); push(ax); // 2e07: push ax
step(); push(read16(ss, uint16_t(-2+bp))); // 2e08: push word ptr [bp - 2]
step(); push(cs); // 2e0b: push cs
step(); push(0x4ef); execute(0x2f55); // 2e0c: call 0x2f55
step(); ; // 2e0f: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2e10: add sp, 8
step(); goto L2e2b; // 2e13: jmp 0x2e2b
L2e15: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2e15: xor ax, ax
step(); push(ax); // 2e17: push ax
step(); ax=uint16_t(5249); // 2e18: mov ax, 0x1481
step(); push(ax); // 2e1b: push ax
step(); ax=uint16_t(5430); // 2e1c: mov ax, 0x1536
step(); push(ax); // 2e1f: push ax
step(); push(read16(ss, uint16_t(-2+bp))); // 2e20: push word ptr [bp - 2]
step(); push(cs); // 2e23: push cs
step(); push(0x507); execute(0x2f55); // 2e24: call 0x2f55
step(); ; // 2e27: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2e28: add sp, 8
L2e2b: step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 2e2b: inc word ptr [bp - 2]
L2e2e: step(); alu(read16(ss, uint16_t(-2+bp)),18,16,Op::SUB); // 2e2e: cmp word ptr [bp - 2], 0x12
step(); if(sf!=of){goto L2df3;} // 2e32: jl 0x2df3
step(); alu(read8(ds, uint16_t(5424)),0,8,Op::SUB); // 2e34: cmp byte ptr [0x1530], 0
step(); if(!zf){goto L2e3e;} // 2e39: jne 0x2e3e
step(); goto L2ec8; // 2e3b: jmp 0x2ec8
L2e3e: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2e3e: xor ax, ax
step(); push(ax); // 2e40: push ax
step(); ax=uint16_t(5249); // 2e41: mov ax, 0x1481
step(); push(ax); // 2e44: push ax
step(); ax=uint16_t(5458); // 2e45: mov ax, 0x1552
step(); push(ax); // 2e48: push ax
step(); ax=uint16_t(12); // 2e49: mov ax, 0xc
step(); push(ax); // 2e4c: push ax
step(); push(cs); // 2e4d: push cs
step(); push(0x531); execute(0x2f55); // 2e4e: call 0x2f55
step(); ; // 2e51: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2e52: add sp, 8
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2e55: xor ax, ax
step(); push(ax); // 2e57: push ax
step(); ax=uint16_t(5249); // 2e58: mov ax, 0x1481
step(); push(ax); // 2e5b: push ax
step(); ax=uint16_t(5472); // 2e5c: mov ax, 0x1560
step(); push(ax); // 2e5f: push ax
step(); ax=uint16_t(15); // 2e60: mov ax, 0xf
step(); push(ax); // 2e63: push ax
step(); push(cs); // 2e64: push cs
step(); push(0x548); execute(0x2f55); // 2e65: call 0x2f55
step(); ; // 2e68: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2e69: add sp, 8
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2e6c: xor ax, ax
step(); push(ax); // 2e6e: push ax
step(); ax=uint16_t(5249); // 2e6f: mov ax, 0x1481
step(); push(ax); // 2e72: push ax
step(); ax=uint16_t(5486); // 2e73: mov ax, 0x156e
step(); push(ax); // 2e76: push ax
step(); ax=uint16_t(16); // 2e77: mov ax, 0x10
step(); push(ax); // 2e7a: push ax
step(); push(cs); // 2e7b: push cs
step(); push(0x55f); execute(0x2f55); // 2e7c: call 0x2f55
step(); ; // 2e7f: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2e80: add sp, 8
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2e83: xor ax, ax
step(); push(ax); // 2e85: push ax
step(); ax=uint16_t(5249); // 2e86: mov ax, 0x1481
step(); push(ax); // 2e89: push ax
step(); ax=uint16_t(5500); // 2e8a: mov ax, 0x157c
step(); push(ax); // 2e8d: push ax
step(); ax=uint16_t(14); // 2e8e: mov ax, 0xe
step(); push(ax); // 2e91: push ax
step(); push(cs); // 2e92: push cs
step(); push(0x576); execute(0x2f55); // 2e93: call 0x2f55
step(); ; // 2e96: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2e97: add sp, 8
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2e9a: xor ax, ax
step(); push(ax); // 2e9c: push ax
step(); ax=uint16_t(5249); // 2e9d: mov ax, 0x1481
step(); push(ax); // 2ea0: push ax
step(); ax=uint16_t(5514); // 2ea1: mov ax, 0x158a
step(); push(ax); // 2ea4: push ax
step(); ax=uint16_t(17); // 2ea5: mov ax, 0x11
step(); push(ax); // 2ea8: push ax
step(); push(cs); // 2ea9: push cs
step(); push(0x58d); execute(0x2f55); // 2eaa: call 0x2f55
step(); ; // 2ead: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2eae: add sp, 8
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 2eb1: xor ax, ax
step(); push(ax); // 2eb3: push ax
step(); ax=uint16_t(5249); // 2eb4: mov ax, 0x1481
step(); push(ax); // 2eb7: push ax
step(); ax=uint16_t(5528); // 2eb8: mov ax, 0x1598
step(); push(ax); // 2ebb: push ax
step(); ax=uint16_t(13); // 2ebc: mov ax, 0xd
step(); push(ax); // 2ebf: push ax
step(); push(cs); // 2ec0: push cs
step(); push(0x5a4); execute(0x2f55); // 2ec1: call 0x2f55
step(); ; // 2ec4: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2ec5: add sp, 8
L2ec8: step(); ds=uint16_t(pop()); // 2ec8: pop ds
step(); sp=uint16_t(bp); // 2ec9: mov sp, bp
step(); bp=uint16_t(pop()); // 2ecb: pop bp
step(); pop();cs=pop();return; // 2ecc: retf
L2efa: step(); push(bp); // 2efa: push bp
step(); bp=uint16_t(sp); // 2efb: mov bp, sp
step(); sp=uint16_t(alu(sp,6,16,Op::SUB)); // 2efd: sub sp, 6
step(); push(ds); // 2f00: push ds
step(); ax=uint16_t(5249); // 2f01: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2f04: mov ds, ax
step(); write16(ss, uint16_t(-2+bp), 0); // 2f06: mov word ptr [bp - 2], 0
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 2f0b: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 2f0e: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 2f11: imul dx
step(); ax=uint16_t(alu(ax,4914,16,Op::ADD)); // 2f13: add ax, 0x1332
step(); write16(ss, uint16_t(-4+bp), ds); // 2f16: mov word ptr [bp - 4], ds
step(); write16(ss, uint16_t(-6+bp), ax); // 2f19: mov word ptr [bp - 6], ax
step(); goto L2f34; // 2f1c: jmp 0x2f34
L2f1e: step(); {const auto location=uint16_t(8+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2f1e: les bx, ptr [bp + 8]
step(); set_lo(ax,read8(es, uint16_t(0+bx))); // 2f21: mov al, byte ptr es:[bx]
step(); {const auto location=uint16_t(-6+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2f24: les bx, ptr [bp - 6]
step(); write8(es, uint16_t(0+bx), lo(ax)); // 2f27: mov byte ptr es:[bx], al
step(); write16(ss, uint16_t(8+bp), alu(read16(ss, uint16_t(8+bp)),2,16,Op::ADD)); // 2f2a: add word ptr [bp + 8], 2
step(); write16(ss, uint16_t(-6+bp), incdec(read16(ss, uint16_t(-6+bp)),16,false)); // 2f2e: inc word ptr [bp - 6]
step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 2f31: inc word ptr [bp - 2]
L2f34: step(); alu(read16(ss, uint16_t(-2+bp)),13,16,Op::SUB); // 2f34: cmp word ptr [bp - 2], 0xd
step(); if(sf!=of){goto L2f1e;} // 2f38: jl 0x2f1e
step(); write16(ss, uint16_t(12+bp), alu(read16(ss, uint16_t(12+bp)),3,16,Op::AND)); // 2f3a: and word ptr [bp + 0xc], 3
step(); ax=uint16_t(read16(ss, uint16_t(12+bp))); // 2f3e: mov ax, word ptr [bp + 0xc]
step(); {const auto location=uint16_t(-6+bp); const auto source_segment=ss; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2f41: les bx, ptr [bp - 6]
step(); write8(es, uint16_t(0+bx), lo(ax)); // 2f44: mov byte ptr es:[bx], al
step(); push(read16(ss, uint16_t(6+bp))); // 2f47: push word ptr [bp + 6]
step(); push(cs); // 2f4a: push cs
step(); push(0x62e); execute(0x3031); // 2f4b: call 0x3031
step(); ; // 2f4e: nop
step(); cx=uint16_t(pop()); // 2f4f: pop cx
step(); ds=uint16_t(pop()); // 2f50: pop ds
step(); sp=uint16_t(bp); // 2f51: mov sp, bp
step(); bp=uint16_t(pop()); // 2f53: pop bp
step(); pop();cs=pop();return; // 2f54: retf
L2f55: step(); push(bp); // 2f55: push bp
step(); bp=uint16_t(sp); // 2f56: mov bp, sp
step(); sp=uint16_t(alu(sp,30,16,Op::SUB)); // 2f58: sub sp, 0x1e
step(); push(si); // 2f5b: push si
step(); push(ds); // 2f5c: push ds
step(); ax=uint16_t(5249); // 2f5d: mov ax, 0x1481
step(); ds=uint16_t(ax); // 2f60: mov ds, ax
step(); write16(ss, uint16_t(-2+bp), 0); // 2f62: mov word ptr [bp - 2], 0
step(); goto L2f84; // 2f67: jmp 0x2f84
L2f69: step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 2f69: mov bx, word ptr [bp - 2]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 2f6c: shl bx, 1
step(); ax=uint16_t(uint16_t(-30+bp)); // 2f6e: lea ax, [bp - 0x1e]
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 2f71: add bx, ax
step(); {const auto location=uint16_t(8+bp); const auto source_segment=ss; si=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 2f73: les si, ptr [bp + 8]
step(); set_lo(ax,read8(es, uint16_t(0+si))); // 2f76: mov al, byte ptr es:[si]
step(); set_hi(ax,0); // 2f79: mov ah, 0
step(); write16(ss, uint16_t(0+bx), ax); // 2f7b: mov word ptr ss:[bx], ax
step(); write16(ss, uint16_t(8+bp), incdec(read16(ss, uint16_t(8+bp)),16,false)); // 2f7e: inc word ptr [bp + 8]
step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 2f81: inc word ptr [bp - 2]
L2f84: step(); alu(read16(ss, uint16_t(-2+bp)),13,16,Op::SUB); // 2f84: cmp word ptr [bp - 2], 0xd
step(); if(sf!=of){goto L2f69;} // 2f88: jl 0x2f69
step(); push(read16(ss, uint16_t(12+bp))); // 2f8a: push word ptr [bp + 0xc]
step(); push(ss); // 2f8d: push ss
step(); ax=uint16_t(uint16_t(-30+bp)); // 2f8e: lea ax, [bp - 0x1e]
step(); push(ax); // 2f91: push ax
step(); push(read16(ss, uint16_t(6+bp))); // 2f92: push word ptr [bp + 6]
step(); push(cs); // 2f95: push cs
step(); push(0x679); execute(0x2efa); // 2f96: call 0x2efa
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 2f99: add sp, 8
step(); ds=uint16_t(pop()); // 2f9c: pop ds
step(); si=uint16_t(pop()); // 2f9d: pop si
step(); sp=uint16_t(bp); // 2f9e: mov sp, bp
step(); bp=uint16_t(pop()); // 2fa0: pop bp
step(); pop();cs=pop();return; // 2fa1: retf
L3031: step(); push(bp); // 3031: push bp
step(); bp=uint16_t(sp); // 3032: mov bp, sp
step(); push(ds); // 3034: push ds
step(); ax=uint16_t(5249); // 3035: mov ax, 0x1481
step(); ds=uint16_t(ax); // 3038: mov ds, ax
step(); push(cs); // 303a: push cs
step(); push(0x71e); execute(0x3326); // 303b: call 0x3326
step(); ; // 303e: nop
step(); push(cs); // 303f: push cs
step(); push(0x723); execute(0x3151); // 3040: call 0x3151
step(); ; // 3043: nop
step(); push(read16(ss, uint16_t(6+bp))); // 3044: push word ptr [bp + 6]
step(); push(cs); // 3047: push cs
step(); push(0x72b); execute(0x307d); // 3048: call 0x307d
step(); ; // 304b: nop
step(); cx=uint16_t(pop()); // 304c: pop cx
step(); push(read16(ss, uint16_t(6+bp))); // 304d: push word ptr [bp + 6]
step(); push(cs); // 3050: push cs
step(); push(0x734); execute(0x3176); // 3051: call 0x3176
step(); ; // 3054: nop
step(); cx=uint16_t(pop()); // 3055: pop cx
step(); push(read16(ss, uint16_t(6+bp))); // 3056: push word ptr [bp + 6]
step(); push(cs); // 3059: push cs
step(); push(0x73d); execute(0x31d8); // 305a: call 0x31d8
step(); ; // 305d: nop
step(); cx=uint16_t(pop()); // 305e: pop cx
step(); push(read16(ss, uint16_t(6+bp))); // 305f: push word ptr [bp + 6]
step(); push(cs); // 3062: push cs
step(); push(0x746); execute(0x322b); // 3063: call 0x322b
step(); ; // 3066: nop
step(); cx=uint16_t(pop()); // 3067: pop cx
step(); push(read16(ss, uint16_t(6+bp))); // 3068: push word ptr [bp + 6]
step(); push(cs); // 306b: push cs
step(); push(0x74f); execute(0x327e); // 306c: call 0x327e
step(); ; // 306f: nop
step(); cx=uint16_t(pop()); // 3070: pop cx
step(); push(read16(ss, uint16_t(6+bp))); // 3071: push word ptr [bp + 6]
step(); push(cs); // 3074: push cs
step(); push(0x758); execute(0x337e); // 3075: call 0x337e
step(); ; // 3078: nop
step(); cx=uint16_t(pop()); // 3079: pop cx
step(); ds=uint16_t(pop()); // 307a: pop ds
step(); bp=uint16_t(pop()); // 307b: pop bp
step(); pop();cs=pop();return; // 307c: retf
L307d: step(); push(bp); // 307d: push bp
step(); bp=uint16_t(sp); // 307e: mov bp, sp
step(); sp=uint16_t(alu(sp,6,16,Op::SUB)); // 3080: sub sp, 6
step(); push(ds); // 3083: push ds
step(); ax=uint16_t(5249); // 3084: mov ax, 0x1481
step(); ds=uint16_t(ax); // 3087: mov ds, ax
step(); alu(read8(ds, uint16_t(5424)),0,8,Op::SUB); // 3089: cmp byte ptr [0x1530], 0
step(); if(zf){goto L309e;} // 308e: je 0x309e
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 3090: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(5596+bx))); // 3093: mov al, byte ptr [bx + 0x15dc]
step(); set_hi(ax,0); // 3097: mov ah, 0
step(); write16(ss, uint16_t(-4+bp), ax); // 3099: mov word ptr [bp - 4], ax
step(); goto L30aa; // 309c: jmp 0x30aa
L309e: step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 309e: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(5578+bx))); // 30a1: mov al, byte ptr [bx + 0x15ca]
step(); set_hi(ax,0); // 30a5: mov ah, 0
step(); write16(ss, uint16_t(-4+bp), ax); // 30a7: mov word ptr [bp - 4], ax
L30aa: step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 30aa: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 30ad: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 30b0: imul dx
step(); bx=uint16_t(ax); // 30b2: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4922+bx))); // 30b4: mov al, byte ptr [bx + 0x133a]
step(); set_hi(ax,0); // 30b8: mov ah, 0
step(); ax=uint16_t(alu(ax,63,16,Op::AND)); // 30ba: and ax, 0x3f
step(); dx=uint16_t(63); // 30bd: mov dx, 0x3f
step(); dx=uint16_t(alu(dx,ax,16,Op::SUB)); // 30c0: sub dx, ax
step(); write16(ss, uint16_t(-2+bp), dx); // 30c2: mov word ptr [bp - 2], dx
step(); alu(read8(ds, uint16_t(5424)),0,8,Op::SUB); // 30c5: cmp byte ptr [0x1530], 0
step(); if(zf){goto L30d7;} // 30ca: je 0x30d7
step(); alu(read16(ss, uint16_t(-4+bp)),6,16,Op::SUB); // 30cc: cmp word ptr [bp - 4], 6
step(); if(cf||zf){goto L30d7;} // 30d0: jbe 0x30d7
step(); ax=uint16_t(1); // 30d2: mov ax, 1
step(); goto L30d9; // 30d5: jmp 0x30d9
L30d7: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 30d7: xor ax, ax
L30d9: step(); write16(ss, uint16_t(-6+bp), ax); // 30d9: mov word ptr [bp - 6], ax
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 30dc: mov bx, word ptr [bp + 6]
step(); alu(read8(ds, uint16_t(5560+bx)),0,8,Op::SUB); // 30df: cmp byte ptr [bx + 0x15b8], 0
step(); if(!zf){goto L3100;} // 30e4: jne 0x3100
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 30e6: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 30e9: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 30ec: imul dx
step(); bx=uint16_t(ax); // 30ee: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4926+bx))); // 30f0: mov al, byte ptr [bx + 0x133e]
step(); set_hi(ax,0); // 30f4: mov ah, 0
step(); ax=uint16_t(alu(ax,ax,16,Op::OR)); // 30f6: or ax, ax
step(); if(zf){goto L3100;} // 30f8: je 0x3100
step(); alu(read16(ss, uint16_t(-6+bp)),0,16,Op::SUB); // 30fa: cmp word ptr [bp - 6], 0
step(); if(zf){goto L3116;} // 30fe: je 0x3116
L3100: step(); bx=uint16_t(read16(ss, uint16_t(-4+bp))); // 3100: mov bx, word ptr [bp - 4]
step(); set_lo(ax,read8(ds, uint16_t(346+bx))); // 3103: mov al, byte ptr [bx + 0x15a]
step(); set_hi(ax,0); // 3107: mov ah, 0
step(); math(read16(ss, uint16_t(-2+bp)), 16, Math::IMUL); // 3109: imul word ptr [bp - 2]
step(); ax=uint16_t(alu(ax,64,16,Op::ADD)); // 310c: add ax, 0x40
step(); set_lo(cx,7); // 310f: mov cl, 7
step(); ax=uint16_t(shift(ax,16,Op::SHR,lo(cx))); // 3111: shr ax, cl
step(); write16(ss, uint16_t(-2+bp), ax); // 3113: mov word ptr [bp - 2], ax
L3116: step(); ax=uint16_t(63); // 3116: mov ax, 0x3f
step(); ax=uint16_t(alu(ax,read16(ss, uint16_t(-2+bp)),16,Op::SUB)); // 3119: sub ax, word ptr [bp - 2]
step(); write16(ss, uint16_t(-2+bp), ax); // 311c: mov word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 311f: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 3122: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 3125: imul dx
step(); bx=uint16_t(ax); // 3127: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4914+bx))); // 3129: mov al, byte ptr [bx + 0x1332]
step(); set_hi(ax,0); // 312d: mov ah, 0
step(); set_lo(cx,6); // 312f: mov cl, 6
step(); ax=uint16_t(shift(ax,16,Op::SHL,lo(cx))); // 3131: shl ax, cl
step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::OR)); // 3133: or word ptr [bp - 2], ax
step(); push(read16(ss, uint16_t(-2+bp))); // 3136: push word ptr [bp - 2]
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 3139: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(5542+bx))); // 313c: mov al, byte ptr [bx + 0x15a6]
step(); set_hi(ax,0); // 3140: mov ah, 0
step(); ax=uint16_t(alu(ax,64,16,Op::ADD)); // 3142: add ax, 0x40
step(); push(ax); // 3145: push ax
step(); push(cs); // 3146: push cs
step(); push(0x82a); adapter(0x2928); // 3147: call 0x2928
step(); cx=uint16_t(pop()); // 314a: pop cx
step(); cx=uint16_t(pop()); // 314b: pop cx
step(); ds=uint16_t(pop()); // 314c: pop ds
step(); sp=uint16_t(bp); // 314d: mov sp, bp
step(); bp=uint16_t(pop()); // 314f: pop bp
step(); pop();cs=pop();return; // 3150: retf
L3151: step(); push(bp); // 3151: push bp
step(); bp=uint16_t(sp); // 3152: mov bp, sp
step(); push(ds); // 3154: push ds
step(); ax=uint16_t(5249); // 3155: mov ax, 0x1481
step(); ds=uint16_t(ax); // 3158: mov ds, ax
step(); alu(read8(ds, uint16_t(336)),0,8,Op::SUB); // 315a: cmp byte ptr [0x150], 0
step(); if(zf){goto L3166;} // 315f: je 0x3166
step(); ax=uint16_t(64); // 3161: mov ax, 0x40
step(); goto L3168; // 3164: jmp 0x3168
L3166: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 3166: xor ax, ax
L3168: step(); push(ax); // 3168: push ax
step(); ax=uint16_t(8); // 3169: mov ax, 8
step(); push(ax); // 316c: push ax
step(); push(cs); // 316d: push cs
step(); push(0x851); adapter(0x2928); // 316e: call 0x2928
step(); cx=uint16_t(pop()); // 3171: pop cx
step(); cx=uint16_t(pop()); // 3172: pop cx
step(); ds=uint16_t(pop()); // 3173: pop ds
step(); bp=uint16_t(pop()); // 3174: pop bp
step(); pop();cs=pop();return; // 3175: retf
L3176: step(); push(bp); // 3176: push bp
step(); bp=uint16_t(sp); // 3177: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 3179: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 317a: dec sp
step(); push(ds); // 317b: push ds
step(); ax=uint16_t(5249); // 317c: mov ax, 0x1481
step(); ds=uint16_t(ax); // 317f: mov ds, ax
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 3181: mov bx, word ptr [bp + 6]
step(); alu(read8(ds, uint16_t(5560+bx)),0,8,Op::SUB); // 3184: cmp byte ptr [bx + 0x15b8], 0
step(); if(zf){goto L318d;} // 3189: je 0x318d
step(); goto L31d3; // 318b: jmp 0x31d3
L318d: step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 318d: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 3190: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 3193: imul dx
step(); bx=uint16_t(ax); // 3195: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4916+bx))); // 3197: mov al, byte ptr [bx + 0x1334]
step(); set_hi(ax,0); // 319b: mov ah, 0
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 319d: shl ax, 1
step(); write16(ss, uint16_t(-2+bp), ax); // 319f: mov word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 31a2: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 31a5: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 31a8: imul dx
step(); bx=uint16_t(ax); // 31aa: mov bx, ax
step(); alu(read8(ds, uint16_t(4926+bx)),0,8,Op::SUB); // 31ac: cmp byte ptr [bx + 0x133e], 0
step(); if(zf){goto L31b7;} // 31b1: je 0x31b7
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 31b3: xor ax, ax
step(); goto L31ba; // 31b5: jmp 0x31ba
L31b7: step(); ax=uint16_t(1); // 31b7: mov ax, 1
L31ba: step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::OR)); // 31ba: or word ptr [bp - 2], ax
step(); push(read16(ss, uint16_t(-2+bp))); // 31bd: push word ptr [bp - 2]
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 31c0: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(5578+bx))); // 31c3: mov al, byte ptr [bx + 0x15ca]
step(); set_hi(ax,0); // 31c7: mov ah, 0
step(); ax=uint16_t(alu(ax,192,16,Op::ADD)); // 31c9: add ax, 0xc0
step(); push(ax); // 31cc: push ax
step(); push(cs); // 31cd: push cs
step(); push(0x8b1); adapter(0x2928); // 31ce: call 0x2928
step(); cx=uint16_t(pop()); // 31d1: pop cx
step(); cx=uint16_t(pop()); // 31d2: pop cx
L31d3: step(); ds=uint16_t(pop()); // 31d3: pop ds
step(); sp=uint16_t(bp); // 31d4: mov sp, bp
step(); bp=uint16_t(pop()); // 31d6: pop bp
step(); pop();cs=pop();return; // 31d7: retf
L31d8: step(); push(bp); // 31d8: push bp
step(); bp=uint16_t(sp); // 31d9: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 31db: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 31dc: dec sp
step(); push(ds); // 31dd: push ds
step(); ax=uint16_t(5249); // 31de: mov ax, 0x1481
step(); ds=uint16_t(ax); // 31e1: mov ds, ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 31e3: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 31e6: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 31e9: imul dx
step(); bx=uint16_t(ax); // 31eb: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4917+bx))); // 31ed: mov al, byte ptr [bx + 0x1335]
step(); set_hi(ax,0); // 31f1: mov ah, 0
step(); set_lo(cx,4); // 31f3: mov cl, 4
step(); ax=uint16_t(shift(ax,16,Op::SHL,lo(cx))); // 31f5: shl ax, cl
step(); write16(ss, uint16_t(-2+bp), ax); // 31f7: mov word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 31fa: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 31fd: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 3200: imul dx
step(); bx=uint16_t(ax); // 3202: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4920+bx))); // 3204: mov al, byte ptr [bx + 0x1338]
step(); set_hi(ax,0); // 3208: mov ah, 0
step(); ax=uint16_t(alu(ax,15,16,Op::AND)); // 320a: and ax, 0xf
step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::OR)); // 320d: or word ptr [bp - 2], ax
step(); push(read16(ss, uint16_t(-2+bp))); // 3210: push word ptr [bp - 2]
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 3213: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(5542+bx))); // 3216: mov al, byte ptr [bx + 0x15a6]
step(); set_hi(ax,0); // 321a: mov ah, 0
step(); ax=uint16_t(alu(ax,96,16,Op::ADD)); // 321c: add ax, 0x60
step(); push(ax); // 321f: push ax
step(); push(cs); // 3220: push cs
step(); push(0x904); adapter(0x2928); // 3221: call 0x2928
step(); cx=uint16_t(pop()); // 3224: pop cx
step(); cx=uint16_t(pop()); // 3225: pop cx
step(); ds=uint16_t(pop()); // 3226: pop ds
step(); sp=uint16_t(bp); // 3227: mov sp, bp
step(); bp=uint16_t(pop()); // 3229: pop bp
step(); pop();cs=pop();return; // 322a: retf
L322b: step(); push(bp); // 322b: push bp
step(); bp=uint16_t(sp); // 322c: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 322e: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 322f: dec sp
step(); push(ds); // 3230: push ds
step(); ax=uint16_t(5249); // 3231: mov ax, 0x1481
step(); ds=uint16_t(ax); // 3234: mov ds, ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 3236: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 3239: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 323c: imul dx
step(); bx=uint16_t(ax); // 323e: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4918+bx))); // 3240: mov al, byte ptr [bx + 0x1336]
step(); set_hi(ax,0); // 3244: mov ah, 0
step(); set_lo(cx,4); // 3246: mov cl, 4
step(); ax=uint16_t(shift(ax,16,Op::SHL,lo(cx))); // 3248: shl ax, cl
step(); write16(ss, uint16_t(-2+bp), ax); // 324a: mov word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 324d: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 3250: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 3253: imul dx
step(); bx=uint16_t(ax); // 3255: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4921+bx))); // 3257: mov al, byte ptr [bx + 0x1339]
step(); set_hi(ax,0); // 325b: mov ah, 0
step(); ax=uint16_t(alu(ax,15,16,Op::AND)); // 325d: and ax, 0xf
step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::OR)); // 3260: or word ptr [bp - 2], ax
step(); push(read16(ss, uint16_t(-2+bp))); // 3263: push word ptr [bp - 2]
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 3266: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(5542+bx))); // 3269: mov al, byte ptr [bx + 0x15a6]
step(); set_hi(ax,0); // 326d: mov ah, 0
step(); ax=uint16_t(alu(ax,128,16,Op::ADD)); // 326f: add ax, 0x80
step(); push(ax); // 3272: push ax
step(); push(cs); // 3273: push cs
step(); push(0x957); adapter(0x2928); // 3274: call 0x2928
step(); cx=uint16_t(pop()); // 3277: pop cx
step(); cx=uint16_t(pop()); // 3278: pop cx
step(); ds=uint16_t(pop()); // 3279: pop ds
step(); sp=uint16_t(bp); // 327a: mov sp, bp
step(); bp=uint16_t(pop()); // 327c: pop bp
step(); pop();cs=pop();return; // 327d: retf
L327e: step(); push(bp); // 327e: push bp
step(); bp=uint16_t(sp); // 327f: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 3281: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 3282: dec sp
step(); push(ds); // 3283: push ds
step(); ax=uint16_t(5249); // 3284: mov ax, 0x1481
step(); ds=uint16_t(ax); // 3287: mov ds, ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 3289: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 328c: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 328f: imul dx
step(); bx=uint16_t(ax); // 3291: mov bx, ax
step(); alu(read8(ds, uint16_t(4923+bx)),0,8,Op::SUB); // 3293: cmp byte ptr [bx + 0x133b], 0
step(); if(zf){goto L329f;} // 3298: je 0x329f
step(); ax=uint16_t(128); // 329a: mov ax, 0x80
step(); goto L32a1; // 329d: jmp 0x32a1
L329f: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 329f: xor ax, ax
L32a1: step(); write16(ss, uint16_t(-2+bp), ax); // 32a1: mov word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 32a4: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 32a7: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 32aa: imul dx
step(); bx=uint16_t(ax); // 32ac: mov bx, ax
step(); alu(read8(ds, uint16_t(4924+bx)),0,8,Op::SUB); // 32ae: cmp byte ptr [bx + 0x133c], 0
step(); if(zf){goto L32ba;} // 32b3: je 0x32ba
step(); ax=uint16_t(64); // 32b5: mov ax, 0x40
step(); goto L32bc; // 32b8: jmp 0x32bc
L32ba: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 32ba: xor ax, ax
L32bc: step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::ADD)); // 32bc: add word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 32bf: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 32c2: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 32c5: imul dx
step(); bx=uint16_t(ax); // 32c7: mov bx, ax
step(); alu(read8(ds, uint16_t(4919+bx)),0,8,Op::SUB); // 32c9: cmp byte ptr [bx + 0x1337], 0
step(); if(zf){goto L32d5;} // 32ce: je 0x32d5
step(); ax=uint16_t(32); // 32d0: mov ax, 0x20
step(); goto L32d7; // 32d3: jmp 0x32d7
L32d5: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 32d5: xor ax, ax
L32d7: step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::ADD)); // 32d7: add word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 32da: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 32dd: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 32e0: imul dx
step(); bx=uint16_t(ax); // 32e2: mov bx, ax
step(); alu(read8(ds, uint16_t(4925+bx)),0,8,Op::SUB); // 32e4: cmp byte ptr [bx + 0x133d], 0
step(); if(zf){goto L32f0;} // 32e9: je 0x32f0
step(); ax=uint16_t(16); // 32eb: mov ax, 0x10
step(); goto L32f2; // 32ee: jmp 0x32f2
L32f0: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 32f0: xor ax, ax
L32f2: step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::ADD)); // 32f2: add word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 32f5: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 32f8: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 32fb: imul dx
step(); bx=uint16_t(ax); // 32fd: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4915+bx))); // 32ff: mov al, byte ptr [bx + 0x1333]
step(); set_hi(ax,0); // 3303: mov ah, 0
step(); ax=uint16_t(alu(ax,15,16,Op::AND)); // 3305: and ax, 0xf
step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::ADD)); // 3308: add word ptr [bp - 2], ax
step(); push(read16(ss, uint16_t(-2+bp))); // 330b: push word ptr [bp - 2]
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 330e: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(5542+bx))); // 3311: mov al, byte ptr [bx + 0x15a6]
step(); set_hi(ax,0); // 3315: mov ah, 0
step(); ax=uint16_t(alu(ax,32,16,Op::ADD)); // 3317: add ax, 0x20
step(); push(ax); // 331a: push ax
step(); push(cs); // 331b: push cs
step(); push(0x9ff); adapter(0x2928); // 331c: call 0x2928
step(); cx=uint16_t(pop()); // 331f: pop cx
step(); cx=uint16_t(pop()); // 3320: pop cx
step(); ds=uint16_t(pop()); // 3321: pop ds
step(); sp=uint16_t(bp); // 3322: mov sp, bp
step(); bp=uint16_t(pop()); // 3324: pop bp
step(); pop();cs=pop();return; // 3325: retf
L3326: step(); push(bp); // 3326: push bp
step(); bp=uint16_t(sp); // 3327: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 3329: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 332a: dec sp
step(); push(ds); // 332b: push ds
step(); ax=uint16_t(5249); // 332c: mov ax, 0x1481
step(); ds=uint16_t(ax); // 332f: mov ds, ax
step(); alu(read8(ds, uint16_t(4906)),0,8,Op::SUB); // 3331: cmp byte ptr [0x132a], 0
step(); if(zf){goto L333d;} // 3336: je 0x333d
step(); ax=uint16_t(128); // 3338: mov ax, 0x80
step(); goto L333f; // 333b: jmp 0x333f
L333d: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 333d: xor ax, ax
L333f: step(); write16(ss, uint16_t(-2+bp), ax); // 333f: mov word ptr [bp - 2], ax
step(); alu(read8(ds, uint16_t(4911)),0,8,Op::SUB); // 3342: cmp byte ptr [0x132f], 0
step(); if(zf){goto L334e;} // 3347: je 0x334e
step(); ax=uint16_t(64); // 3349: mov ax, 0x40
step(); goto L3350; // 334c: jmp 0x3350
L334e: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 334e: xor ax, ax
L3350: step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::OR)); // 3350: or word ptr [bp - 2], ax
step(); alu(read8(ds, uint16_t(5424)),0,8,Op::SUB); // 3353: cmp byte ptr [0x1530], 0
step(); if(zf){goto L335f;} // 3358: je 0x335f
step(); ax=uint16_t(32); // 335a: mov ax, 0x20
step(); goto L3361; // 335d: jmp 0x3361
L335f: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 335f: xor ax, ax
L3361: step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::OR)); // 3361: or word ptr [bp - 2], ax
step(); set_lo(ax,read8(ds, uint16_t(4910))); // 3364: mov al, byte ptr [0x132e]
step(); set_hi(ax,0); // 3367: mov ah, 0
step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::OR)); // 3369: or word ptr [bp - 2], ax
step(); push(read16(ss, uint16_t(-2+bp))); // 336c: push word ptr [bp - 2]
step(); ax=uint16_t(189); // 336f: mov ax, 0xbd
step(); push(ax); // 3372: push ax
step(); push(cs); // 3373: push cs
step(); push(0xa57); adapter(0x2928); // 3374: call 0x2928
step(); cx=uint16_t(pop()); // 3377: pop cx
step(); cx=uint16_t(pop()); // 3378: pop cx
step(); ds=uint16_t(pop()); // 3379: pop ds
step(); sp=uint16_t(bp); // 337a: mov sp, bp
step(); bp=uint16_t(pop()); // 337c: pop bp
step(); pop();cs=pop();return; // 337d: retf
L337e: step(); push(bp); // 337e: push bp
step(); bp=uint16_t(sp); // 337f: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 3381: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 3382: dec sp
step(); push(ds); // 3383: push ds
step(); ax=uint16_t(5249); // 3384: mov ax, 0x1481
step(); ds=uint16_t(ax); // 3387: mov ds, ax
step(); alu(read16(ds, uint16_t(366)),0,16,Op::SUB); // 3389: cmp word ptr [0x16e], 0
step(); if(zf){goto L33a8;} // 338e: je 0x33a8
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 3390: mov ax, word ptr [bp + 6]
step(); dx=uint16_t(14); // 3393: mov dx, 0xe
step(); math(dx, 16, Math::IMUL); // 3396: imul dx
step(); bx=uint16_t(ax); // 3398: mov bx, ax
step(); set_lo(ax,read8(ds, uint16_t(4927+bx))); // 339a: mov al, byte ptr [bx + 0x133f]
step(); set_hi(ax,0); // 339e: mov ah, 0
step(); ax=uint16_t(alu(ax,3,16,Op::AND)); // 33a0: and ax, 3
step(); write16(ss, uint16_t(-2+bp), ax); // 33a3: mov word ptr [bp - 2], ax
step(); goto L33ad; // 33a6: jmp 0x33ad
L33a8: step(); write16(ss, uint16_t(-2+bp), 0); // 33a8: mov word ptr [bp - 2], 0
L33ad: step(); push(read16(ss, uint16_t(-2+bp))); // 33ad: push word ptr [bp - 2]
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 33b0: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(5542+bx))); // 33b3: mov al, byte ptr [bx + 0x15a6]
step(); set_hi(ax,0); // 33b7: mov ah, 0
step(); ax=uint16_t(alu(ax,224,16,Op::ADD)); // 33b9: add ax, 0xe0
step(); push(ax); // 33bc: push ax
step(); push(cs); // 33bd: push cs
step(); push(0xaa1); adapter(0x2928); // 33be: call 0x2928
step(); cx=uint16_t(pop()); // 33c1: pop cx
step(); cx=uint16_t(pop()); // 33c2: pop cx
step(); ds=uint16_t(pop()); // 33c3: pop ds
step(); sp=uint16_t(bp); // 33c4: mov sp, bp
step(); bp=uint16_t(pop()); // 33c6: pop bp
step(); pop();cs=pop();return; // 33c7: retf
L33c8: step(); push(bp); // 33c8: push bp
step(); bp=uint16_t(sp); // 33c9: mov bp, sp
step(); push(ds); // 33cb: push ds
step(); ax=uint16_t(5249); // 33cc: mov ax, 0x1481
step(); ds=uint16_t(ax); // 33cf: mov ds, ax
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 33d1: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(337+bx))); // 33d4: mov al, byte ptr [bx + 0x151]
step(); set_hi(ax,0); // 33d8: mov ah, 0
step(); push(ax); // 33da: push ax
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 33db: mov bx, word ptr [bp + 6]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 33de: shl bx, 1
step(); push(read16(ds, uint16_t(472+bx))); // 33e0: push word ptr [bx + 0x1d8]
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 33e4: mov bx, word ptr [bp + 6]
step(); set_lo(ax,read8(ds, uint16_t(368+bx))); // 33e7: mov al, byte ptr [bx + 0x170]
step(); set_hi(ax,0); // 33eb: mov ah, 0
step(); push(ax); // 33ed: push ax
step(); push(read16(ss, uint16_t(6+bp))); // 33ee: push word ptr [bp + 6]
step(); push(cs); // 33f1: push cs
step(); push(0xad5); execute(0x34ad); // 33f2: call 0x34ad
step(); ; // 33f5: nop
step(); sp=uint16_t(alu(sp,8,16,Op::ADD)); // 33f6: add sp, 8
step(); bx=uint16_t(read16(ss, uint16_t(6+bp))); // 33f9: mov bx, word ptr [bp + 6]
step(); write8(ds, uint16_t(404+bx), lo(ax)); // 33fc: mov byte ptr [bx + 0x194], al
step(); ds=uint16_t(pop()); // 3400: pop ds
step(); bp=uint16_t(pop()); // 3401: pop bp
step(); pop();cs=pop();return; // 3402: retf
L34ad: step(); push(bp); // 34ad: push bp
step(); bp=uint16_t(sp); // 34ae: mov bp, sp
step(); sp=uint16_t(alu(sp,8,16,Op::SUB)); // 34b0: sub sp, 8
step(); push(ds); // 34b3: push ds
step(); ax=uint16_t(5249); // 34b4: mov ax, 0x1481
step(); ds=uint16_t(ax); // 34b7: mov ds, ax
step(); ax=uint16_t(read16(ss, uint16_t(8+bp))); // 34b9: mov ax, word ptr [bp + 8]
step(); bx=uint16_t(12); // 34bc: mov bx, 0xc
step(); dx=int16_t(ax)<0?65535:0; // 34bf: cdq
step(); math(bx, 16, Math::IDIV); // 34c0: idiv bx
step(); ax=uint16_t(incdec(ax,16,true)); // 34c2: dec ax
step(); write16(ss, uint16_t(-4+bp), ax); // 34c3: mov word ptr [bp - 4], ax
step(); ax=uint16_t(read16(ss, uint16_t(8+bp))); // 34c6: mov ax, word ptr [bp + 8]
step(); bx=uint16_t(12); // 34c9: mov bx, 0xc
step(); dx=int16_t(ax)<0?65535:0; // 34cc: cdq
step(); math(bx, 16, Math::IDIV); // 34cd: idiv bx
step(); dx=uint16_t(shift(dx,16,Op::SHL,1)); // 34cf: shl dx, 1
step(); bx=uint16_t(dx); // 34d1: mov bx, dx
step(); ax=uint16_t(read16(ds, uint16_t(40+bx))); // 34d3: mov ax, word ptr [bx + 0x28]
step(); write16(ss, uint16_t(-2+bp), ax); // 34d7: mov word ptr [bp - 2], ax
step(); alu(read16(ss, uint16_t(10+bp)),8192,16,Op::SUB); // 34da: cmp word ptr [bp + 0xa], 0x2000
step(); if(!zf){goto L34e4;} // 34df: jne 0x34e4
step(); goto L3599; // 34e1: jmp 0x3599
L34e4: step(); alu(read16(ss, uint16_t(10+bp)),8192,16,Op::SUB); // 34e4: cmp word ptr [bp + 0xa], 0x2000
step(); if(zf||(sf!=of)){goto L3540;} // 34e9: jle 0x3540
step(); write16(ss, uint16_t(10+bp), alu(read16(ss, uint16_t(10+bp)),8192,16,Op::SUB)); // 34eb: sub word ptr [bp + 0xa], 0x2000
step(); ax=uint16_t(read16(ss, uint16_t(8+bp))); // 34f0: mov ax, word ptr [bp + 8]
step(); ax=uint16_t(alu(ax,read16(ds, uint16_t(4912)),16,Op::ADD)); // 34f3: add ax, word ptr [0x1330]
step(); bx=uint16_t(12); // 34f7: mov bx, 0xc
step(); dx=int16_t(ax)<0?65535:0; // 34fa: cdq
step(); math(bx, 16, Math::IDIV); // 34fb: idiv bx
step(); dx=uint16_t(shift(dx,16,Op::SHL,1)); // 34fd: shl dx, 1
step(); bx=uint16_t(dx); // 34ff: mov bx, dx
step(); ax=uint16_t(read16(ds, uint16_t(40+bx))); // 3501: mov ax, word ptr [bx + 0x28]
step(); write16(ss, uint16_t(-8+bp), ax); // 3505: mov word ptr [bp - 8], ax
step(); ax=uint16_t(read16(ss, uint16_t(-8+bp))); // 3508: mov ax, word ptr [bp - 8]
step(); alu(ax,read16(ss, uint16_t(-2+bp)),16,Op::SUB); // 350b: cmp ax, word ptr [bp - 2]
step(); if(!cf){goto L3513;} // 350e: jae 0x3513
step(); write16(ss, uint16_t(-8+bp), shift(read16(ss, uint16_t(-8+bp)),16,Op::SHL,1)); // 3510: shl word ptr [bp - 8], 1
L3513: step(); ax=uint16_t(read16(ss, uint16_t(-8+bp))); // 3513: mov ax, word ptr [bp - 8]
step(); ax=uint16_t(alu(ax,read16(ss, uint16_t(-2+bp)),16,Op::SUB)); // 3516: sub ax, word ptr [bp - 2]
step(); write16(ss, uint16_t(-8+bp), ax); // 3519: mov word ptr [bp - 8], ax
step(); ax=uint16_t(read16(ss, uint16_t(-8+bp))); // 351c: mov ax, word ptr [bp - 8]
step(); math(read16(ss, uint16_t(10+bp)), 16, Math::MUL); // 351f: mul word ptr [bp + 0xa]
step(); cx=uint16_t(13); // 3522: mov cx, 0xd
L3525: step(); cf=false; // 3525: clc
step(); dx=uint16_t(shift(dx,16,Op::RCR,1)); // 3526: rcr dx, 1
step(); ax=uint16_t(shift(ax,16,Op::RCR,1)); // 3528: rcr ax, 1
step(); --cx;if(cx){goto L3525;} // 352a: loop 0x3525
step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::ADD)); // 352c: add word ptr [bp - 2], ax
step(); goto L3537; // 352f: jmp 0x3537
L3531: step(); write16(ss, uint16_t(-2+bp), shift(read16(ss, uint16_t(-2+bp)),16,Op::SHR,1)); // 3531: shr word ptr [bp - 2], 1
step(); write16(ss, uint16_t(-4+bp), incdec(read16(ss, uint16_t(-4+bp)),16,false)); // 3534: inc word ptr [bp - 4]
L3537: step(); alu(read16(ss, uint16_t(-2+bp)),1023,16,Op::SUB); // 3537: cmp word ptr [bp - 2], 0x3ff
step(); if(!cf&&!zf){goto L3531;} // 353c: ja 0x3531
step(); goto L3599; // 353e: jmp 0x3599
L3540: step(); ax=uint16_t(8192); // 3540: mov ax, 0x2000
step(); ax=uint16_t(alu(ax,read16(ss, uint16_t(10+bp)),16,Op::SUB)); // 3543: sub ax, word ptr [bp + 0xa]
step(); write16(ss, uint16_t(10+bp), ax); // 3546: mov word ptr [bp + 0xa], ax
step(); ax=uint16_t(read16(ss, uint16_t(8+bp))); // 3549: mov ax, word ptr [bp + 8]
step(); ax=uint16_t(alu(ax,read16(ds, uint16_t(4912)),16,Op::SUB)); // 354c: sub ax, word ptr [0x1330]
step(); bx=uint16_t(12); // 3550: mov bx, 0xc
step(); dx=int16_t(ax)<0?65535:0; // 3553: cdq
step(); math(bx, 16, Math::IDIV); // 3554: idiv bx
step(); dx=uint16_t(shift(dx,16,Op::SHL,1)); // 3556: shl dx, 1
step(); bx=uint16_t(dx); // 3558: mov bx, dx
step(); ax=uint16_t(read16(ds, uint16_t(40+bx))); // 355a: mov ax, word ptr [bx + 0x28]
step(); write16(ss, uint16_t(-8+bp), ax); // 355e: mov word ptr [bp - 8], ax
step(); ax=uint16_t(read16(ss, uint16_t(-8+bp))); // 3561: mov ax, word ptr [bp - 8]
step(); alu(ax,read16(ss, uint16_t(-2+bp)),16,Op::SUB); // 3564: cmp ax, word ptr [bp - 2]
step(); if(cf||zf){goto L356c;} // 3567: jbe 0x356c
step(); write16(ss, uint16_t(-8+bp), shift(read16(ss, uint16_t(-8+bp)),16,Op::SHR,1)); // 3569: shr word ptr [bp - 8], 1
L356c: step(); ax=uint16_t(read16(ss, uint16_t(-2+bp))); // 356c: mov ax, word ptr [bp - 2]
step(); ax=uint16_t(alu(ax,read16(ss, uint16_t(-8+bp)),16,Op::SUB)); // 356f: sub ax, word ptr [bp - 8]
step(); write16(ss, uint16_t(-8+bp), ax); // 3572: mov word ptr [bp - 8], ax
step(); ax=uint16_t(read16(ss, uint16_t(-8+bp))); // 3575: mov ax, word ptr [bp - 8]
step(); math(read16(ss, uint16_t(10+bp)), 16, Math::MUL); // 3578: mul word ptr [bp + 0xa]
step(); cx=uint16_t(13); // 357b: mov cx, 0xd
L357e: step(); cf=false; // 357e: clc
step(); dx=uint16_t(shift(dx,16,Op::RCR,1)); // 357f: rcr dx, 1
step(); ax=uint16_t(shift(ax,16,Op::RCR,1)); // 3581: rcr ax, 1
step(); --cx;if(cx){goto L357e;} // 3583: loop 0x357e
step(); write16(ss, uint16_t(-2+bp), alu(read16(ss, uint16_t(-2+bp)),ax,16,Op::SUB)); // 3585: sub word ptr [bp - 2], ax
step(); goto L3590; // 3588: jmp 0x3590
L358a: step(); write16(ss, uint16_t(-2+bp), shift(read16(ss, uint16_t(-2+bp)),16,Op::SHL,1)); // 358a: shl word ptr [bp - 2], 1
step(); write16(ss, uint16_t(-4+bp), incdec(read16(ss, uint16_t(-4+bp)),16,true)); // 358d: dec word ptr [bp - 4]
L3590: step(); ax=uint16_t(read16(ss, uint16_t(-2+bp))); // 3590: mov ax, word ptr [bp - 2]
step(); alu(ax,read16(ds, uint16_t(40)),16,Op::SUB); // 3593: cmp ax, word ptr [0x28]
step(); if(cf){goto L358a;} // 3597: jb 0x358a
L3599: step(); push(read16(ss, uint16_t(-2+bp))); // 3599: push word ptr [bp - 2]
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 359c: mov ax, word ptr [bp + 6]
step(); ax=uint16_t(alu(ax,160,16,Op::ADD)); // 359f: add ax, 0xa0
step(); push(ax); // 35a2: push ax
step(); push(cs); // 35a3: push cs
step(); push(0xc87); adapter(0x2928); // 35a4: call 0x2928
step(); cx=uint16_t(pop()); // 35a7: pop cx
step(); cx=uint16_t(pop()); // 35a8: pop cx
step(); ax=uint16_t(read16(ss, uint16_t(-4+bp))); // 35a9: mov ax, word ptr [bp - 4]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 35ac: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 35ae: shl ax, 1
step(); dx=uint16_t(read16(ss, uint16_t(12+bp))); // 35b0: mov dx, word ptr [bp + 0xc]
step(); dx=uint16_t(alu(dx,ax,16,Op::OR)); // 35b3: or dx, ax
step(); ax=uint16_t(read16(ss, uint16_t(-2+bp))); // 35b5: mov ax, word ptr [bp - 2]
step(); set_lo(cx,8); // 35b8: mov cl, 8
step(); ax=uint16_t(shift(ax,16,Op::SHR,lo(cx))); // 35ba: shr ax, cl
step(); dx=uint16_t(alu(dx,ax,16,Op::OR)); // 35bc: or dx, ax
step(); write16(ss, uint16_t(-6+bp), dx); // 35be: mov word ptr [bp - 6], dx
step(); push(read16(ss, uint16_t(-6+bp))); // 35c1: push word ptr [bp - 6]
step(); ax=uint16_t(read16(ss, uint16_t(6+bp))); // 35c4: mov ax, word ptr [bp + 6]
step(); ax=uint16_t(alu(ax,176,16,Op::ADD)); // 35c7: add ax, 0xb0
step(); push(ax); // 35ca: push ax
step(); push(cs); // 35cb: push cs
step(); push(0xcaf); adapter(0x2928); // 35cc: call 0x2928
step(); cx=uint16_t(pop()); // 35cf: pop cx
step(); cx=uint16_t(pop()); // 35d0: pop cx
step(); ax=uint16_t(read16(ss, uint16_t(-6+bp))); // 35d1: mov ax, word ptr [bp - 6]
step(); goto L35d6; // 35d4: jmp 0x35d6
L35d6: step(); ds=uint16_t(pop()); // 35d6: pop ds
step(); sp=uint16_t(bp); // 35d7: mov sp, bp
step(); bp=uint16_t(pop()); // 35d9: pop bp
step(); pop();cs=pop();return; // 35da: retf
L3afd: step(); push(bp); // 3afd: push bp
step(); bp=uint16_t(sp); // 3afe: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 3b00: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 3b01: dec sp
step(); push(ds); // 3b02: push ds
step(); ax=uint16_t(5249); // 3b03: mov ax, 0x1481
step(); ds=uint16_t(ax); // 3b06: mov ds, ax
step(); ax=uint16_t(read16(ss, uint16_t(10+bp))); // 3b08: mov ax, word ptr [bp + 0xa]
step(); bx=uint16_t(12); // 3b0b: mov bx, 0xc
step(); dx=uint16_t(alu(dx,dx,16,Op::XOR)); // 3b0e: xor dx, dx
step(); math(bx, 16, Math::DIV); // 3b10: div bx
step(); alu(ax,7,16,Op::SUB); // 3b12: cmp ax, 7
step(); if(cf||zf){goto L3b19;} // 3b15: jbe 0x3b19
step(); goto L3b5e; // 3b17: jmp 0x3b5e
L3b19: step(); ax=uint16_t(read16(ss, uint16_t(8+bp))); // 3b19: mov ax, word ptr [bp + 8]
step(); dx=uint16_t(alu(dx,dx,16,Op::XOR)); // 3b1c: xor dx, dx
step(); math(read16(ds, uint16_t(66)), 16, Math::DIV); // 3b1e: div word ptr [0x42]
step(); alu(dx,read16(ss, uint16_t(6+bp)),16,Op::SUB); // 3b22: cmp dx, word ptr [bp + 6]
step(); if(!zf){goto L3b5e;} // 3b25: jne 0x3b5e
step(); alu(read16(ss, uint16_t(10+bp)),12,16,Op::SUB); // 3b27: cmp word ptr [bp + 0xa], 0xc
step(); if(!cf){goto L3b2f;} // 3b2b: jae 0x3b2f
step(); goto L3b5e; // 3b2d: jmp 0x3b5e
L3b2f: step(); ax=uint16_t(read16(ss, uint16_t(10+bp))); // 3b2f: mov ax, word ptr [bp + 0xa]
step(); bx=uint16_t(12); // 3b32: mov bx, 0xc
step(); dx=uint16_t(alu(dx,dx,16,Op::XOR)); // 3b35: xor dx, dx
step(); math(bx, 16, Math::DIV); // 3b37: div bx
step(); dx=uint16_t(shift(dx,16,Op::SHL,1)); // 3b39: shl dx, 1
step(); bx=uint16_t(dx); // 3b3b: mov bx, dx
step(); ax=uint16_t(read16(ds, uint16_t(82+bx))); // 3b3d: mov ax, word ptr [bx + 0x52]
step(); write16(ss, uint16_t(-2+bp), ax); // 3b41: mov word ptr [bp - 2], ax
step(); ax=uint16_t(read16(ss, uint16_t(10+bp))); // 3b44: mov ax, word ptr [bp + 0xa]
step(); bx=uint16_t(12); // 3b47: mov bx, 0xc
step(); dx=uint16_t(alu(dx,dx,16,Op::XOR)); // 3b4a: xor dx, dx
step(); math(bx, 16, Math::DIV); // 3b4c: div bx
step(); set_lo(cx,8); // 3b4e: mov cl, 8
step(); set_lo(cx,alu(lo(cx),lo(ax),8,Op::SUB)); // 3b50: sub cl, al
step(); write16(ss, uint16_t(-2+bp), shift(read16(ss, uint16_t(-2+bp)),16,Op::SHR,lo(cx))); // 3b52: shr word ptr [bp - 2], cl
step(); push(read16(ss, uint16_t(-2+bp))); // 3b55: push word ptr [bp - 2]
step(); push(cs);push(0x123d);cs=uint16_t(4096);adapter(0x1c71); // 3b58: lcall 0x1000, 0x1c71
step(); cx=uint16_t(pop()); // 3b5d: pop cx
L3b5e: step(); ds=uint16_t(pop()); // 3b5e: pop ds
step(); sp=uint16_t(bp); // 3b5f: mov sp, bp
step(); bp=uint16_t(pop()); // 3b61: pop bp
step(); pop();cs=pop();return; // 3b62: retf
L3b63: step(); push(bp); // 3b63: push bp
step(); bp=uint16_t(sp); // 3b64: mov bp, sp
step(); push(ds); // 3b66: push ds
step(); ax=uint16_t(5249); // 3b67: mov ax, 0x1481
step(); ds=uint16_t(ax); // 3b6a: mov ds, ax
step(); ax=uint16_t(read16(ss, uint16_t(8+bp))); // 3b6c: mov ax, word ptr [bp + 8]
step(); dx=uint16_t(alu(dx,dx,16,Op::XOR)); // 3b6f: xor dx, dx
step(); math(read16(ds, uint16_t(66)), 16, Math::DIV); // 3b71: div word ptr [0x42]
step(); alu(dx,read16(ss, uint16_t(6+bp)),16,Op::SUB); // 3b75: cmp dx, word ptr [bp + 6]
step(); if(!zf){goto L3b7f;} // 3b78: jne 0x3b7f
step(); push(cs);push(0x125f);cs=uint16_t(4096);adapter(0x1ca4); // 3b7a: lcall 0x1000, 0x1ca4
L3b7f: step(); ds=uint16_t(pop()); // 3b7f: pop ds
step(); bp=uint16_t(pop()); // 3b80: pop bp
step(); pop();cs=pop();return; // 3b81: retf
L3b98: step(); alu(read16(ds, uint16_t(80)),1,16,Op::SUB); // 3b98: cmp word ptr [0x50], 1
step(); if(cf||zf){goto L3ba6;} // 3b9d: jbe 0x3ba6
step(); write16(ds, uint16_t(80), incdec(read16(ds, uint16_t(80)),16,true)); // 3b9f: dec word ptr [0x50]
step(); return; // 3ba3: jmp 0x4146
L3ba6: step(); bx=uint16_t(read16(ds, uint16_t(108))); // 3ba6: mov bx, word ptr [0x6c]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3baa: shl bx, 1
step(); ax=uint16_t(read16(ds, uint16_t(138+bx))); // 3bac: mov ax, word ptr [bx + 0x8a]
step(); write16(ds, uint16_t(358), ax); // 3bb0: mov word ptr [0x166], ax
step(); alu(read16(ds, uint16_t(358)),64,16,Op::SUB); // 3bb3: cmp word ptr [0x166], 0x40
step(); if(cf){goto L3bc4;} // 3bb8: jb 0x3bc4
step(); alu(read16(ds, uint16_t(124)),0,16,Op::SUB); // 3bba: cmp word ptr [0x7c], 0
step(); if(!zf){goto L3bc4;} // 3bbf: jne 0x3bc4
step(); goto L405c; // 3bc1: jmp 0x405c
L3bc4: step(); alu(read16(ds, uint16_t(358)),64,16,Op::SUB); // 3bc4: cmp word ptr [0x166], 0x40
step(); if(cf){goto L3bde;} // 3bc9: jb 0x3bde
step(); write16(ds, uint16_t(108), 0); // 3bcb: mov word ptr [0x6c], 0
step(); bx=uint16_t(read16(ds, uint16_t(108))); // 3bd1: mov bx, word ptr [0x6c]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3bd5: shl bx, 1
step(); ax=uint16_t(read16(ds, uint16_t(138+bx))); // 3bd7: mov ax, word ptr [bx + 0x8a]
step(); write16(ds, uint16_t(358), ax); // 3bdb: mov word ptr [0x166], ax
L3bde: step(); write16(ds, uint16_t(112), 1); // 3bde: mov word ptr [0x70], 1
step(); write16(ds, uint16_t(134), 0); // 3be4: mov word ptr [0x86], 0
step(); goto L3ffd; // 3bea: jmp 0x3ffd
L3bed: step(); alu(read16(ds, uint16_t(66)),0,16,Op::SUB); // 3bed: cmp word ptr [0x42], 0
step(); if(zf){goto L3bf7;} // 3bf2: je 0x3bf7
step(); goto L3ee3; // 3bf4: jmp 0x3ee3
L3bf7: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3bf7: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3bfb: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3bfd: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3bff: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3c03: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3c06: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3c09: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3c0b: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3c0d: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3c10: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3c12: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3c14: add bx, ax
step(); alu(read8(es, uint16_t(0+bx)),11,8,Op::SUB); // 3c16: cmp byte ptr es:[bx], 0xb
step(); if(!cf&&!zf){goto L3c1f;} // 3c1a: ja 0x3c1f
step(); goto L3ee0; // 3c1c: jmp 0x3ee0
L3c1f: step(); alu(read16(ds, uint16_t(106)),0,16,Op::SUB); // 3c1f: cmp word ptr [0x6a], 0
step(); if(zf){goto L3c5a;} // 3c24: je 0x3c5a
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3c26: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3c2a: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3c2c: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3c2e: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3c32: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3c35: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3c38: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3c3a: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3c3c: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3c3f: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3c41: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3c43: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(1+bx))); // 3c45: mov al, byte ptr es:[bx + 1]
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3c49: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3c4d: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3c4f: shl bx, 1
step(); alu(lo(ax),read8(ds, uint16_t(423+bx)),8,Op::SUB); // 3c51: cmp al, byte ptr [bx + 0x1a7]
step(); if(!zf){goto L3c5a;} // 3c55: jne 0x3c5a
step(); goto L3ce9; // 3c57: jmp 0x3ce9
L3c5a: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3c5a: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3c5e: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3c60: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3c62: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3c66: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3c69: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3c6c: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3c6e: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3c70: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3c73: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3c75: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3c77: add bx, ax
step(); alu(read8(es, uint16_t(1+bx)),0,8,Op::SUB); // 3c79: cmp byte ptr es:[bx + 1], 0
step(); if(zf){goto L3cba;} // 3c7e: je 0x3cba
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3c80: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3c84: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3c86: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3c88: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3c8c: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3c8f: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3c92: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3c94: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3c96: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3c99: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3c9b: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3c9d: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(1+bx))); // 3c9f: mov al, byte ptr es:[bx + 1]
step(); set_hi(ax,0); // 3ca3: mov ah, 0
step(); dx=uint16_t(69); // 3ca5: mov dx, 0x45
step(); math(dx, 16, Math::IMUL); // 3ca8: imul dx
step(); ax=uint16_t(alu(ax,421,16,Op::ADD)); // 3caa: add ax, 0x1a5
step(); push(ds); // 3cad: push ds
step(); push(ax); // 3cae: push ax
step(); push(read16(ds, uint16_t(134))); // 3caf: push word ptr [0x86]
step(); push(cs); // 3cb3: push cs
step(); push(0x1397); execute(0x2b45); // 3cb4: call 0x2b45
step(); sp=uint16_t(alu(sp,6,16,Op::ADD)); // 3cb7: add sp, 6
L3cba: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3cba: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3cbe: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3cc0: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3cc2: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3cc6: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3cc9: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3ccc: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3cce: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3cd0: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3cd3: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3cd5: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3cd7: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(1+bx))); // 3cd9: mov al, byte ptr es:[bx + 1]
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3cdd: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3ce1: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3ce3: shl bx, 1
step(); write8(ds, uint16_t(423+bx), lo(ax)); // 3ce5: mov byte ptr [bx + 0x1a7], al
L3ce9: step(); alu(read16(ds, uint16_t(106)),0,16,Op::SUB); // 3ce9: cmp word ptr [0x6a], 0
step(); if(zf){goto L3d2e;} // 3cee: je 0x3d2e
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3cf0: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3cf4: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3cf6: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3cf8: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3cfc: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3cff: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3d02: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3d04: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3d06: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3d09: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3d0b: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3d0d: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(2+bx))); // 3d0f: mov al, byte ptr es:[bx + 2]
step(); set_hi(ax,0); // 3d13: mov ah, 0
step(); ax=uint16_t(alu(ax,read16(ds, uint16_t(414)),16,Op::SUB)); // 3d15: sub ax, word ptr [0x19e]
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3d19: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3d1d: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3d1f: shl bx, 1
step(); set_lo(dx,read8(ds, uint16_t(424+bx))); // 3d21: mov dl, byte ptr [bx + 0x1a8]
step(); set_hi(dx,0); // 3d25: mov dh, 0
step(); alu(ax,dx,16,Op::SUB); // 3d27: cmp ax, dx
step(); if(!zf){goto L3d2e;} // 3d29: jne 0x3d2e
step(); goto L3dcf; // 3d2b: jmp 0x3dcf
L3d2e: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3d2e: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3d32: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3d34: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3d36: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3d3a: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3d3d: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3d40: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3d42: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3d44: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3d47: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3d49: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3d4b: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(2+bx))); // 3d4d: mov al, byte ptr es:[bx + 2]
step(); set_hi(ax,0); // 3d51: mov ah, 0
step(); alu(ax,read16(ds, uint16_t(414)),16,Op::SUB); // 3d53: cmp ax, word ptr [0x19e]
step(); if(cf){goto L3d8f;} // 3d57: jb 0x3d8f
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3d59: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3d5d: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3d5f: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3d61: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3d65: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3d68: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3d6b: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3d6d: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3d6f: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3d72: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3d74: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3d76: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(2+bx))); // 3d78: mov al, byte ptr es:[bx + 2]
step(); set_hi(ax,0); // 3d7c: mov ah, 0
step(); ax=uint16_t(alu(ax,read16(ds, uint16_t(414)),16,Op::SUB)); // 3d7e: sub ax, word ptr [0x19e]
step(); push(ax); // 3d82: push ax
step(); push(read16(ds, uint16_t(134))); // 3d83: push word ptr [0x86]
step(); push(cs); // 3d87: push cs
step(); push(0x146b); execute(0x2bfa); // 3d88: call 0x2bfa
step(); cx=uint16_t(pop()); // 3d8b: pop cx
step(); cx=uint16_t(pop()); // 3d8c: pop cx
step(); goto L3d9c; // 3d8d: jmp 0x3d9c
L3d8f: step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 3d8f: xor ax, ax
step(); push(ax); // 3d91: push ax
step(); push(read16(ds, uint16_t(134))); // 3d92: push word ptr [0x86]
step(); push(cs); // 3d96: push cs
step(); push(0x147a); execute(0x2bfa); // 3d97: call 0x2bfa
step(); cx=uint16_t(pop()); // 3d9a: pop cx
step(); cx=uint16_t(pop()); // 3d9b: pop cx
L3d9c: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3d9c: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3da0: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3da2: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3da4: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3da8: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3dab: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3dae: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3db0: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3db2: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3db5: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3db7: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3db9: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(2+bx))); // 3dbb: mov al, byte ptr es:[bx + 2]
step(); set_lo(ax,alu(lo(ax),read8(ds, uint16_t(414)),8,Op::SUB)); // 3dbf: sub al, byte ptr [0x19e]
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3dc3: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3dc7: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3dc9: shl bx, 1
step(); write8(ds, uint16_t(424+bx), lo(ax)); // 3dcb: mov byte ptr [bx + 0x1a8], al
L3dcf: step(); alu(read16(ds, uint16_t(106)),0,16,Op::SUB); // 3dcf: cmp word ptr [0x6a], 0
step(); if(zf){goto L3e0a;} // 3dd4: je 0x3e0a
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3dd6: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3dda: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3ddc: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3dde: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3de2: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3de5: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3de8: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3dea: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3dec: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3def: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3df1: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3df3: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(3+bx))); // 3df5: mov al, byte ptr es:[bx + 3]
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3df9: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3dfd: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3dff: shl bx, 1
step(); alu(lo(ax),read8(ds, uint16_t(425+bx)),8,Op::SUB); // 3e01: cmp al, byte ptr [bx + 0x1a9]
step(); if(!zf){goto L3e0a;} // 3e05: jne 0x3e0a
step(); goto L3e70; // 3e07: jmp 0x3e70
L3e0a: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3e0a: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3e0e: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3e10: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3e12: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3e16: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3e19: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3e1c: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3e1e: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3e20: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3e23: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3e25: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3e27: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(3+bx))); // 3e29: mov al, byte ptr es:[bx + 3]
step(); set_hi(ax,0); // 3e2d: mov ah, 0
step(); set_lo(cx,8); // 3e2f: mov cl, 8
step(); ax=uint16_t(shift(ax,16,Op::SHL,lo(cx))); // 3e31: shl ax, cl
step(); ax=uint16_t(alu(ax,255,16,Op::OR)); // 3e33: or ax, 0xff
step(); push(ax); // 3e36: push ax
step(); push(read16(ds, uint16_t(134))); // 3e37: push word ptr [0x86]
step(); push(cs); // 3e3b: push cs
step(); push(0x151f); execute(0x2c79); // 3e3c: call 0x2c79
step(); cx=uint16_t(pop()); // 3e3f: pop cx
step(); cx=uint16_t(pop()); // 3e40: pop cx
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3e41: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3e45: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3e47: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3e49: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3e4d: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3e50: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3e53: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3e55: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3e57: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3e5a: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3e5c: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3e5e: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(3+bx))); // 3e60: mov al, byte ptr es:[bx + 3]
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3e64: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3e68: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3e6a: shl bx, 1
step(); write8(ds, uint16_t(425+bx), lo(ax)); // 3e6c: mov byte ptr [bx + 0x1a9], al
L3e70: step(); alu(read16(ds, uint16_t(74)),0,16,Op::SUB); // 3e70: cmp word ptr [0x4a], 0
step(); if(zf){goto L3e7a;} // 3e75: je 0x3e7a
step(); goto L3ee0; // 3e77: jmp 0x3ee0
L3e7a: step(); push(read16(ds, uint16_t(134))); // 3e7a: push word ptr [0x86]
step(); push(cs); // 3e7e: push cs
step(); push(0x1562); execute(0x2d75); // 3e7f: call 0x2d75
step(); cx=uint16_t(pop()); // 3e82: pop cx
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3e83: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3e87: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3e89: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3e8b: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3e8f: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3e92: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3e95: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3e97: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3e99: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3e9c: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3e9e: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3ea0: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(0+bx))); // 3ea2: mov al, byte ptr es:[bx]
step(); set_hi(ax,0); // 3ea5: mov ah, 0
step(); push(ax); // 3ea7: push ax
step(); push(read16(ds, uint16_t(134))); // 3ea8: push word ptr [0x86]
step(); push(cs); // 3eac: push cs
step(); push(0x1590); execute(0x2cbb); // 3ead: call 0x2cbb
step(); cx=uint16_t(pop()); // 3eb0: pop cx
step(); cx=uint16_t(pop()); // 3eb1: pop cx
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3eb2: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3eb6: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3eb8: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3eba: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3ebe: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3ec1: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3ec4: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3ec6: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3ec8: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3ecb: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3ecd: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3ecf: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(0+bx))); // 3ed1: mov al, byte ptr es:[bx]
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3ed4: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3ed8: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3eda: shl bx, 1
step(); write8(ds, uint16_t(422+bx), lo(ax)); // 3edc: mov byte ptr [bx + 0x1a6], al
L3ee0: step(); goto L3f9b; // 3ee0: jmp 0x3f9b
L3ee3: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3ee3: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3ee7: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3ee9: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3eeb: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3eef: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3ef2: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3ef5: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3ef7: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3ef9: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3efc: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3efe: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3f00: add bx, ax
step(); alu(read8(es, uint16_t(0+bx)),0,8,Op::SUB); // 3f02: cmp byte ptr es:[bx], 0
step(); if(cf||zf){goto L3f36;} // 3f06: jbe 0x3f36
step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3f08: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f0c: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f0e: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3f10: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3f14: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3f17: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3f1a: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3f1c: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3f1e: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3f21: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3f23: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3f25: add bx, ax
step(); set_lo(ax,read8(es, uint16_t(0+bx))); // 3f27: mov al, byte ptr es:[bx]
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3f2a: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f2e: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f30: shl bx, 1
step(); write8(ds, uint16_t(422+bx), lo(ax)); // 3f32: mov byte ptr [bx + 0x1a6], al
L3f36: step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3f36: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f3a: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f3c: shl bx, 1
step(); alu(read8(ds, uint16_t(422+bx)),11,8,Op::SUB); // 3f3e: cmp byte ptr [bx + 0x1a6], 0xb
step(); if(cf||zf){goto L3f71;} // 3f43: jbe 0x3f71
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3f45: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f49: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f4b: shl bx, 1
step(); set_lo(ax,read8(ds, uint16_t(422+bx))); // 3f4d: mov al, byte ptr [bx + 0x1a6]
step(); set_hi(ax,0); // 3f51: mov ah, 0
step(); push(ax); // 3f53: push ax
step(); set_lo(cx,read8(ds, uint16_t(72))); // 3f54: mov cl, byte ptr [0x48]
step(); set_lo(cx,incdec(lo(cx),8,true)); // 3f58: dec cl
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3f5a: mov ax, word ptr [0x6a]
step(); ax=uint16_t(shift(ax,16,Op::SHL,lo(cx))); // 3f5d: shl ax, cl
step(); ax=uint16_t(alu(ax,read16(ds, uint16_t(74)),16,Op::ADD)); // 3f5f: add ax, word ptr [0x4a]
step(); push(ax); // 3f63: push ax
step(); push(read16(ds, uint16_t(134))); // 3f64: push word ptr [0x86]
step(); push(cs); // 3f68: push cs
step(); push(0x164c); execute(0x3afd); // 3f69: call 0x3afd
step(); sp=uint16_t(alu(sp,6,16,Op::ADD)); // 3f6c: add sp, 6
step(); goto L3f9b; // 3f6f: jmp 0x3f9b
L3f71: step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3f71: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f75: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f77: shl bx, 1
step(); set_lo(ax,read8(ds, uint16_t(422+bx))); // 3f79: mov al, byte ptr [bx + 0x1a6]
step(); set_hi(ax,0); // 3f7d: mov ah, 0
step(); push(ax); // 3f7f: push ax
step(); set_lo(cx,read8(ds, uint16_t(72))); // 3f80: mov cl, byte ptr [0x48]
step(); set_lo(cx,incdec(lo(cx),8,true)); // 3f84: dec cl
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3f86: mov ax, word ptr [0x6a]
step(); ax=uint16_t(shift(ax,16,Op::SHL,lo(cx))); // 3f89: shl ax, cl
step(); ax=uint16_t(alu(ax,read16(ds, uint16_t(74)),16,Op::ADD)); // 3f8b: add ax, word ptr [0x4a]
step(); push(ax); // 3f8f: push ax
step(); push(read16(ds, uint16_t(134))); // 3f90: push word ptr [0x86]
step(); push(cs); // 3f94: push cs
step(); push(0x1678); execute(0x3b63); // 3f95: call 0x3b63
step(); sp=uint16_t(alu(sp,6,16,Op::ADD)); // 3f98: add sp, 6
L3f9b: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 3f9b: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3f9f: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3fa1: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 3fa3: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3fa7: mov ax, word ptr [0x6a]
step(); dx=uint16_t(48); // 3faa: mov dx, 0x30
step(); math(dx, 16, Math::IMUL); // 3fad: imul dx
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3faf: add bx, ax
step(); ax=uint16_t(read16(ds, uint16_t(134))); // 3fb1: mov ax, word ptr [0x86]
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3fb4: shl ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,1)); // 3fb6: shl ax, 1
step(); bx=uint16_t(alu(bx,ax,16,Op::ADD)); // 3fb8: add bx, ax
step(); alu(read8(es, uint16_t(0+bx)),4,8,Op::SUB); // 3fba: cmp byte ptr es:[bx], 4
step(); if(!zf){goto L3ff9;} // 3fbe: jne 0x3ff9
step(); bx=uint16_t(read16(ds, uint16_t(134))); // 3fc0: mov bx, word ptr [0x86]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3fc4: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 3fc6: shl bx, 1
step(); write8(ds, uint16_t(422+bx), 0); // 3fc8: mov byte ptr [bx + 0x1a6], 0
step(); alu(read16(ds, uint16_t(66)),0,16,Op::SUB); // 3fcd: cmp word ptr [0x42], 0
step(); if(!zf){goto L3fdf;} // 3fd2: jne 0x3fdf
step(); push(read16(ds, uint16_t(134))); // 3fd4: push word ptr [0x86]
step(); push(cs); // 3fd8: push cs
step(); push(0x16bc); execute(0x2d75); // 3fd9: call 0x2d75
step(); cx=uint16_t(pop()); // 3fdc: pop cx
step(); goto L3ff9; // 3fdd: jmp 0x3ff9
L3fdf: step(); set_lo(cx,read8(ds, uint16_t(72))); // 3fdf: mov cl, byte ptr [0x48]
step(); set_lo(cx,incdec(lo(cx),8,true)); // 3fe3: dec cl
step(); ax=uint16_t(read16(ds, uint16_t(106))); // 3fe5: mov ax, word ptr [0x6a]
step(); ax=uint16_t(shift(ax,16,Op::SHL,lo(cx))); // 3fe8: shl ax, cl
step(); ax=uint16_t(alu(ax,read16(ds, uint16_t(74)),16,Op::ADD)); // 3fea: add ax, word ptr [0x4a]
step(); push(ax); // 3fee: push ax
step(); push(read16(ds, uint16_t(134))); // 3fef: push word ptr [0x86]
step(); push(cs); // 3ff3: push cs
step(); push(0x16d7); execute(0x3b63); // 3ff4: call 0x3b63
step(); cx=uint16_t(pop()); // 3ff7: pop cx
step(); cx=uint16_t(pop()); // 3ff8: pop cx
L3ff9: step(); write16(ds, uint16_t(134), incdec(read16(ds, uint16_t(134)),16,false)); // 3ff9: inc word ptr [0x86]
L3ffd: step(); alu(read16(ds, uint16_t(134)),11,16,Op::SUB); // 3ffd: cmp word ptr [0x86], 0xb
step(); if(!cf){goto L4007;} // 4002: jae 0x4007
step(); goto L3bed; // 4004: jmp 0x3bed
L4007: step(); write16(ds, uint16_t(74), incdec(read16(ds, uint16_t(74)),16,false)); // 4007: inc word ptr [0x4a]
step(); set_lo(cx,read8(ds, uint16_t(72))); // 400b: mov cl, byte ptr [0x48]
step(); set_lo(cx,incdec(lo(cx),8,true)); // 400f: dec cl
step(); ax=uint16_t(1); // 4011: mov ax, 1
step(); ax=uint16_t(shift(ax,16,Op::SHL,lo(cx))); // 4014: shl ax, cl
step(); alu(ax,read16(ds, uint16_t(74)),16,Op::SUB); // 4016: cmp ax, word ptr [0x4a]
step(); if(!cf&&!zf){goto L4026;} // 401a: ja 0x4026
step(); write16(ds, uint16_t(74), 0); // 401c: mov word ptr [0x4a], 0
step(); write16(ds, uint16_t(106), incdec(read16(ds, uint16_t(106)),16,false)); // 4022: inc word ptr [0x6a]
L4026: step(); bx=uint16_t(read16(ds, uint16_t(358))); // 4026: mov bx, word ptr [0x166]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 402a: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 402c: shl bx, 1
step(); {const auto location=uint16_t(5168+bx); const auto source_segment=ds; bx=uint16_t(read16(source_segment,location)); es=read16(source_segment,uint16_t(location+2));} // 402e: les bx, ptr [bx + 0x1430]
step(); ax=uint16_t(read16(es, uint16_t(3074+bx))); // 4032: mov ax, word ptr es:[bx + 0xc02]
step(); alu(ax,read16(ds, uint16_t(106)),16,Op::SUB); // 4037: cmp ax, word ptr [0x6a]
step(); if(!cf&&!zf){goto L4054;} // 403b: ja 0x4054
step(); write16(ds, uint16_t(106), 0); // 403d: mov word ptr [0x6a], 0
step(); write16(ds, uint16_t(108), incdec(read16(ds, uint16_t(108)),16,false)); // 4043: inc word ptr [0x6c]
step(); alu(read16(ds, uint16_t(108)),64,16,Op::SUB); // 4047: cmp word ptr [0x6c], 0x40
step(); if(cf){goto L4054;} // 404c: jb 0x4054
step(); write16(ds, uint16_t(108), 0); // 404e: mov word ptr [0x6c], 0
L4054: step(); write16(ds, uint16_t(80), 1); // 4054: mov word ptr [0x50], 1
step(); goto L40b4; // 405a: jmp 0x40b4
L405c: step(); push(cs);push(0x1741);cs=uint16_t(4096);adapter(0x1ca4); // 405c: lcall 0x1000, 0x1ca4
step(); write16(ds, uint16_t(134), 0); // 4061: mov word ptr [0x86], 0
step(); goto L4076; // 4067: jmp 0x4076
L4069: step(); push(read16(ds, uint16_t(134))); // 4069: push word ptr [0x86]
step(); push(cs); // 406d: push cs
step(); push(0x1751); execute(0x2d75); // 406e: call 0x2d75
step(); cx=uint16_t(pop()); // 4071: pop cx
step(); write16(ds, uint16_t(134), incdec(read16(ds, uint16_t(134)),16,false)); // 4072: inc word ptr [0x86]
L4076: step(); alu(read16(ds, uint16_t(134)),11,16,Op::SUB); // 4076: cmp word ptr [0x86], 0xb
step(); if(cf){goto L4069;} // 407b: jb 0x4069
step(); api_flags_extra&=uint16_t(~0x200u); // 407d: cli
step(); write16(ds, uint16_t(382), 9480); // 407e: mov word ptr [0x17e], 0x2508
step(); ax=uint16_t(read16(ds, uint16_t(364))); // 4084: mov ax, word ptr [0x16c]
step(); write16(ds, uint16_t(396), ax); // 4087: mov word ptr [0x18c], ax
step(); ax=uint16_t(read16(ds, uint16_t(362))); // 408a: mov ax, word ptr [0x16a]
step(); write16(ds, uint16_t(388), ax); // 408d: mov word ptr [0x184], ax
step(); push(ds); // 4090: push ds
step(); ax=uint16_t(382); // 4091: mov ax, 0x17e
step(); push(ax); // 4094: push ax
step(); ax=uint16_t(33); // 4095: mov ax, 0x21
step(); push(ax); // 4098: push ax
step(); push(cs);push(0x177e);cs=uint16_t(4096);adapter(0xfb4); // 4099: lcall 0x1000, 0xfb4
step(); sp=uint16_t(alu(sp,6,16,Op::ADD)); // 409e: add sp, 6
step(); api_flags_extra&=uint16_t(~0x200u); // 40a1: cli
step(); write16(ds, uint16_t(112), 0); // 40a2: mov word ptr [0x70], 0
step(); write16(ds, uint16_t(80), 0); // 40a8: mov word ptr [0x50], 0
step(); write16(ds, uint16_t(108), 65); // 40ae: mov word ptr [0x6c], 0x41
L40b4: step(); alu(read16(ds, uint16_t(414)),0,16,Op::SUB); // 40b4: cmp word ptr [0x19e], 0
step(); if(!zf){goto L40be;} // 40b9: jne 0x40be
step(); return; // 40bb: jmp 0x4146
L40be: step(); alu(read16(ds, uint16_t(378)),0,16,Op::SUB); // 40be: cmp word ptr [0x17a], 0
step(); if(zf){goto L40df;} // 40c3: je 0x40df
step(); ax=uint16_t(read16(ds, uint16_t(414))); // 40c5: mov ax, word ptr [0x19e]
step(); alu(ax,read16(ds, uint16_t(416)),16,Op::SUB); // 40c8: cmp ax, word ptr [0x1a0]
step(); if(cf){goto L40d7;} // 40cc: jb 0x40d7
step(); ax=uint16_t(read16(ds, uint16_t(416))); // 40ce: mov ax, word ptr [0x1a0]
step(); write16(ds, uint16_t(414), alu(read16(ds, uint16_t(414)),ax,16,Op::SUB)); // 40d1: sub word ptr [0x19e], ax
step(); goto L40dd; // 40d5: jmp 0x40dd
L40d7: step(); write16(ds, uint16_t(414), 0); // 40d7: mov word ptr [0x19e], 0
L40dd: step(); goto L40e6; // 40dd: jmp 0x40e6
L40df: step(); ax=uint16_t(read16(ds, uint16_t(416))); // 40df: mov ax, word ptr [0x1a0]
step(); write16(ds, uint16_t(414), alu(read16(ds, uint16_t(414)),ax,16,Op::ADD)); // 40e2: add word ptr [0x19e], ax
L40e6: step(); alu(read16(ds, uint16_t(414)),128,16,Op::SUB); // 40e6: cmp word ptr [0x19e], 0x80
step(); if(cf){return;} // 40ec: jb 0x4146
step(); push(cs);push(0x17d3);cs=uint16_t(4096);adapter(0x1ca4); // 40ee: lcall 0x1000, 0x1ca4
step(); write16(ds, uint16_t(134), 0); // 40f3: mov word ptr [0x86], 0
step(); goto L4108; // 40f9: jmp 0x4108
L40fb: step(); push(read16(ds, uint16_t(134))); // 40fb: push word ptr [0x86]
step(); push(cs); // 40ff: push cs
step(); push(0x17e3); execute(0x2d75); // 4100: call 0x2d75
step(); cx=uint16_t(pop()); // 4103: pop cx
step(); write16(ds, uint16_t(134), incdec(read16(ds, uint16_t(134)),16,false)); // 4104: inc word ptr [0x86]
L4108: step(); alu(read16(ds, uint16_t(134)),11,16,Op::SUB); // 4108: cmp word ptr [0x86], 0xb
step(); if(cf){goto L40fb;} // 410d: jb 0x40fb
step(); api_flags_extra&=uint16_t(~0x200u); // 410f: cli
step(); write16(ds, uint16_t(382), 9480); // 4110: mov word ptr [0x17e], 0x2508
step(); ax=uint16_t(read16(ds, uint16_t(364))); // 4116: mov ax, word ptr [0x16c]
step(); write16(ds, uint16_t(396), ax); // 4119: mov word ptr [0x18c], ax
step(); ax=uint16_t(read16(ds, uint16_t(362))); // 411c: mov ax, word ptr [0x16a]
step(); write16(ds, uint16_t(388), ax); // 411f: mov word ptr [0x184], ax
step(); push(ds); // 4122: push ds
step(); ax=uint16_t(382); // 4123: mov ax, 0x17e
step(); push(ax); // 4126: push ax
step(); ax=uint16_t(33); // 4127: mov ax, 0x21
step(); push(ax); // 412a: push ax
step(); push(cs);push(0x1810);cs=uint16_t(4096);adapter(0xfb4); // 412b: lcall 0x1000, 0xfb4
step(); sp=uint16_t(alu(sp,6,16,Op::ADD)); // 4130: add sp, 6
step(); api_flags_extra&=uint16_t(~0x200u); // 4133: cli
step(); write16(ds, uint16_t(112), 0); // 4134: mov word ptr [0x70], 0
step(); write16(ds, uint16_t(80), 0); // 413a: mov word ptr [0x50], 0
step(); write16(ds, uint16_t(108), 65); return; // 4140: mov word ptr [0x6c], 0x41
L4150: step(); push(bp); // 4150: push bp
step(); bp=uint16_t(sp); // 4151: mov bp, sp
step(); sp=uint16_t(incdec(sp,16,true)); // 4153: dec sp
step(); sp=uint16_t(incdec(sp,16,true)); // 4154: dec sp
step(); push(ds); // 4155: push ds
step(); ax=uint16_t(5249); // 4156: mov ax, 0x1481
step(); ds=uint16_t(ax); // 4159: mov ds, ax
step(); write16(ds, uint16_t(80), 1); // 415b: mov word ptr [0x50], 1
step(); write16(ss, uint16_t(-2+bp), 0); // 4161: mov word ptr [bp - 2], 0
step(); goto L419b; // 4166: jmp 0x419b
L4168: step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 4168: mov bx, word ptr [bp - 2]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 416b: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 416d: shl bx, 1
step(); write8(ds, uint16_t(422+bx), 0); // 416f: mov byte ptr [bx + 0x1a6], 0
step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 4174: mov bx, word ptr [bp - 2]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 4177: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 4179: shl bx, 1
step(); write8(ds, uint16_t(423+bx), 0); // 417b: mov byte ptr [bx + 0x1a7], 0
step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 4180: mov bx, word ptr [bp - 2]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 4183: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 4185: shl bx, 1
step(); write8(ds, uint16_t(424+bx), 0); // 4187: mov byte ptr [bx + 0x1a8], 0
step(); bx=uint16_t(read16(ss, uint16_t(-2+bp))); // 418c: mov bx, word ptr [bp - 2]
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 418f: shl bx, 1
step(); bx=uint16_t(shift(bx,16,Op::SHL,1)); // 4191: shl bx, 1
step(); write8(ds, uint16_t(425+bx), 0); // 4193: mov byte ptr [bx + 0x1a9], 0
step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 4198: inc word ptr [bp - 2]
L419b: step(); alu(read16(ss, uint16_t(-2+bp)),11,16,Op::SUB); // 419b: cmp word ptr [bp - 2], 0xb
step(); if(cf){goto L4168;} // 419f: jb 0x4168
step(); push(cs); // 41a1: push cs
step(); push(0x1885); execute(0x2de1); // 41a2: call 0x2de1
step(); api_flags_extra&=uint16_t(~0x200u); // 41a5: cli
step(); ax=uint16_t(8); // 41a6: mov ax, 8
step(); push(ax); // 41a9: push ax
step(); push(cs);push(0x188f);cs=uint16_t(4096);adapter(0xf86); // 41aa: lcall 0x1000, 0xf86
step(); cx=uint16_t(pop()); // 41af: pop cx
step(); write16(ds, uint16_t(364), dx); // 41b0: mov word ptr [0x16c], dx
step(); write16(ds, uint16_t(362), ax); // 41b4: mov word ptr [0x16a], ax
step(); push(cs); // 41b7: push cs
step(); ax=uint16_t(4706); // 41b8: mov ax, 0x1262
step(); push(ax); // 41bb: push ax
step(); ax=uint16_t(8); // 41bc: mov ax, 8
step(); push(ax); // 41bf: push ax
step(); push(cs);push(0x18a5);cs=uint16_t(4096);adapter(0xf9c); // 41c0: lcall 0x1000, 0xf9c
step(); sp=uint16_t(alu(sp,6,16,Op::ADD)); // 41c5: add sp, 6
step(); write16(ss, uint16_t(-2+bp), 0); // 41c8: mov word ptr [bp - 2], 0
step(); goto L41da; // 41cd: jmp 0x41da
L41cf: step(); push(read16(ss, uint16_t(-2+bp))); // 41cf: push word ptr [bp - 2]
step(); push(cs); // 41d2: push cs
step(); push(0x18b6); execute(0x2d75); // 41d3: call 0x2d75
step(); cx=uint16_t(pop()); // 41d6: pop cx
step(); write16(ss, uint16_t(-2+bp), incdec(read16(ss, uint16_t(-2+bp)),16,false)); // 41d7: inc word ptr [bp - 2]
L41da: step(); alu(read16(ss, uint16_t(-2+bp)),11,16,Op::SUB); // 41da: cmp word ptr [bp - 2], 0xb
step(); if(cf){goto L41cf;} // 41de: jb 0x41cf
step(); ax=uint16_t(alu(ax,ax,16,Op::XOR)); // 41e0: xor ax, ax
step(); write16(ds, uint16_t(106), ax); // 41e2: mov word ptr [0x6a], ax
step(); write16(ds, uint16_t(108), ax); // 41e5: mov word ptr [0x6c], ax
step(); api_flags_extra|=0x200; // 41e8: sti
step(); ds=uint16_t(pop()); // 41e9: pop ds
step(); sp=uint16_t(bp); // 41ea: mov sp, bp
step(); bp=uint16_t(pop()); // 41ec: pop bp
step(); pop();cs=pop();return; // 41ed: retf
}
