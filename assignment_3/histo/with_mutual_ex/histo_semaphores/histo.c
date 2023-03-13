#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

struct histogram_data {
   int * image;
   int first;
   int last;
   int * histo;
};

sem_t semaphores[256];

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
	}
	printf("\n");
}

void * histogram(void * parameters){  
    struct histogram_data * histo_param = (struct histogram_data *) parameters;
    const int first = histo_param->first;
    const int last = histo_param->last;
    int * image = histo_param->image;
    int * histo = histo_param->histo;
    int index, element;

    for (index=first; index < last; index++)
    {
        element=image[index];
        sem_wait(&semaphores[element]);
        histo[element]++;
        sem_post(&semaphores[element]);
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

    int index;
    int current_element=0;
    const int elements_per_thread = num_rows * num_cols / num_threads;
    int remaining_elements = (num_rows * num_cols) % num_threads;
    pthread_t thread_ids[num_threads];
    struct histogram_data * histo_param = (struct histogram_data *) malloc(num_threads * sizeof(struct histogram_data));
    for (index=0; index < num_threads; index++)
    {
        histo_param[index].image = image;
        histo_param[index].first = current_element;
        current_element += elements_per_thread;
        if (remaining_elements)
        {
            remaining_elements--;
            current_element++;
        }
        histo_param[index].last = current_element;
        histo_param[index].histo = histo;
    }
    histo_param[num_threads-1].last = num_cols * num_rows;

    for (index = 0; index < 256; index ++) 
    {
        if ( sem_init(&semaphores[index], 0, 1) != 0 )
        {
            die("Initalization of semaphore failed.");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &before);
    /* Do your thing here */
    for (int index=0; index<num_threads; index++) {
        pthread_create(&thread_ids[index],
        NULL,
        histogram,
        (void *)&histo_param[index]);
    }

    /* Do your thing here */
    for (index =0; index<num_threads ; index++) {
        pthread_join(thread_ids[index], NULL);
    }

    if (debug){
    	print_histo(histo);
    }

    clock_gettime(CLOCK_MONOTONIC, &after);

    for (index = 0; index < 256; index ++) 
    {
        sem_destroy(&semaphores[index]);
    }

    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Histo took: % .6e seconds \n", time);

    free(histo);
    free(image);
    free(histo_param);
}
