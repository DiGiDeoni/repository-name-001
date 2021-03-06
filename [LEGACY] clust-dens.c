/**
This C program implements the clustering algorithm by 
Rodriguez and Laio (2014).
A great part of this work is based from Eric Yuan's code,
which is obtainable by accessing http://eric-yuan.me. 
**/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define KKK 		7				//initial centroid count
#define DIM 		2 				//dimension/feature
char IN [20] = "aggregation"; 		//input file name
char DG [20] = "aggrhodel";			//output decision graph

/*=============================================
	DATA SETS		 FEATURES		DENSITY
  =============================================
	iris-data			4D			GAUSSIAN
	s1					2D			GAUSSIAN
	aggregation			2D			GAUSSIAN
	spiral.txt			2D			NON-GAUSSIAN
*/

#define newline		printf("\n")	
#define MAX(a,b)	( (a)>(b) ? (a):(b)  )		
#define ArSeq(n)	( (n*(n-1))/2 )		

#define elif else if

#ifndef bool
    #define bool int
    #define false ((bool)0)
    #define true  ((bool)1)
#endif

#define for_i(a,b) \
		for (int a=0; a<b; a++)
			
#define for_ij(a,b,c,d)	\
		for_i(a,b) for_i(c,d)	

void Terminate(char ot[]){
	printf("\n\n%s\n",ot);
	printf("Terminating routine\n\n");
	exit(1);
	}
	
int Time_Now(char ing[] ){
	printf ("\n%s:\t",ing);
    time_t t_time;
    char mos[25];
    struct tm* tm_info;
    time(&t_time);
    tm_info = localtime(&t_time);
    strftime(mos, 25, "%Y:%m:%d - %H:%M:%S\n", tm_info);
    puts(mos);
    return 0;
	}
	
float* vector(int N){
	float *u	= (float*)calloc(N,sizeof(float));
	if (!u) {
		Terminate ("Failed vector allocation");
		return NULL;
		}
	return u;
	}

float** matrix (int Na, int Nb){
	int a,b;
	float **me = (float**)calloc(Na,sizeof(float*));
	if (!me) Terminate ("Failed allocating memory for 1st pointers");
	if ((me[0] = (float*)calloc(Nb,sizeof(float))) == NULL){
		Terminate ("Failed constructing matrix");
		return NULL;
		}
	for (a=0;a<Na;a++) 
		me[a] = (float*)calloc(Nb,sizeof(float));
	return me;
	}

void free_matrix(float **ly){
  float **it = (float **)ly;
	free(it[0]);
	free(ly);
	}

float** GetData(
	char 	mos[], 		//file name
	float** elem, 		//data matrix
	long 	rows, 		//row length
	long* 	temp){		//total data size counter

	elem = (float**) calloc(rows,sizeof(float*));
	char str[20], val[10]; 
	long int i = 0, j = 0, k = 0, resize;
	FILE* fp1;
	
	/* check if file is present */
	if((fp1 = fopen(mos,"r"))==NULL)	
		Terminate("Missing input");	
	
	for(i=0; !feof(fp1); i++){
		fgets (str,100,fp1);
		/* check if # of row scanned exceeds intial row*/
		if (i >= rows){
			resize = 2*rows;
			elem = (float**) realloc(elem, sizeof(float *)*resize);
			if (!elem)
				Terminate("Failed to reallocate memory");	
			rows = resize;
		}
		elem[i] =  vector(DIM);
		for_i(j,DIM){		
		/*	SPIRAL DATA SET (2D)
			k = 5*j; 		
			elem[i][j] = atof(strncpy(val,&str[k],8));			
		*/	
		/*	S1 DATA SET (2D)
			k = 8*j; 		
			elem[i][j] = atof(strncpy(val,&str[4+k],8));			
		*/	
		/*	AGGREGATION DATA SET (2D)
		*/			
			k = 5*j; 		
			elem[i][j] = atof(strncpy(val,&str[k],10));
		/*	IRIS DATA SET (4D)
			k = 4*j; 		
			elem[i][j] = atof(strncpy(val,&str[k],4)); 
		*/
		/*	MOPSI-FINLAND DATA SED (2D)
			k = 6*j;			
			elem[i][j] = atof(strncpy(val,&str[k],7)); 
		*/
		}
	}
	fclose(fp1);
	
	/*trim remaining array elements*/
	if(i<rows){
		for(j=i; j<rows; j++)
			free(elem[j]);
		elem = (float**) realloc(elem, sizeof(float*)*i);
	}
	*temp = i;
	
	/*
	for_i (i, temp[0]){
		printf("%d\t",i+1);
		for_i(j,DIM)
			printf("%.2f\t", elem[i][j]);
		newline;
	}
	*/
	
	return elem;
}	
	
