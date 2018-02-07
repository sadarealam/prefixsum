#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>
#include<math.h>

#define LIST_SIZE  8000000

int* prefix_sum_chunk(int *list,int size){
	int log_of_chunk_size  = ceil(log(size)/log(2));	
	int *temp = (int *)malloc(sizeof(int) * chunk_size);	
	for(int i=0;i<log_of_chunk_size;i++){	
		int* current ;
		int* prev ;
		if(i%2==0) {
			current = temp ;
			prev  = list ;
		}else{
			current = list;
			prev = temp;
		}	
		for(int j=0;j<chunk_size;j++){
			int poi = pow(2,i);			
			if(j<poi){
				current[j] = prev[j];
			}else{
				current[j] = prev[j]+prev[j-poi];
			}
		}
	}
	if((log_of_chunk_size-1)%2 == 0) return temp ;
	else return list ;
}

void add_last_toAll(int *list,int size,int last_item){
	for(int i=0;i<size;i++){
		list[i] = list[i]+last_item ;
	}
}

int main(int argc,char *argv[]){
	int rank;
	int size;
	int status = MPI_Init(&argc,$argv);
	if(status!=MPI_SUCCESS){
		printf("MPI initialization Problem");
		return 0 ;
	}
	
	MPI_Comm_rank(&rank);
	MPI_Comm_size(&size);
	
	int chunk_size = LIST_SIZE  / size ;

	int * list = NULL ;
	int *temp = NULL ;
	if(rank == 0 ){
		list = (int *) malloc(sizeof(int) * LIST_SIZE );
		for(int i=0;i<LIST_SIZE;i++){
			list[i] = i+1;
		}
		
		for(int i=1;i<size;i++){
			int *sub_list = list+chunk_size ;
			if(i==size-1) MPI_Send(sub_list,chunk_size+(LIST_SIZE%size),MPI_INT,i,0,MPI_COMM_WORLD);
			else MPI_Send(sub_list,chunk_size,MPI_INT,i,0,MPI_COMM_WORLD);			
		}


		temp = prefix_sum_chunk(list,chunk_size);
		
		int last_item = &temp[chunk_size-1];
		MPI_Send(&last_item,1,MPI_INT,1,1,MPI_COMM_WORLD);
		
		int final_sum ;
		MPI_STATUS status2;
		MPI_Recv(&final_sum,1,MPI_INT,size-1,1,MPI_COMM_WORLD,&status2);

		int *result = (int* ) malloc(sizeof(int)*LIST_SIZE);
		int pointer = 0;
		for(int i=0;i<chunk_size;i++){
			result[i]=temp[i];
		}

		for(int =1;i<size;i++){			
			if(i == size-1) chunk_size = chunk_size + (LIST_SIZE%size) ;
			MPI_STATUS status;
			int *temp = (int*) malloc(sizeof(int)*chunk_size);
			MPI_Recv(temp,chunk_size,MPI_INT,i,2,MPI_COMM_WORLD,&status);
			for(int j=0 ;j<chunk_size;j++){
				result[j] = temp[pointer+j];
			}
			pointer = chunk_size ;			
		}

	}else{
				
		if(rank == size-1) chunk_size = chunk_size + (LIST_SIZE%size) ;
		temp = (int*) malloc(sizeof(int) * chunk_size);
		MPI_STATUS status;
		MPI_Recv(temp,chunk_size,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		temp = prefix_sum_chunk(temp,chunk_size);
		int pre_last_item ;
		MPI_STATUS last_status;
		MPI_Recv(&pre_last_item,1,MPI_INT,rank-1,1,MPI_COMM_WORLD,&last_status);
		add_last_toAll(temp,chunk_size,pre_last_item);
		int last_item = temp[0][chunk_size-1];
		if(rank != size-1) MPI_Send(&last_item,1,MPI_INT,rank+1,1,MPI_COMM_WORLD);
		else MPI_Send(&last_item,1,MPI_INT,0,1,MPI_COMM_WORLD);
		
		MPI_Send(&temp,chunk_size,MPI_INT,0,2,MPI_COMM_WORLD);

	}

	
}
