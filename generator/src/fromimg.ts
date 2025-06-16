import sharp from "sharp";
import { Rect, RectHeader } from "./encode.js";

interface ImageData {
	width: number;
	height: number;
	data: Uint8Array;
}

export async function openImage(path: string): Promise<ImageData> {
	const { data, info } = await sharp(path).ensureAlpha().raw({ depth: 'uchar' }).toBuffer({ resolveWithObject: true });
	if (info.channels !== 4) throw Error(`got ${info.channels} channels!?`);
	return {
		data,
		width: info.width,
		height: info.height
	}
}

function splitsFromImage(img: ImageData): Rect[] {
	const hashColor = (r: number, g: number, b: number) => (r | (g << 8) | (b << 16)).toString();
	const data = img.data;
	const rects: Record<string, Rect> = {};

	let i = 0;
	for (let y=0; y<img.height; y++) {
		for (let x=0; x<img.width; x++, i+=4) {
			const hash = hashColor(data[i], data[i+1], data[i+2]);
			if (hash in rects) {
				const rect = rects[hash];
				if (x < rect.mins.x) rect.mins.x = x;
				if (y < rect.mins.y) rect.mins.y = y;
				if (x >= rect.maxs.x) rect.maxs.x = x+1;
				if (y >= rect.maxs.y) rect.maxs.y = y+1;
			}
			else {
				rects[hash] = { flags: 0, mins: { x, y }, maxs: { x, y } };
			}
		}
	}

	return Object.values(rects);
}

export function rectFileFromImage(img: ImageData): RectHeader {
	const rects = splitsFromImage(img);
	return {
		flags: 0,
		texSize: { x: img.width, y: img.height },
		rects,
	}
}