float Euclid(
	float** elem, 		//data elements
	int 	psrc, 		//source point
	int 	pref){		//referrence point
		
	float tmp = 0, dx = 0;
	long j;
	
	for_i(j,DIM){
		dx  = elem[psrc][j]-elem[pref][j];
		tmp += dx*dx;
	}
	
	return sqrt(tmp);
}

void DistVec (
	float* 	dvec, 		//distance vector
	float** elem, 		//data elements
	long 	size){		//data size
		
	//does not double count
	long i = 0, j = 0, k = 0;
	
	for_ij(i,size,j,i)
		dvec[k++]	= Euclid(elem,i,j);
}

void mSort(
	float* 	dvec, 		//sort distance
	float* 	temp, 		//temporary distance container
	long 	left, 		//left bound
	long 	rght){		//right bound
/*
obtained from 
http://www.cprogramming.com/tutorial/computersciencetheory/merge.html
*/
	if(rght == left + 1)
        return;
	
	else{
		long int i = 0;
		long int size = rght-left;
		long int midpt = size/2;
		/* l and r are to the positions in the left and rght subarrays */
		int l = left, r = left + midpt;
		/* sort each subarray */
		mSort(dvec, temp, left, left + midpt);
		mSort(dvec, temp, left + midpt, rght);
		
		/* merge the arrays together using temp for temporary storage */ 
		for_i(i, size){
			/* Check to see if any elements remain in the left array; if so,
			* we check if there are any elements left in the rght array; if
			* so, we compare them.  Otherwise, we know that the merge must
			* use take the element from the left array */
			if(l < left + midpt && 
					(r == rght || MAX(dvec[l], dvec[r]) == dvec[l]))
				temp[i] = dvec[l++];
			else
				temp[i] = dvec[r++];
		}
		/* Copy the sorted subarray back to the dvec */
		for(i = left; i < rght; i++)
			dvec[i] = temp[i - left];
	}
}

float GetDC(
	float* 	dvec, 		//distance vector
	float 	rate, 		//neighbor ratio
	long 	size){		//data size
		
	long int pos;	
	float cutoffdistance;
	float* scratch = vector(size);
	float* duplic8 = vector(size);
	
	for_i(pos, size) 
		duplic8[pos] = dvec[pos];
	
	pos = size*rate;
	mSort(duplic8, scratch, 0, size);
	cutoffdistance = duplic8[size-pos];
	
	free (scratch);
	free (duplic8);
	return cutoffdistance;
}

void LocalDensity(
	float* 	dens, 		//density
	float* 	dvec, 		//distance vector
	float 	dcut, 		//cutoff distance
	long 	lnth){		//data size
		
	long int i = 0,  j = 0, k = 0;
	float dx, num, den;
	
	for_ij(i,lnth,j,i){
			if (dvec[k++] <= dcut ){
				num  = dvec[k]*dvec[k];
				den  = dcut*dcut;
				dens[i] += exp(-num/den);
				dens[j] += exp(-num/den);
			}
	}
}

