import zipfile
import os
import shutil

folder_structure = [['3rd-parties-libraries', '3D-models', 'componentsearchengine_ZIP_archive', 'footprints.pretty', 'schematic-symbols'], ['board-dimensions'], ['bom'], ['datasheets'], ['fabrication'], ['images'], ['logos'], ['pcb_render'], ['schematic_pdf']]

currentpath = os.path.dirname(os.getcwd())

for folders in folder_structure:
	tempfolderpath = os.path.join(currentpath, folders[0])	
	if not os.path.exists(tempfolderpath):
		os.makedirs(tempfolderpath)
	if len(folders) > 1:
		for sub_folders in folders:
			tempsubfolderpath = os.path.join(tempfolderpath, sub_folders)
			if not os.path.exists(tempsubfolderpath):
				os.makedirs(tempsubfolderpath)
				
if not os.path.isfile(currentpath + '/3rd-parties-libraries' + '/kicad_CSE_unpack.py'):
                        shutil.copyfile(os.path.join(os.getcwd()) + '/kicad_CSE_unpack.py', currentpath + '/3rd-parties-libraries' + '/kicad_CSE_unpack.py')
