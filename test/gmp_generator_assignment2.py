# See https://stackoverflow.com/questions/40082165/python-create-pgm-file

import numpy as np
import os

def create_pmg(width, height):
    p_num = width * height
    arr = np.random.randint(0,255,p_num)

    filename = 'ExperimentM=' + str(width) + 'N=' + str(height) + '.pgm'

    # open file for writing 
    fout=open(filename, 'wb')

    # define PGM Header
    pgmHeader = 'P2' + '\n' + str(width) + ' ' + str(height) + '\n' + str(255) +  '\n'

    pgmHeader_byte = bytearray(pgmHeader,'utf-8')

    # write the header to the file
    fout.write(pgmHeader_byte)

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

if __name__ == '__main__':
    # define the width  (columns) and height (rows) of your image
    current_directory = os.getcwd()
    final_directory = os.path.join(current_directory, r'experiments_assignment2')

    if not os.path.exists(final_directory):
        os.makedirs(final_directory)
    
    os.chdir(current_directory + '/experiments_assignment2')
    
    #Set one of parameters
    M = [100, 1000, 100, 20000, 5000]
    N = [100, 1000, 20000, 100, 5000]

    for i in range(0, len(M)):
        create_pmg(N[i], M[i])

