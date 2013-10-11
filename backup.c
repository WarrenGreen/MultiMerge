/** @file msort.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CEIL(X) (X-(int)(X) > 0 ? (int)(X+1) : (int)(X))
#define MIN(X,Y) ((X) < (Y) ? : (X) : (Y))

typedef struct _split{
	int *nums;
	int *toMerge;
	int size;
}split;

int cmpfun(const void *a, const void *b){
	return ( *(int*)a - *(int*)b );
}

int sort(void *part){
	split *partition = part;
	qsort(partition->nums, partition->size, sizeof(int), &cmpfun);
	fprintf(stderr, "Sorted %d elements.\n", partition->size);
	return 0;
}

int merge(void *part){
	split *partition = part;
	int i,j,duplicates = 0;
	int* new = malloc(size*(int));
	for(i=0,j=0;i+j<size;){
		if(partition->nums[i]<partition->toMerge[j]){
			new[i+j] = partition->nums[i];
			i++;
		}else if(partition->nums[i]==partition->toMerge[j]){
			duplicates++;
			new[i+j] = partition->nums[i];
			i++;
		}else{
			new[i+j] = partition->toMerge[j];
			j++;
		}
	}

	return new;
}

int main(int argc, char **argv)
{
	if(argc != 2) {
		fprintf( stderr,"usage: %s [#of splits] \n", argv[0]);
		return 1;
	}

	int numSplits = atoi(argv[1]);
	char* line = NULL;
	size_t size;
	int *numbers = malloc(7*sizeof(int));
	int count =0;

	while(getline(&line, &size,stdin) != -1){
		fflush(stdout);
		line[ strlen(line)-1] = '\0';
		if(count>sizeof(numbers)) realloc(numbers, ((int)1.5*sizeof(numbers)));
		numbers[count] = atoi(line);
		count++;
		line = NULL;
	}


	int i,j;

	for(i=0; i<count;i++){
		printf("%d\n", numbers[i]);
	}
	printf("_____________\n");

	split* splits = malloc(sizeof(split) * numSplits); 

	for(i =0;i<numSplits;i++){
		splits[i].nums = malloc(5*sizeof(int));
		splits[i].size = CEIL(count/numSplits);
		for(j=0;j<splits[i].size;j++){
			splits[i].nums[j] = numbers[(i*5)+j];
		}
	}
	
	pthread_t *threads = malloc(sizeof(pthread_t)*numSplits);
	for(i =0; i<numSplits;i++){
		pthread_create(&threads[i], NULL, sort, &splits[i]);
	}

	int *ret;
	for(i=0;i<numSplits;i++){

		pthread_join(threads[i], (void **) &ret);
	}

	for(i=0;i<(int)numSplits/2;i++){
		splits[i*2].toMerge = splits[(i*2)+1].nums;
		pthread_create(&threads[i], NULL, merge, &splits[i*2]);
	}

	int *new;
	for(i=0;i<(int)numSplits/2;i++){
		pthread_join(threads[i], (void **) &new);
		splits[i*2].nums = new;
		
	}

	for(i =0; i<numSplits;i++){
		for(j=0;j<5;j++){
			printf("%d\n", splits[i].nums[j]);
		}
	}


	return 0;
}