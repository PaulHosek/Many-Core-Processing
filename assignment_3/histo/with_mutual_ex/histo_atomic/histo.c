#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <omp.h>

void die(const char *msg){
    if (errno != 0) 
        perror(msg);
    else
        fprintf(stderr, "error: %s\n", msg);
    exit(1);
}   

void generate_image(int num_rows, int num_cols, int * image){
    for (int i = 0; i < num_cols * num_rows; ++i)
    {
        image[i] = rand() % 256; //255 + 1 for num bins
    }
}

void read_image(const char * image_path, int num_rows, int num_cols, int * image){
	char format[3];
    FILE *f;
    unsigned imgw, imgh, maxv, v;
    size_t i;

	printf("Reading PGM data from %s...\n", image_path);

	if (!(f = fopen(image_path, "r"))) die("fopen");

	fscanf(f, "%2s", format);
    if (format[0] != 'P' || format[1] != '2') die("only ASCII PGM input is supported");
    
    if (fscanf(f, "%u", &imgw) != 1 ||
        fscanf(f, "%u", &imgh) != 1 ||
        fscanf(f, "%u", &maxv) != 1) die("invalid input");

    if (imgw != num_cols || imgh != num_rows) {
        fprintf(stderr, "input data size (%ux%u) does not match cylinder size (%zux%zu)\n",
                imgw, imgh, num_cols, num_rows);
        die("invalid input");
    }

    for (i = 0; i < num_cols * num_rows; ++i)
    {
        if (fscanf(f, "%u", &v) != 1) die("invalid data");
        image[i] = ((int)v * 255) / maxv; //255 for num bins
    }
    fclose(f);
}

void print_histo(int * histo){
	for (int i = 0; i < 256; ++i)
	{	
		if(i != 0 && (i % 10 == 0)) {
            printf("\n");
        }
		printf("%d ", histo[i]);
	}
    printf("\n");
}

void print_image(int num_rows, int num_cols, int * image){
	int index = 0;
	for (int i = 0; i < num_rows; ++i){	
		for (int j = 0; j < num_cols; ++j){
	        index = i * num_cols + j;
			printf("%d ", image[index]);
		}
	// TODO: put back print out of the loop! Just for debugging 
    printf("\n");
	}
}

void calculate_partition(int nrows, int nthreads, int * partition_sizes, int * offsets){
    /*Calculate how many rows each thread gets such that the differences are minimized. Return an array of size nthreads 
    where each entry is the number of rows assigned to this thread. Also return array of size nthreads where each entry is the 
    offset index for this thread. */
    int i;
    if (nrows % nthreads == 0){
        // rows can be evenly distributed
        for (i = 0; i < nthreads; ++i){
            partition_sizes[i] = nrows/nthreads;
            offsets[i] = i * partition_sizes[i]; 
        }
    } else {
        // nlower processors will get floor(nrows/nthreads) rows
        int nlower = nthreads - (nrows % nthreads); 
        // implicit calculation of floor(nrows/nthreads) because C truncates the decimals 
        int floor = nrows/nthreads ;
        for (i = 0; i < nthreads; ++i){
            if (i < nlower){
                partition_sizes[i] = floor;
            } else {
                partition_sizes[i] = floor + 1;
            }
            if (i > 0){
                offsets[i] = offsets[i-1] + partition_sizes[i-1];
            } else {
                offsets[i] = 0;
            }
        }
    }
}

void histogram(int * histo, int * image, int ndata){
    int sum[256] = {0};
    for (int j = 0; j < ndata; j ++){
        sum[image[j]] ++;
    }
    for (int i = 0; i < 256; i++){
        if (sum[i] > 0){
            #pragma omp atomic
            histo[i] += sum[i];
        }
    }
} 


int main(int argc, char *argv[]){
    int c;
    int seed = 42;
    const char *image_path = 0;
    image_path ="../../../../images/pat1_100x150.pgm";
    int gen_image = 0;
    int debug = 0;

    int num_rows = 150;
    int num_cols = 100;
    int num_threads = 1;

    struct timespec before, after;

    int * histo = (int *) calloc(256, sizeof(int));

    /* Read command-line options. */
    while((c = getopt(argc, argv, "s:i:rp:n:m:g")) != -1) {
        switch(c) {
            case 's':
                seed = atoi(optarg);
                break;
            case 'i':
            	image_path = optarg;
            	break;
            case 'r':
            	gen_image = 1;
            	break;
            case 'p':
                num_threads = atoi(optarg);
                break;
            case 'n':
            	num_rows = strtol(optarg, 0, 10);
            	break;
            case 'm':
				num_cols = strtol(optarg, 0, 10);
				break;
			case 'g':
				debug = 1;
				break;
            case '?':
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return -1;
            default:
                return -1;
        }
    }

    int * image = (int *) malloc(sizeof(int) * num_cols * num_rows);

    /* Seed such that we can always reproduce the same random vector */
    if (gen_image){
    	srand(seed);
    	generate_image(num_rows, num_cols, image);
    }else{
    	read_image(image_path,num_rows, num_cols, image);
    }

    //print_image(num_rows, num_cols, image);

    int * partition_sizes = (int * ) malloc(sizeof(int) * num_threads);
    int * offsets = (int * ) malloc(sizeof(int) * num_threads);
    calculate_partition(num_rows, num_threads, partition_sizes, offsets);

    clock_gettime(CLOCK_MONOTONIC, &before);
    
    omp_set_num_threads(num_threads);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int offset = offsets[id] * num_cols;
        histogram(histo, &image[offset], partition_sizes[id] * num_cols);
    }

    /* Do your thing here */

    if (debug){
    	print_histo(histo);
    }

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;


    printf("Histo took: % .6e seconds \n", time);

    FILE *fpt;
    fpt = fopen("data_atomic.csv", "a+");
    fprintf(fpt,"atomic, sectioned, updateafterloop, %d, %d, %d, %d, % .6e \n", debug, num_rows, num_cols, num_threads, time);
    fclose(fpt);

    free(histo);
    free(image);
    free(partition_sizes);
    free(offsets);
}
