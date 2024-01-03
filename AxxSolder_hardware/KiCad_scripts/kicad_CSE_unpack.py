import zipfile
import os
import shutil

def remove(path):
    """ param <path> could either be relative or absolute. """
    if os.path.isfile(path) or os.path.islink(path):
        os.remove(path)  # remove the file
    elif os.path.isdir(path):
        shutil.rmtree(path)  # remove dir and all contains
    else:
        raise ValueError("file {} is not a file or dir.".format(path))

currentpath = os.getcwd()
zipfolders = os.getcwd() + "/componentsearchengine_ZIP_archive"

tempfolderpath = os.path.join(currentpath, "3D-models")
if not os.path.exists(tempfolderpath):
    os.makedirs(tempfolderpath)

tempfolderpath = os.path.join(currentpath, "footprints.pretty")
if not os.path.exists(tempfolderpath):
    os.makedirs(tempfolderpath)

tempfolderpath = os.path.join(currentpath,"schematic-symbols") 
if not os.path.exists(tempfolderpath):
    os.makedirs(tempfolderpath)

# make temp folder
tempfolderpath = os.path.join(currentpath, "temp")
os.makedirs(tempfolderpath, exist_ok=True)
print("temp directory created %s" % tempfolderpath)

for filename in os.listdir(zipfolders):
    f = os.path.join(zipfolders, filename)
    # checking if it is a file
    if f.endswith('.zip'):

        with zipfile.ZipFile(f, 'r') as zip_ref:
            zip_ref.extractall(tempfolderpath)

            if(1==1):#try:
                file = f.split('LIB_')[1].strip(".zip")+"/3D"
                print(file)
                for filename in os.listdir(tempfolderpath + "/" + file):
                    if not os.path.isfile(currentpath + "/3D-models/" + filename):
                        os.replace(os.path.join(tempfolderpath + "/" + file, filename), currentpath + "/3D-models/" + filename)
                        print("moving file %s" % filename)
                    else:
                        print("%s did already exist!" % filename)
                
                file = f.split('LIB_')[1].strip(".zip")+"/KiCad"
                for filename in os.listdir(tempfolderpath + "/" + file):
                    if (filename.endswith('.lib') or filename.endswith('.kicad_sym')):
                        if not os.path.isfile(currentpath + "/schematic-symbols/" + filename):
                            os.replace(os.path.join(tempfolderpath + "/" + file, filename), currentpath + "/schematic-symbols/" + filename)
                            print("moving file %s" % filename)
                        else:
                            print("%s did already exist!" % filename)
                        
                for filename in os.listdir(tempfolderpath + "/" + file):
                    if filename.endswith('.kicad_mod') or filename.endswith('.mod'):
                        if not os.path.isfile(currentpath + "/footprints.pretty/" + filename):
                            os.replace(os.path.join(tempfolderpath + "/" + file, filename), currentpath + "/footprints.pretty/" + filename)
                            print("moving file %s" % filename)
                        else:
                            print("%s did already exist!" % filename)
            #except Exception as e: print(e)
remove(tempfolderpath)
print("temp directory removed %s" % tempfolderpath)
