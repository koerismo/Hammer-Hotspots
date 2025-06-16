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
	b.write_u16(rect.flags);
	write_v2(b, rect.mins); 
	write_v2(b, rect.maxs); 
}

export interface RectHeader {
	flags: number;
	texSize: Vec2;
	rects: Rect[];
}

function write_file(b: ViewBuffer, file: RectHeader) {
	b.write_str('RECT', 4);
	b.write_u16(file.flags);
	write_v2(b, file.texSize);
	b.write_u16(file.rects.length);
	b.write_u16(b.pointer + 2);

	for (let i=0; i<file.rects.length; i++) {
		write_rect(b, file.rects[i]);
	}
}

function est_file(file: RectHeader) {
	const HEAD_SIZE = 14;
	const RECT_SIZE = 10;
	return HEAD_SIZE + file.rects.length * RECT_SIZE;
}

export function encode(file: RectHeader): ArrayBuffer {
	const view = new ViewBuffer(est_file(file));
	view.set_endian(true);
	write_file(view, file);
	return view.buffer;
}
