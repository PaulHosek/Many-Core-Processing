# Adapted from https://stackoverflow.com/questions/40082165/python-create-pgm-file

import numpy as np
import sys, getopt
import os.path

def main(argv):
    outputfile = "./output.pgm"
    columns = 100
    rows = 100
    max_value = 65535
    try:
      opts, args = getopt.getopt(argv,"ho:m:n:v:",["help","output=","columns=","rows=", "max_value="])
    except getopt.GetoptError:
      print ('Usage gpm_generator.py -o <outputfile> -m <columns> -n <rows> -v <max_value>')
      sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
         print ("Generates a random plain portable greymap image (.pgm)")
         print ('Parameters:\n')
         print ("-h, --help")
         print ("-o, --output=[.]")
         print ("-m, --columns=[100]")
         print ("-n, --rows=[100]")
         print ("-v, --max_value=[65535]")
         sys.exit()
        elif opt in ("-o", "--output"):
         outputfile = arg
        elif opt in ("-m", "--columns"):
         columns = int(arg)
        elif opt in ("-n", "--rows"):
         rows = int(arg)
        elif opt in ("-v", "--max_value"):
         max_value = int(arg)
    
    if (outputfile.split(".")[-1] != "pgm"):
        print ('Invalid path of output file')
        sys.exit(3)

    p_num = columns * rows
    arr = np.random.randint(0,max_value,p_num)

    # open file for writing 
    fout=open(outputfile, 'wb')

    # define PGM Header
    pgmHeader = 'P2' + ' ' + str(columns) + ' ' + str(rows) + ' ' + str(max_value) +  '\n'

    pgmHeader_byte = bytearray(pgmHeader,'utf-8')

    # write the header to the file
    fout.write(pgmHeader_byte)

    # write the data to the file 
    img = np.reshape(arr,(rows,columns))

    for j in range(rows):
        bnd = list(img[j,:])
        bnd_str = np.char.mod('%d',bnd)
        bnd_str = np.append(bnd_str,'\n')
        bnd_str = [' '.join(bnd_str)][0]    
        bnd_byte = bytearray(bnd_str,'utf-8')        
        fout.write(bnd_byte)

    fout.close()
   
if __name__ == "__main__":
   main(sys.argv[1:])
