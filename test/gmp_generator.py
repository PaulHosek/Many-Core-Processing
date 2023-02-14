# See https://stackoverflow.com/questions/40082165/python-create-pgm-file

import numpy as np
import os

def create_pmg(width, height):
    p_num = width * height
    arr = np.random.randint(0,255,p_num)

    filename = 'RectangularM=' + str(width) + 'N=' + str(height) + '.pgm'

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
    final_directory_squares = os.path.join(current_directory, r'squares')
    final_directory_wide_rect = os.path.join(current_directory, r'wide_rect')
    final_directory_thin_rect = os.path.join(current_directory, r'thin_rect')

    if not os.path.exists(final_directory_squares):
        os.makedirs(final_directory_squares)

    if not os.path.exists(final_directory_wide_rect):
        os.makedirs(final_directory_wide_rect)

    if not os.path.exists(final_directory_thin_rect):
        os.makedirs(final_directory_thin_rect)
    
    os.chdir(current_directory + '/squares')
    
    #Set one of parameters
    M = [50, 75, 110, 200, 500, 800, 1000, 1300, 1600, 2000]
    N = [50, 75, 110, 200, 500, 800, 1000, 1300, 1600, 2000]

    for i in range(0, len(M)):
        create_pmg(N[i], M[i])

    os.chdir(current_directory + '/wide_rect')
    #Set two of paramters
    M = [50, 75, 110, 200, 500, 800, 1000, 1300, 1600, 2000]
    N = [100, 100, 100, 100, 100, 100, 100, 100, 100, 100]

    for i in range(0, len(M)):
        create_pmg(N[i], M[i])

    os.chdir(current_directory + '/thin_rect')
    #Set three of parameters
    N = [50, 75, 110, 200, 500, 800, 1000, 1300, 1600, 2000]
    M = [100, 100, 100, 100, 100, 100, 100, 100, 100, 100]

    for i in range(0, len(M)):
        create_pmg(N[i], M[i])

