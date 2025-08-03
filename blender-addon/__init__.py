from . import src

bl_info = {
	'name': 'Source HotSpot',
    'author': 'Koerismo',
    'description': 'Export Hotspot resources from Blender',
    'blender': (4, 0, 0),
    'version': (0, 3, 0),
    'location': '3D View > Sidebar',
    'category': 'Import-Export'
}

def register():
    src.register()

def unregister():
    src.unregister()