long Peak(
	float* 	dens,	 	//density
	float* 	delt,		//nearest high density
	long* 	indx, 		//index of delta
	float* 	dvec, 		//distance vector
	long 	lnth){		//data size
	long  i = 0, j = 0, k = 0, inf = 0;
	float tmp_i, tmp_j;
	
	for(i = 0; i<lnth-1; i++){
		if (dens[i] > dens[i+1]){
			tmp_j = dens[i];
			k = i;
		}
		else{
			tmp_j = dens[i+1];
			k = i+1;
		}
		if (tmp_i < tmp_j){
			tmp_i = tmp_j;
			inf = k;
		}	
	}	
	
	for(i=1, k=0; i<lnth; i++)
		for(j=0; j<i; j++, k++){	
			if (dens[j] > dens[i]){
				if ((delt[i]==0) || (delt[i]>dvec[k])){ 
					delt[i] = dvec[k];
					indx[i] = j;
				}
			}
			elif (dens[i] >= dens[j]){ 
				if((delt[j]==0) || (delt[j]>dvec[k])){ 
					delt[j] = dvec[k];
					indx[j] = i;
				}
			}
			if ((i==inf) || (i>inf && j==inf))
				if (delt[inf] < dvec[k]){
					delt[inf] = dvec[k];
					indx[inf] = i;
				}
		}
	return inf;
}

long* ClustCenters(
	float* 	dens, 		//density array
	float* 	delt, 		//delta array
	float 	rate, 		//ratio
	long*	temp,
	long 	imax, 		//index of max rho and delta
	long 	lnth){		//data size
	
	long i = 0, j = 0, k = KKK;
	
	long* cntr = (long*) calloc(KKK,sizeof(long*));
		if (!cntr) Terminate("Failed to create array");
	
	float mindel = delt[imax]*rate;
	float minrho = dens[imax]*rate;
	
	for_i(i,lnth-1){
		if(dens[i]>minrho && delt[i]>mindel){
			if ((j+1)>=KKK){
				k += 5;
				cntr = (long*) realloc(cntr, sizeof(long*)*k);
				if(!cntr) Terminate("Failed to reallocate");
			}
			cntr[j] = i;
			j++;
		}
	}
	*temp = j;
	return cntr;
}

void AssignClust(	
	long*	indx,		//cluster assignment
	long*	peak,		//cluster centers
	long	lnth,		//data size
	long	pnum){		//cluster count
	
	long  h = 0, i = 0, j = 0;

	for_i(i,pnum) 
		indx[peak[i]] = peak[i];
	
	for_i(i,lnth){
		j = indx[i];	
		h = 0;
		while (indx[j] != j){
			j = indx[j];
			if (h++ == lnth){
				printf("failed assignment on %d\n", i);
				break;
			}
		}
		indx[i] = j;
	}
	return;
}

void Halo(
	float* 	dens,		//density
	float* 	dvec,		//distance vector
	long* 	indx,		//cluster assignment index
	long* 	cntr,		//cluster centers
	long	lnth,		//array size
	long	pnum,		//cluster centers count
	long	dcut		//cutoff distance
	){
	
	long 	i, j, k;
	float 	RAve;
	float* 	BordR = vector(pnum);
	
	for_ij(i,lnth,j,pnum)
		if(indx[i]==cntr[j]){
			indx[i] = j;
			break;
		}

	for(i=0; i<lnth-1; i++){
		for(j=i+1; j<lnth; j++){
			k = ArSeq(j)+i;
				if ((indx[i]!=indx[j]) && (dcut>=dvec[k])){
				RAve = (dens[i]+dens[j])/2;
				if(RAve>BordR[indx[i]])
					BordR[indx[i]] = RAve;
				if(RAve>BordR[indx[j]])
					BordR[indx[j]] = RAve;	
			}
			k++;
		}
	}
	
	/*for_i(i,lnth)
		if(dens[i]<BordR[indx[i]])
			indx[i] = -1;
	*/
	free (BordR);
	return;
}

