import os
import shutil

def convert_to_utf8(filename, codec):

    # try to open the file and exit if some IOError occurs
    try:
        f = open(filename, 'r').read()
    except Exception:
	return 1

    try:
	data = f.decode(codec)
    except Exception:
    	return 1

    # now get the absolute path of our filename and append _old_ + codec
    # to the end of it (for our backup file)
    fpath = os.path.abspath(filename)
    newfilename = fpath + '_old_'+codec
    # and make our backup file with shutil
    shutil.copy(filename, newfilename)

    # and at last convert it to utf-8
    f = open(filename, 'w')
    try:
        f.write(data.encode('utf-8'))
    except Exception, e:
        print e
    finally:
        f.close()
    return 0

