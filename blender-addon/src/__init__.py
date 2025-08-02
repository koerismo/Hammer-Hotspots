import bpy
import bmesh
from bpy.types import Context, Event
from bpy.props import IntVectorProperty, BoolProperty
from bpy_extras.io_utils import ExportHelper
from typing import Literal, Any
from . import hotlib as H

HOTSPOT_ATTRIBUTE = 'hotspot_flags'

def get_mesh(obj: bpy.types.Object | None) -> bpy.types.Mesh | None:
	if obj == None or obj.type != 'MESH' or obj.data == None: return None
	return obj.data # type: ignore

def get_object_attr(mesh: bpy.types.Mesh, can_create=False):
	data = mesh.attributes.get(HOTSPOT_ATTRIBUTE, None)
	if can_create and data is None:
		data = mesh.attributes.new(HOTSPOT_ATTRIBUTE, 'INT', 'FACE')
	return data

def to_int(num: float, res: int) -> int:
	if num <= 0: return 0
	if num >= 1: return res
	return round(num * res)

class HotSpot_MainPanel(bpy.types.Panel):
	bl_idname = 'SHOTSPOT_PT_MainPanel'
	bl_label = 'Source HotSpot'
	bl_space_type = 'VIEW_3D'
	bl_region_type = 'UI'
	bl_category = 'Item'

	def draw(self, context: bpy.types.Context):
		layout = self.layout
		if layout is None: return

		current_mesh = get_mesh(context.object)
		layout.enabled = current_mesh != None
		layout.use_property_split = True

		layout.prop(current_mesh, 'hs_dimensions')
		layout.operator(HotSpot_SetFlagsOp.bl_idname)
		layout.operator(HotSpot_ExportOp.bl_idname)

class HotSpot_SetFlagsOp(bpy.types.Operator):
	bl_idname = 'shotspot.setflags'
	bl_label = 'Set Face Flags'
	bl_description = 'Sets the flags of the selected faces.'
	bl_options = { 'REGISTER', 'UNDO' }

	hs_flag_rotate:   bpy.props.BoolProperty(name='Rotation', description='Enables random rotation on this region.', options=set()) # type: ignore
	hs_flag_reflect:  bpy.props.BoolProperty(name='Reflection', description='Enables random reflection on this region.', options=set()) # type: ignore
	hs_flag_altgroup: bpy.props.BoolProperty(name='Alt-Group', description='Makes this region use the alt-group.', options=set()) # type: ignore

	@classmethod
	def poll(cls, context: Context) -> bool:
		if context.mode != 'EDIT_MESH': return False
		if context.edit_object == None: return False
		return True

	def invoke(self, context: Context, event: Event):
		wm = context.window_manager
		assert wm
		return wm.invoke_props_dialog(self)

	def execute(self, context: Context) -> set[Literal['RUNNING_MODAL', 'CANCELLED', 'FINISHED', 'PASS_THROUGH', 'INTERFACE']]:
		mesh = get_mesh(context.edit_object)
		if mesh == None: return { 'CANCELLED' }
		bm = bmesh.from_edit_mesh(mesh)

		attr: Any = get_object_attr(mesh, can_create=True)
		assert attr, 'Failed to create hotspot_flags attribute!'

		layer = bm.faces.layers.int.get(HOTSPOT_ATTRIBUTE)
		assert layer, 'Failed to get hotspot_flags data layer from mesh!'

		face_flags = 0x00
		if self.hs_flag_rotate:   face_flags |= H.HotSpotFlags.Rotation
		if self.hs_flag_reflect:  face_flags |= H.HotSpotFlags.Reflection
		if self.hs_flag_altgroup: face_flags |= H.HotSpotFlags.AltGroup

		count = 0
		for face in bm.faces:
			if not face.select: continue
			face[layer] = face_flags
			count += 1

		bmesh.update_edit_mesh(mesh)
		bm.free()

		self.report({ 'INFO' }, f'Set flags for {count} faces!')
		return { 'FINISHED' }

