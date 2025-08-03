from struct import Struct
from typing import NamedTuple
from enum import IntFlag
from math import inf

S_HotSpotHeader = Struct('<BBH')
S_HotSpotRect = Struct('<BHHHH')

def linejoin(*lines: str, indent: str=''):
	return '\n'.join([indent + l for l in lines])

class BBox():
	min_x: float = inf
	min_y: float = inf
	max_x: float = -inf
	max_y: float = -inf

	def extend(self, x: float, y: float):
		if x < self.min_x: self.min_x = x
		if y < self.min_y: self.min_y = y
		if x > self.max_x: self.max_x = x
		if y > self.max_y: self.max_y = y

	def to_rect(self, flags: int, res_x: int, res_y: int):
		return HotSpotRect(flags,
			round(self.min_x * res_x),
			round((1.0 - self.max_y) * res_y),
			round(self.max_x * res_x),
			round((1.0 - self.min_y) * res_y),
		)

class HotSpotFlags(IntFlag):
	Rotation   = 0x1
	Reflection = 0x2
	AltGroup   = 0x4

class HotSpotRect(NamedTuple):
	flags: int
	min_x: int
	min_y: int
	max_x: int
	max_y: int

	def pack_into(self, bytes: bytearray, idx: int):
		S_HotSpotRect.pack_into(
			bytes, idx, self.flags,
			self.min_x, self.min_y,
			self.max_x, self.max_y)
		
	def to_string(self):
		flags: list[str] = []
		if self.flags & HotSpotFlags.Rotation:    flags.append('\trotate 1')
		if self.flags & HotSpotFlags.Reflection:  flags.append('\treflect 1')
		if self.flags & HotSpotFlags.AltGroup:    flags.append('\talt 1')

		return linejoin(
			'rectangle',
			'{',
			f'\tmin "{self.min_x} {self.min_y}"',
			f'\tmax "{self.max_x} {self.max_y}"',
			*flags,
			'}'
		, indent='\t')

class HotSpotFile(NamedTuple):
	rects: list[HotSpotRect]

	def pack(self):
		length = 4 + 9 * len(self.rects)
		data = bytearray(length)
		S_HotSpotHeader.pack_into(data, 0,
			0x1, 0x0,
			len(self.rects))

		idx = 4
		for rect in self.rects:
			rect.pack_into(data, idx)
			idx += 9

		return data
	
	def to_string(self):
		return linejoin(
			'Rectangles',
			'{',
			*[rect.to_string() for rect in self.rects],
			'}'
		)
