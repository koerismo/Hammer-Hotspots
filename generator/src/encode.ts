import { ViewBuffer } from "stupid-buffer";

export interface Vec2 {
	x: number;
	y: number;
}

function write_v2(b: ViewBuffer, v2: Vec2) {
	b.write_u16(v2.x);
	b.write_u16(v2.y);
}

export interface Rect {
	flags: number;
	mins: Vec2;
	maxs: Vec2;
}

function write_rect(b: ViewBuffer, rect: Rect) {
	b.write_u8(rect.flags);
	write_v2(b, rect.mins); 
	write_v2(b, rect.maxs); 
}

export const RectFlags = {
	AllowRotation: 0x1,
	AllowReflection: 0x2,
	AltGroup: 0x4,
} as const;

export interface RectHeader {
	flags: number;
	texSize: Vec2;
	rects: Rect[];
}

function write_file(b: ViewBuffer, file: RectHeader) {
	b.write_u8(1);
	b.write_u8(file.flags);
	b.write_u16(file.rects.length);

	for (let i=0; i<file.rects.length; i++) {
		write_rect(b, file.rects[i]);
	}
}

function est_file(file: RectHeader) {
	const HEAD_SIZE = 4;
	const RECT_SIZE = 9;
	return HEAD_SIZE + file.rects.length * RECT_SIZE;
}

export function encode(file: RectHeader): ArrayBuffer {
	const view = new ViewBuffer(est_file(file));
	view.set_endian(true);
	write_file(view, file);
	return view.buffer;
}
