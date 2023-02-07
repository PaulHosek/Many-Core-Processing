# See https://stackoverflow.com/questions/40082165/python-create-pgm-file

import numpy as np

# define the width  (columns) and height (rows) of your image
width = 40
height = 40

p_num = width * height
arr = np.random.randint(0,255,p_num)

# open file for writing 
filename = 'test.pgm'
fout=open(filename, 'wb')

# define PGM Header
pgmHeader = 'P2' + ' ' + str(width) + ' ' + str(height) + ' ' + str(255) +  '\n'

pgmHeader_byte = bytearray(pgmHeader,'utf-8')

# write the header to the file
fout.write(pgmHeader)

# write the data to the file 
img = np.reshape(arr,(height,width))

for j in range(height):
    bnd = list(img[j,:])
    bnd_str = np.char.mod('%d',bnd)
    bnd_str = np.append(bnd_str,'\n')
    bnd_str = [' '.join(bnd_str)][0]    
    bnd_byte = bytearray(bnd_str,'utf-8')        
    fout.write(bnd_byte)

fout.close()