import { exit } from "node:process";
import { encode, Rect, RectHeader } from "./encode.js";
import { rectFileFromImage, openImage } from "./fromimg.js";
import { stat, writeFile } from "node:fs/promises";

const test: RectHeader = {
	flags: 0xABCD,
	texSize: { x: 512, y: 1024 },
	rects: [
		{ flags: 0x12, mins: { x: 0, y: 0 }, maxs: { x: 128, y: 128 } },
		{ flags: 0x34, mins: { x: 128, y: 0 }, maxs: { x: 512, y: 128 } },
		{ flags: 0x56, mins: { x: 0, y: 128 }, maxs: { x: 512, y: 512 } },
		{ flags: 0x78, mins: { x: 0, y: 512 }, maxs: { x: 512, y: 1024 } },
	]
};

function getCurrentPM() {
	return process.env.npm_execpath.match(/(?:\/|\\)(\w+)[\w-]*?\.\w+$/)[1];
}

function printUsage() {
	const pm = getCurrentPM();
	console.error(`\nUsage: ${pm} run generate <imagePath>\n`);
}

async function main(args: string[]) {
	console.log('"'+process.env.npm_execpath+'"');
	if (args[0].endsWith('node.exe')) args = args.slice(1);
	if (args.length < 2) {
		printUsage();
		exit(1);
	}

	const fp = args[1];
	try {
		const fstat = await stat(fp);
		if (!fstat.isFile) {
			console.error('Expected <filePath> to be a file!');
			printUsage();
			exit(1);
		}
	}
	catch (e) {
		console.error('Could not read file! Does it exist?');
		exit(1);
	}

	const img = await openImage(fp);
	const data = rectFileFromImage(img);
	const encoded = encode(data);
	writeFile(fp.replace(/\..+?$/, '.rect'), new Uint8Array(encoded));

	console.log('Converted rectmap image successfully!');
	exit(0);
}

main(process.argv);
