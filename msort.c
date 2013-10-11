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
	int size2;
}split;

int cmpfun(const void *a, const void *b){
	return ( *(int*)a - *(int*)b );
}

int *sort(void *part){
	split *section = part;
	qsort(section->nums, section->size, sizeof(int), &cmpfun);
	fprintf(stderr, "Sorted %d elements.\n", section->size);
	return 0;
}

int *merge(void *part){
	split *partition = part;
	int i,j,duplicates = 0;
	int* new = malloc((partition->size + partition->size2) * sizeof(int));
	for(i=0,j=0;i+j<partition->size+partition->size2;){
		if(i<partition->size && (j>=partition->size2|| partition->nums[i]<partition->toMerge[j])){
			new[i+j] = partition->nums[i];
			i++;
		}else if(i>=partition->size|| partition->nums[i]>partition->toMerge[j]){
			new[i+j] = partition->toMerge[j];
			j++;
		}else if(partition->nums[i]==partition->toMerge[j]){
			duplicates++;
			new[i+j] = partition->nums[i];
			i++;
		}
	}
	fprintf(stderr, "Merged %d and %d elements with %d duplicates.\n", partition->size, partition->size2, duplicates);
	free(partition->nums);
	partition->nums = new;
	partition->size += partition->size2;
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
	int *numbers = malloc(6*sizeof(int));
	int n_size = 6;
	int count =0;

	while(getline(&line, &size,stdin) != -1){
		fflush(stdout);
		line[ strlen(line)-1] = '\0';
		if(count>=n_size) numbers = realloc(numbers, ((int)(n_size *= 1.5)) * sizeof(int));
		numbers[count] = atoi(line);
		count++;
		line = NULL;
	}

	int i,j;
	int splitSize = CEIL((float)count/numSplits);
	pthread_t *threads = malloc(numSplits * sizeof(pthread_t));
	split *splits = malloc(numSplits * sizeof(split));

	for(i=0;i<numSplits;i++){
		splits[i].nums = malloc(splitSize * sizeof(int));
		splits[i].size = splitSize;
		if((i*splitSize)+splitSize > count) splits[i].size = count - (i)*splitSize;
		for(j=0; j< splits[i].size;j++){
			splits[i].nums[j] = numbers[(i*splitSize)+j];
		}

		pthread_create(&threads[i], NULL, sort, &splits[i]);
	}

	int *ret;
	for(i=0;i<numSplits;i++){
		pthread_join(threads[i],(void **) &ret);
	}

	int activeSplits = numSplits;
	int ct = 0;
	while(activeSplits>1){
		for(i=0;i<numSplits;i++){
			if(splits[i].size >0){
				for(j=i+1;j<numSplits;j++) 
					if(splits[j].size>0)break;
				if(j<numSplits){
					splits[i].toMerge = splits[j].nums;
					splits[i].size2 = splits[j].size;
					splits[j].size = 0;
					pthread_create(&threads[ct], NULL, merge, &splits[i]);
					ct++;
					activeSplits-=1;
				}
			}
		}

		for(i=0;i<ct;i++){
			pthread_join(threads[i], (void **) &ret);
		}
	}
	
	qsort(numbers, count, sizeof(int), &cmpfun);

	for(i=0;i<numSplits;i++){
		if(splits[i].size>0){
			for(j=0; j< splits[i].size;j++){
				printf("%d\n", splits[i].nums[j]);

			}
		}
		free(splits[i].nums);
	}

	free(numbers);
	free(splits);

	return 0;
}
