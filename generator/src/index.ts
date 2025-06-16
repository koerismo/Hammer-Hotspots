import { encode, Rect, RectHeader } from "./encode.js";
import { writeFile } from "node:fs/promises";

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

const buf = encode(test);
writeFile('./test.rect', new Uint8Array(buf));