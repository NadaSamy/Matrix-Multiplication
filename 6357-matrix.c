#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/*GLOBAL VARIABLES*/
int ** result;
int ** matrix1;
int ** matrix2;
clock_t startFunc1, endFunc1, startFunc2, endFunc2;
float diff1, diff2;

typedef struct ElementByElement 
{
	int row[1000];
	int coloumn[1000];
	int size;
	int sum;
	int i;
	int j;
	
}ElementByElement;

typedef struct RowByRow
{
	int rowIndex;
	int matrix2ROW;
	int matrix2COL;
	int sum;
}RowByRow;

/*FUNCTION PROTOYPES*/
void readData(int * arr1, int * arr2,char name[]);
void calculateElementByElement(int* index1, int* index2, int* resIndex);
void * calculateRes(void * arg);
void writeData(int * index);
void calculateRowByRow(int * index1,int * index2,int* res_index);
void * calculate2(void * arg);
void writeData2(int * index);
void initializeResultMatrix(int* res_index);

/* MAIN */
int main()
{
	//declare an array for the dimensions of each matrix and the fileName
	int mat1_index[2], mat2_index[2], res_index[2];
	char fileName[200];
	
	//get file name from user
	printf("Enter file name: ");
	scanf("%s",fileName);
	
	//read data from file
	readData(mat1_index,mat2_index, fileName);
	
	// store result matrix dimensions
	res_index[0] = mat1_index[0];
	res_index[1] = mat2_index[1];
	
	//check if the multiplication if possible
	if(mat1_index[1] != mat2_index[0])
	{
		//update file with the error message
		char message[] = {"MATRIX MULTIPLICATION NOT POSSIBLE."};
		FILE* fp = fopen("output.txt","w");
		fprintf(fp,"%s",message);
		fclose(fp);
		exit(0);
	}
	
	//allocate memory for the result matrix
	result = malloc(sizeof(int*) * res_index[0]);
	for(int y = 0; y < res_index[0]; y++)
	{
		result[y] = malloc(sizeof(int) * res_index[1]);
	}
	
	//initialize result matrix with zeros
	initializeResultMatrix(res_index);
	//caluclate result matrix element by element
	calculateElementByElement(mat1_index,mat2_index, res_index);
	//calculate time difference
	diff1 = endFunc1 - startFunc2;
	diff1 /= CLOCKS_PER_SEC;
	//write result and time in file
	writeData(res_index);
	
	//initialize result matrix zeros again for re-calculation
	initializeResultMatrix(res_index);
	// calculate result matrix using row by row method
	calculateRowByRow(mat1_index,mat2_index,res_index);
	//calculate time difference
	diff2 = endFunc2 - startFunc2;
	diff2 /= CLOCKS_PER_SEC;
	//write data in file
	writeData2(res_index);
	
	for(int i = 0; i < mat1_index[0]; i ++)
	{
		free(matrix1[i]);
		free(result[i]);
	}
	for(int j = 0; j < mat2_index[0]; j++)
	{
		free(matrix2[j]);
	}
	free(matrix1);
	free(matrix2);
	free(result);
	return 0;
}

/* FUNCTION DEFINITIONS */
void readData(int* arr1, int* arr2, char name[])
{
	FILE* fp;
	//open file in read mode
	fp = fopen(name,"r");
	int i = 0, j = 0;
	//if file opening was insuccessful print error message and exit
	if(fp == NULL)
	{
		printf("Error in opening file.");
		exit(0);
	}
	else
	{
		//MATRIX 1
		// read the matrix dimenions
		fscanf(fp,"%d %d\n",&arr1[0],&arr1[1]);
		//allocate memory for matrix 1
		matrix1 = malloc(sizeof(int*) * arr1[0]);
		for(int y = 0; y < arr1[0]; y++)
		{
			matrix1[y] = malloc(sizeof(int) * arr1[1]);
		}
		// read matrix element values and store them
		while(i < arr1[0])
		{
			j = 0;
			while(j < arr1[1])
			{
				fscanf(fp,"%d ",&matrix1[i][j]);
				j++;
			}
			i++;
		}
		
		//MATRIX 2
		//read matrix 2 dimensions
		fscanf(fp,"%d %d\n",&arr2[0],&arr2[1]);
		//allocate memory for matrix 2
		matrix2 = malloc(sizeof(int*) * arr2[0]);
		for(int y = 0; y < arr2[0]; y++)
		{
			matrix2[y] = malloc(sizeof(int) * arr2[1]);
		}
		
		i =0;
		j = 0;
		//scan elements of matrix 2 and store them
		while(i < arr2[0])
		{
			j = 0;
			while(j < arr2[1])
			{
				fscanf(fp,"%d ",&matrix2[i][j]);
				j++;
			}
			i++;
		}	
	}
	fclose(fp);
}


