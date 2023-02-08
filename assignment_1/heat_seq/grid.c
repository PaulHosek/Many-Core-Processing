

struct pointType {
    double temperature[2];        /* Old and new temperature value */
    double temperature_sums[2];   /* Sums of the temperatures at the north and south cells and sum of the temperatures at the east and west cells*/
    double conductivity;          /* Conductivity */
    double weight_direct;        /* Weight as direct neighbor */
    double weight_indirect;      /* Weight as diagonal neighbor */
};

struct grid {
    struct pointType * points;  /* Points array (one-dimensional)*/
    int                 N;      /* Number of rows */
    int                 M;     /* Number of columns */
    int                 old;    /* Flips between 0 and 1 to indicate where old or new temperature values are*/
};


#define T(g, p)        (g)->points[p].temperature[(g)->old]     /* temperature (getter) */
#define TN(g, p)       (g)->points[p].temperature[(g)->old^1]    /* new temperature, just for setting */
#define WD(g, p)       (g)->points[p].weight_direct             /* Weight as direct neighbor*/
#define WI(g, p)       (g)->points[p].weight_indirect           /* Weight as diagonal neighbor*/
#define C(g, p)       (g)->points[p].conductivity               /* Conductivity*/
#define TSV(g, p)       (g)->points[p].temperature_sums[0]      /* Temperature sum of the vertical neighbors*/
#define TSH(g, p)       (g)->points[p].temperature_sums[1]      /* Temperature sum of the horizontal neighbors*/
