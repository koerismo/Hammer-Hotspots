import sharp from "sharp";
import { RectFlags, type Rect, type RectHeader } from "./encode.ts";

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

function splitsFromImage(img: ImageData, baseFlags: number, uva: boolean): Rect[] {
	const isAltGroup = (b: number) => (uva && (b >= 128));
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
				let flags = baseFlags;
				if (isAltGroup(data[i+2])) flags |= RectFlags.AltGroup;
				rects[hash] = { flags, mins: { x, y }, maxs: { x, y } };
			}
		}
	}

	return Object.values(rects).sort((a, b) => {
		if (a.mins.y !== b.mins.y) return a.mins.y - b.mins.y;
		return a.mins.x - b.mins.x;
	});
}

export function rectFileFromImage(img: ImageData, baseFlags: number, uva: boolean): RectHeader {
	const rects = splitsFromImage(img, baseFlags, uva);
	return {
		flags: 0,
		texSize: { x: img.width, y: img.height },
		rects,
	}
}