void calculateElementByElement(int* mat1_index, int* mat2_index, int* res_index)
{
		
	ElementByElement data;
	data.size = mat1_index[1];
	int count = 0;

	//CREATE THREADS
	int numOfThreads = res_index[0] * res_index[1];
	pthread_t th[numOfThreads];
	
	
	//create array of structs
	ElementByElement arr[numOfThreads];
	
	// CREATE ROWS
	//save clock time now
	startFunc1 = clock();
	//loop over rows of first matrix
	for(int i = 0; i < mat1_index[0]; i++)
	{
		//each loop get the row
		for(int j = 0; j < mat1_index[1]; j++)
		{
			data.row[j] = matrix1[i][j];
		}
		//loop over the coloumns of second matrix
		for(int z = 0; z < mat2_index[1]; z++)
		{
			//each loop get the coloumn
			for(int y = 0; y < mat2_index[0]; y++)
			{
				data.coloumn[y] = matrix2[y][z];	
			}
			//initialize each element in struct with needed values
			arr[count].sum = 0;
			arr[count].i = i;
			arr[count].j = z;
			arr[count].size = mat1_index[1];
			//copy rows and coloumns gotten before in the struct arr[i]
			for(int w = 0; w < mat1_index[1]; w++)
			{
				arr[count].row[w] = data.row[w];
				arr[count].coloumn[w] = data.coloumn[w];
				
			}
			//create the threads and pass the needed argument and function
			pthread_create(&th[count],NULL,&calculateRes, &arr[count++]);
			
		}	
		
		
	}
	//wait for threads
	for(int t = 0; t < numOfThreads; t++)
	{
		
		pthread_join(th[t],NULL);
	}
	//save clock time now.
	endFunc1 = clock();

}


void * calculateRes(void * arg)
{
	//derefrence void pointer
	ElementByElement * temp = arg;
	 temp->sum = 0;
	 //multiply each element in the row with each element in the coloumn and calculate sum
	for(int y = 0; y < temp->size; y++)
	{
		temp->sum += temp->row[y] * temp->coloumn[y];
	}
	//store sum in result matrix
	result[temp->i][temp->j] = temp->sum;
	
}


void writeData(int * index)
{

	FILE* fp;
	//open file in write mode
	fp = fopen("output.txt","w");
	char name[] = {"ELEMENT BY ELEMENT"};
	char time[] = {"TIME = "};
	// if file opening was insucessful then print error message
	if(fp == NULL)
	{
		printf("Error in opening file.");
		exit(0);
	}
	else
	{
		//print method name and result matrix and time
		fprintf(fp,"%s\n",name);
		for(int i = 0; i < index[0]; i++)
		{
			for(int j = 0; j < index[1]; j++)
			{
				fprintf(fp,"%d ",result[i][j]);
			}
			fprintf(fp,"\n");
		}
		//print time in the file
		fprintf(fp, "%s ",time);
		fprintf(fp,"%f\n\n",diff1);
	}
	
	fclose(fp);
}

void writeData2(int * index)
{

	FILE* fp;
	//open file in append mode to append to the last result written
	fp = fopen("output.txt","a");
	char name[] = {"ROW BY ROW"};
	char time[] = {"TIME = "};
	if(fp == NULL)
	{
		printf("Error in opening file.");
		exit(0);
	}
	else
	{
		fprintf(fp,"%s\n",name);
		for(int i = 0; i < index[0]; i++)
		{
			for(int j = 0; j < index[1]; j++)
			{
				fprintf(fp,"%d ",result[i][j]);
			}
			fprintf(fp,"\n");
		}
		fprintf(fp, "%s ",time);
		fprintf(fp,"%f",diff2);
	}
	
	fclose(fp);
}

void * calculate2(void * arg)
{
	//derefrence void pointer
	RowByRow * new = arg;
	//loop over coloumns in matrix 2
	for(int i = 0; i < new->matrix2COL; i++)
	{
		new->sum = 0;
		//loop over the rows of matrix 2
		for(int j = 0; j < new->matrix2ROW; j++)
		{
			//multiply the first element of each row in matrix 1 by the first element of each row in matrix 2
			//add the result to sum
			new->sum += matrix1[new->rowIndex][j] * matrix2[j][i];
		}
		//store in the result matrix
		result[new->rowIndex][i] = new->sum;
	}
	
}


void calculateRowByRow(int * index1,int * index2,int* res_index)
{
	//CREATING  THREADS
	int numberOfThreads = res_index[0], count = 0;
	pthread_t arrayOfThreads[numberOfThreads];
	
	//CREATING ARRAY OF STRUCT ROW BY ROW
	RowByRow arr[numberOfThreads];
	//save clock time now
	startFunc2 = clock();
	//loop over the number of rows
	for(int i = 0; i < index1[0]; i++)
	{
		//save row index that will be used by thread and dimensions of matrix 2
		arr[i].rowIndex = i;
		arr[i].matrix2ROW = index2[0];
		arr[i].matrix2COL = index2[1];
		arr[i].sum = 0;
		//create the threads and pass the needed argument and function
		pthread_create(&arrayOfThreads[i],NULL,&calculate2,&arr[i]);
	}
	// wait for each thread
	for(int j = 0; j < numberOfThreads; j++)
	{
		pthread_join(arrayOfThreads[j],NULL);
	}
	//save clock time now
	endFunc2 = clock();
	
}

void initializeResultMatrix(int* res_index)
{
	//put zeros in the result matrix
	for(int i = 0; i < res_index[0]; i++)
	{
		for(int j = 0; j < res_index[1]; j++)
		{
			result[i][j] = 0;
		}
	}
}
