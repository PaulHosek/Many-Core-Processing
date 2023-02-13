#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"
#include <stdio.h>

#include <immintrin.h>

double * update_4(int index, int M,int N, double * temps_new,
 double * temps_old, double * conductivity, double * direct, double * indirect);
void initialise(const struct parameters* p);

double * temps_old;
double * temps_new;
double * conductivity;
double * direct;
double * indirect;

void print_grid(double *my_grid, int M,int N){
    /* Print temperature with halo cells */
    int i = 0;
    for (; i < N; ++i){
        for (int j = 0; j < M; ++j){
            printf("%10.3f ", my_grid[i * M + j]);
        }
        printf("\n");
    }
}

void do_compute(const struct parameters* p, struct results *r){
    initialise(p);
    // print_grid(indirect, p->M, p->N);

    double * new_res = update_4(p->M, p->M,p->N, temps_old,temps_new, conductivity, direct,indirect);
    printf("%f %f %f %f\n",
        new_res[0], new_res[1], new_res[2], new_res[3]);


    // TODO: loop etc here

    // Free memory space 
    _mm_free(new_res);
    _mm_free(temps_new);
    _mm_free(temps_old);
    _mm_free(conductivity);
    _mm_free(direct);
    _mm_free(indirect);
}


void initialise(const struct parameters* p){
    // initialise array
    int M = p->M;
    int N = p->N;
    int MN = M*N; 


    temps_old = (double *) _mm_malloc((N + 2) * M * sizeof(double), 32);
    temps_new = (double *) _mm_malloc((N + 2) * M * sizeof(double), 32);
    conductivity = (double *) _mm_malloc((N) * M * sizeof(double), 32); // TODO: check if right size
    direct = (double *) _mm_malloc((N) * M * sizeof(double), 32);
    indirect = (double *) _mm_malloc((N) * M * sizeof(double), 32);


    // Halo rows
    for (int index = 0; index < M; index++)
    {
        temps_old[index] = p->tinit[index];
        temps_new[index] = temps_old[index];
        temps_old[MN + M + index] = p->tinit[MN - M + index];
        temps_new[MN + M + index] = temps_old[MN + M + index];
    }
    
     // Fill the temperature values into the grid cells
    for (int index = 0; index < M * N; index++)
    {   
        temps_new[M+index] = p->tinit[index];
        double cur_conduct = p->conductivity[index];
        double joint_weight_diagonal_neighbors = (1 - cur_conduct) / (sqrt(2.0) + 1);
        double joint_weight_direct_neighbors = 1 - cur_conduct - joint_weight_diagonal_neighbors;

        // T(&cylinder_grid, M + index) = p->tinit[index];

        conductivity[M+index] = cur_conduct;
        direct[M+index] = joint_weight_direct_neighbors / 4.0;
        indirect[M+index] = joint_weight_diagonal_neighbors / 4.0;
    }
}
// printf("hallo\n");
// __m256d gelieza = _mm256_load_pd(temps_old);
// __m256d paul = _mm256_load_pd(temps_old+4);
// __m256d my_vector = _mm256_add_pd(gelieza,paul);
// __attribute__ ((aligned (32))) double output[4];
// _mm256_store_pd(output, my_vector);



// printf("%f %f %f %f\n",
//         output[0], output[1], output[2], output[3]);

double* update_4(int index, int M,int N, double * temps_new,
 double * temps_old, double * conductivity, double * direct, double * indirect)
{

    // generate index arrays
    int indices[] = {index, index+1, index+2,index+3};
    int indices_left[4];
    int indices_right[4];
    int i;
    for(i=0; i<4;i++){
        indices_left[i] =  indices[i] - 1;
        indices_right[i] = indices[i] + 1;

        if (indices[i] % M == 0)
        {
            indices_left[i] = indices[i] + M - 1;
        }

        if (indices[i] % M == M - 1)
        {
            indices_right[i] = indices[i] - M + 1;
        }
    }

    double * return_temperatures = (double *) _mm_malloc(4*sizeof(double), 32);

    // TODO: fix the aweful indexing there 
    // Load temperatures and condictivity
    __m256d cur_old_temps = _mm256_load_pd(&(temps_old[index]));
    __m256d cur_conduct = _mm256_load_pd(&conductivity[index]);

    __m256d left_direct = _mm256_set_pd(temps_old[indices_left[0]],temps_old[indices_left[1]],temps_old[indices_left[2]],temps_old[indices_left[3]]);
    __m256d left_indirect_top = _mm256_set_pd(temps_old[indices_left[0]-M],temps_old[indices_left[1]-M],temps_old[indices_left[2]-M],temps_old[indices_left[3]-M]);
    __m256d left_indirect_below = _mm256_set_pd(temps_old[indices_left[0]+M],temps_old[indices_left[1]+M],temps_old[indices_left[2]+M],temps_old[indices_left[3]+M]);

    __m256d right_direct = _mm256_set_pd(temps_old[indices_right[0]],temps_old[indices_right[1]],temps_old[indices_right[2]],temps_old[indices_right[3]]);
    __m256d right_indirect_top = _mm256_set_pd(temps_old[indices_right[0]-M],temps_old[indices_right[1]-M],temps_old[indices_right[2]-M],temps_old[indices_right[3]-M]);
    __m256d right_indirect_below = _mm256_set_pd(temps_old[indices_right[0]+M],temps_old[indices_right[1]+M],temps_old[indices_right[2]+M],temps_old[indices_right[3]+M]);

    __m256d cur_direct_top = _mm256_load_pd(&temps_old[index-M]);
    __m256d cur_direct_below = _mm256_load_pd(&temps_old[index+M]);
    
    __m256d indirect_weight = _mm256_load_pd(&indirect[index-M]);
    __m256d direct_weight = _mm256_load_pd(&direct[index-M]);

    // Compute stuff
    __m256d cur_temp_res = _mm256_mul_pd(cur_old_temps,cur_conduct);

    __m256d indirect_temp_res = _mm256_add_pd(_mm256_add_pd(left_indirect_top,left_indirect_below), _mm256_add_pd(right_indirect_top,right_indirect_below));
    indirect_temp_res = _mm256_mul_pd(indirect_temp_res, indirect_weight);
    
    __m256d direct_temp_res = _mm256_add_pd(_mm256_add_pd(cur_direct_top,cur_direct_below), _mm256_add_pd(right_direct,left_direct));
    direct_temp_res = _mm256_mul_pd(direct_temp_res, direct_weight);

    // add to final sum and store
    __m256d final_sum = _mm256_add_pd(_mm256_add_pd(indirect_temp_res, direct_temp_res), cur_temp_res);

    printf("%f %f %f %f\n",
        direct_temp_res[0], direct_temp_res[1], direct_temp_res[2], direct_temp_res[3]);


    _mm256_store_pd(return_temperatures, final_sum);


    return return_temperatures; 
}