void main(){
	Time_Now("start");
 
	long int i=0, j=0, k=0, resize;		
	long ARRAY_SIZE	= 200;
	float** data = (float**) calloc(ARRAY_SIZE,sizeof(float*));
		if (!data) Terminate("Failed to create first pointer");
	long *tmp = (long*) calloc (1,sizeof(long*));
		if (!tmp) Terminate("Failed creating pointer");
	data = GetData(IN, data, ARRAY_SIZE, tmp);
	ARRAY_SIZE = tmp[0];
	
	printf("ARRAY SIZE: %d\n\n", ARRAY_SIZE);

	long   NEW_SIZE = ArSeq(ARRAY_SIZE); 
	printf("ARRAY SEQ: %d\n\n", NEW_SIZE);
	float* dvec = vector (NEW_SIZE);
	DistVec(dvec, data, ARRAY_SIZE);
	
	float  dc	= GetDC(dvec, 0.01, NEW_SIZE);
	printf("cutoff distance is: %f\n",dc);
	
	float* rho	= vector(ARRAY_SIZE);
	LocalDensity(rho, dvec, dc, ARRAY_SIZE);
	
	float* delta 	= vector(ARRAY_SIZE);
	long* delta_i 	= (long*) calloc(ARRAY_SIZE,sizeof(long*));
		if (!delta_i) Terminate("Failed to create array");
	long   iMax 	= Peak(rho, delta, delta_i, dvec, ARRAY_SIZE);
	float  rhoMax 	= rho[iMax];
	float  deltaMax = delta[iMax];
	
	printf("MAX value index: %d\n", iMax);
	
	float* gamma = vector(ARRAY_SIZE);
	for_i (i,ARRAY_SIZE) 
		gamma[i] = rho[i] * delta[i];
	
	printf("MAXIMUM RHO: %f\n",rhoMax);
	printf("MAXIMUM DELTA: %f\n",deltaMax);
	
	long* peaks = ClustCenters(rho, delta, 0.16, tmp, iMax, ARRAY_SIZE);
	long  nClust = tmp[0];
	free (tmp);
	
	printf("there are %d clusters\n",nClust);
	
	if(nClust<1)
		Terminate("NO CLUSTERS FOUND");

	for_i(i,nClust)
		printf("cluster %d\n",peaks[i]);
	
	AssignClust(delta_i, peaks, ARRAY_SIZE, nClust);
		
	printf("Clusters assigned\n");
		
	Halo(rho, dvec, delta_i, peaks, ARRAY_SIZE, nClust, dc);
	
	FILE *f1 = fopen(DG, "w");
	fprintf(f1,"#point \t\t rho \t\t delta \t\t rho*delta\n");
	for_i (i,ARRAY_SIZE){
		fprintf(f1,"%d\t\t\t", i+1);
		fprintf(f1,"%.0f\t\t\t", rho[i]);
		fprintf(f1,"%.5f\t\t\t\t", delta[i]);
		fprintf(f1,"%.5f\n", gamma[i]);
	}
	fclose(f1);
	
	FILE *f2 = NULL;
	char f3[15];
	for_i(i,nClust){
		snprintf (f3, sizeof (char)*30, "%d.cl", i);
		f2 = fopen(f3,"w");
		for_i(j,ARRAY_SIZE)
			if(delta_i[j]==i){
				for_i(k,DIM)
					fprintf(f2,"%.3f\t", data[j][k]);
				fprintf(f2,"\n");
			}
	}
	fclose (f2);
	
	f2 = fopen ("halo.cl", "w");
	for_i(i,ARRAY_SIZE)
		if(delta_i[i]==-1){
			for_i(j,DIM)
				fprintf(f2,"%.3f\t", data[i][j]);
			fprintf(f2,"\n");
		}
	fclose (f2);
		
	Time_Now("end");
	
	free 		(rho);
	free 		(dvec);
	free 		(peaks);
	free 		(delta);
	free 		(delta_i);
	free_matrix (data);
}
