import { exit } from "node:process";
import { encode, type Vec2, type Rect, type RectHeader, RectFlags } from "./encode.ts";
import { rectFileFromImage, openImage } from "./fromimg.ts";
import { stat, writeFile } from "node:fs/promises";
import yargs from 'yargs';
import { hideBin } from 'yargs/helpers';

interface Args {
	input: string;
	width: number;
	uva: boolean;
	'flag-rotate': boolean;
	'flag-flip': boolean;
}

function scaleVec(v: Vec2, s: number) {
	v.x *= s;
	v.y *= s;
	const a = (v.x % 1 !== 0) || (v.y % 1 !== 0);
	v.x = Math.trunc(v.x);
	v.y = Math.trunc(v.y);
	return a;
}

function scaleRects(rects: Rect[], scale: number) {
	let oddScale = false;
	for (let i=0; i<rects.length; i++) {
		const r = rects[i];
		if (scaleVec(r.mins, scale)) oddScale ||= true;
		if (scaleVec(r.maxs, scale)) oddScale ||= true;
	}
	return oddScale;
}

function getRectCounts(rects: Rect[]) {
	const count = rects.length;
	let count_alt = 0;
	let count_rot = 0;
	let count_flip = 0;
	for (let i=0; i<count; i++) {
		const r = rects[i];
		if (r.flags & RectFlags.AltGroup) count_alt ++;
		if (r.flags & RectFlags.AllowRotation) count_rot ++;
		if (r.flags & RectFlags.AllowReflection) count_flip ++;
	}
	return {
		count,
		count_alt,
		count_rot,
		count_flip
	}
}

async function main(options: Args) {
	const { input: fp, width: w, uva, "flag-rotate": flagRotate, "flag-flip": flagFlip } = options;
	try {
		const fstat = await stat(fp);
		if (!fstat.isFile) {
			console.error('Expected <filePath> to be a file!');
			exit(1);
		}
	}
	catch (e) {
		console.error('Could not read file! Does it exist?');
		exit(1);
	}

	let baseFlags = 0x00;
	if (flagRotate) baseFlags |= RectFlags.AllowRotation;
	if (flagFlip) baseFlags |= RectFlags.AllowReflection;

	const img = await openImage(fp);
	const data = rectFileFromImage(img, baseFlags, uva);
	if (w) {
		console.log(`Scaling rects ${Math.round(w / img.width * 100) / 100}x...`);
		const oddScale = scaleRects(data.rects, w / img.width);
		if (oddScale) {
			console.warn('One or more regions were rounded to the nearest pixel!');
		}
	}

	const encoded = encode(data);
	await writeFile(fp.replace(/\..+?$/, '.hot'), new Uint8Array(encoded));
	
	const info = getRectCounts(data.rects);
	console.log(`Converted ${data.texSize.x}x${data.texSize.y} colormap image successfully with ${info.count} rects! (${info.count_alt} alternates, ${info.count_rot} rotatable, ${info.count_flip} reflectable)`);
	exit(0);
}

const args: Args = await yargs(hideBin(process.argv))
	.positional('input', { type: 'string', desc: 'The input RGB ID texture.' })
	.demandOption('input')
	.option('width', { type: 'number', alias: 'w', desc: 'Scales rects as if the image were X pixels wide.', default: 0 })
	.option('flag-rotate', { type: 'boolean', desc: 'Enables AllowRotation flag.', default: false })
	.option('flag-flip', { type: 'boolean', desc: 'Enables AllowReflection flag.', default: false })
	.option('uva', { type: 'boolean', alias: 'xya', desc: 'Uses the blue channel (>128) to determine the alt-group.', default: false })
	.parse();

main(args);