class HotSpot_ExportOp(bpy.types.Operator, ExportHelper):
	bl_idname = 'shotspot.export'
	bl_label = 'Export'
	bl_description = 'Exports the current UV map to a hotspot definition file.'

	filename_ext = '.rect'
	filter_glob: bpy.props.StringProperty(default='*.rect', options={ 'HIDDEN' })

	use_text_format: BoolProperty(name='Use Text Format', description='Export to text-based format?', default=True)

	@classmethod
	def poll(cls, context: Context) -> bool:
		if context.mode != 'OBJECT': return False
		return get_mesh(context.object) != None

	# def invoke(self, context: Context, event: Event) -> set:
	# 	assert context.active_object
	# 	if not self.filename:
	# 		self.filename = context.active_object.name + '.rect'

	# 	wm = context.window_manager
	# 	assert wm
	# 	wm.fileselect_add(self)
	# 	return { 'RUNNING_MODAL' }

	def execute(self, context: Context) -> set[Literal['RUNNING_MODAL', 'CANCELLED', 'FINISHED', 'PASS_THROUGH', 'INTERFACE']]:
		mesh = get_mesh(context.active_object)
		if mesh == None: return { 'CANCELLED' }

		attr = get_object_attr(mesh)
		res_x, res_y = mesh.hs_dimensions # type: ignore

		uv_layer = mesh.uv_layers.active
		if uv_layer == None:
			self.report({ 'ERROR' }, 'No active UV layer!')
			return { 'CANCELLED' }

		uvs = uv_layer.uv
		regions: list[H.BBox] = [H.BBox() for x in range(len(mesh.polygons))]

		# Get the UV bounds of each face

		# Referenced from
		# https://blender.stackexchange.com/a/309846

		for loop_triangle in mesh.loop_triangles:
			poly_idx = loop_triangle.polygon_index
			poly_bbox = regions[poly_idx]

			for i in range(3):
				loop_vert_idx = loop_triangle.loops[i]
				loop_vert_uvs = uvs[loop_vert_idx].vector
				poly_bbox.extend(loop_vert_uvs.x, loop_vert_uvs.y)

		# Convert uv-space regions to integer rects

		rects: list[H.HotSpotRect] = []
		for i in range(len(mesh.polygons)):
			face_flags = attr.data[i].value if attr else 0x0 # type: ignore
			rects.append(regions[i].to_rect(face_flags, res_x, res_y))

		out = H.HotSpotFile(rects)
		if self.use_text_format:
			with open(self.filepath, 'w') as f:
				f.write(out.as_text())
		else:
			with open(self.filepath, 'wb') as f:
				f.write(out.pack())
			

		self.report({ 'INFO' }, f'Wrote {len(regions)} rects to resource!')
		return { 'FINISHED' }

def face_edit_menu_func(self: bpy.types.Panel, context: Context):
	assert self.layout
	self.layout.separator()
	self.layout.operator(HotSpot_SetFlagsOp.bl_idname, text='Set HotSpot Flags')

def register():
	bpy.types.Mesh.hs_dimensions = bpy.props.IntVectorProperty(name='Dimensions', size=2, options=set(), min=8, max=16384, default=(1024, 1024)) # type: ignore
	bpy.utils.register_class(HotSpot_MainPanel)
	bpy.utils.register_class(HotSpot_ExportOp)
	bpy.utils.register_class(HotSpot_SetFlagsOp)
	bpy.types.VIEW3D_MT_edit_mesh_faces.append(face_edit_menu_func)

def unregister():
	bpy.utils.unregister_class(HotSpot_MainPanel)
	bpy.utils.unregister_class(HotSpot_ExportOp)
	bpy.utils.unregister_class(HotSpot_SetFlagsOp)
	bpy.types.VIEW3D_MT_edit_mesh_faces.remove(face_edit_menu_func)

if __name__ == '__main__':
	register()
