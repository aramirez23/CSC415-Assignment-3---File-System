/************************************************************** 
* Class:  CSC-415-0# Spring 2020 
* Group Name: Kernel Sanders
* Name: Alexander Beers
* Student ID: 917308410
* Name: Alicia Ramirez
* Student ID: 917379715
* Name: Michael Canson
* Student ID: 920601003
* Name: Amron Berhanu
* Student ID: 916320644
* Project: Assignment 3 – File System 
* 
* File: fsdriver3.c
* 
* Description: Interactive test driver that executes various commands 
*			   for the custom file system included with logicalFS.h 
* **************************************************************/
#include "logicalFS.h"

//command prompt functions
void listDir(int currentDirIndex, entry *entries, int size);
int changeDir(char *args[], int currentDirIndex, entry *entryList, uint64_t size);
void previousDir(char *args[], int * currentDirIndex, entry *entryList, uint64_t size);
void makeDir(char *args[], int currentDirIndex, entry *entryList, char *bitMap);
void rmDir(char *args[], int currentDirIndex, entry *entryList, char *bitMap, uint64_t numDirEntries);
void rmFile(char *args[], int currentDirIndex, entry *entryList, char *bitMap, uint64_t numDirEntries);
void addFile(char *args[], int currentDirIndex, entry *entryList, char *bitMap);
int cpFile(char *args[], int currentDirIndex, entry *entryList, char *bitMap, uint64_t numDirEntries);
void mvFile(char *args[], int currentDirIndex, entry *entryList, char *bitMap, uint64_t numDirEntries);
void copyNormaltoCurrent(char *args[], int currentDirIndex, entry *entryList, char *bitMap);
void copyCurrenttoNormal(char *args[], char fileName[], entry *entryList, uint64_t numDirEntries, int currentDirIndex);

int main(int argc, char *argv[])
{
	//get name of volume to use in startPartitionSystem
	if (argv[1] == NULL)
	{
		printf("Error: must add as an argument the name of Volume to load.\nIf entered name is not found a new Volume will be created with entered name.\n");
		exit(0);
	} 
	char* fileName = malloc(strlen(argv[1]) + 1);
	strcpy(fileName, argv[1]);

	uint64_t vSize = VOLUME_SIZE;
	uint64_t *volSizePtr = &vSize;
	uint64_t bSize = BLOCK_SIZE;
	uint64_t *blockSizePtr = &bSize;
	uint64_t numBytes = (AVGDIRENTRIES * sizeof(entry));
	uint64_t rootDirBlocks = ((numBytes + (BLOCK_SIZE - 1)) / BLOCK_SIZE);
	uint64_t numDirEntries = (rootDirBlocks * BLOCK_SIZE) / sizeof(entry);

	startPartitionSystem(fileName, volSizePtr, blockSizePtr);
	
	//init VCB, entrylist, and bitmap
	volumeEntry *vcb = malloc(BLOCK_SIZE);
	entry *entryList = malloc(rootDirBlocks * BLOCK_SIZE); 
	entry entries[numDirEntries];
	char *bitMapBuf = malloc((((sizeof(char) * LBA_COUNT) / BLOCK_SIZE) + 1) * BLOCK_SIZE);
	init(vcb, bitMapBuf, entries, entryList, fileName, rootDirBlocks, numDirEntries);
	init(vcb, bitMapBuf, entries, entryList, fileName, rootDirBlocks, numDirEntries);
	int currentDirIndex = 0;


	char line[1024];
	char *args[500];
	
	//main testdriver loop
	while (1)
	{
		printf("\nThe current directory is: %s\n", (entryList + currentDirIndex)->name);

		if (argv[1] == NULL)
			argv[1] = ">";

		// reading user line
		if (fgets(line, sizeof(line), stdin) == NULL)
		{
			exit(EXIT_SUCCESS);
		}

		line[strlen(line) - 1] = '\0'; // removes '\n' from fgets

		//error message for empty line
		if (strcmp(line, "") == 0)
		{
			printf("Error: Empty line. Try again. \n");
			continue;
		}
		if (strcmp(line, " ") == 0)
		{
			printf("command not found.\n");
			continue;
		}
		//when user decides to exit
		if (!strcmp(line, "exit"))
		{
			free(vcb);
			free(entryList);
			free(bitMapBuf);
			printf("Exiting...\n");
			closePartitionSystem();
			return 0;
		}

		int i = 0;
		args[0] = strtok(line, " ");

		while (args[i] != NULL)
		{
			args[++i] = strtok(NULL, " ");
		}

		if (strcmp(args[0], "ls") == 0)
		{
			listDir(currentDirIndex, entryList, numDirEntries);
		}
		else if (strcmp(args[0], "cd") == 0)
		{
			int indexVal = changeDir(args, currentDirIndex, entryList, numDirEntries);
			if (indexVal != -1)
				currentDirIndex = indexVal;
		}
		else if (strcmp(args[0], "cd..") == 0)
		{
			previousDir(args, &currentDirIndex, entryList, numDirEntries);
		}
		else if (strcmp(args[0], "mkdir") == 0)
		{
			makeDir(args, currentDirIndex, entryList, bitMapBuf);
		}
		else if (strcmp(args[0], "rmdir") == 0)
		{
			rmDir(args, currentDirIndex, entryList, bitMapBuf, numDirEntries);
		}
		else if (strcmp(args[0], "rm") == 0)
		{
			rmFile(args, currentDirIndex, entryList, bitMapBuf, numDirEntries);
		}
		else if (strcmp(args[0], "addFile") == 0)
		{
			addFile(args, currentDirIndex, entryList, bitMapBuf);
		}
		else if (strcmp(args[0], "cp") == 0)
		{
			cpFile(args, currentDirIndex, entryList, bitMapBuf, numDirEntries);
		}
		else if (strcmp(args[0], "mv") == 0)
		{
			mvFile(args, currentDirIndex, entryList, bitMapBuf, numDirEntries);
		}
		else if (strcmp(args[0], "cpn") == 0)
		{
			copyNormaltoCurrent(args, currentDirIndex, entryList, bitMapBuf);
		}
		else if (strcmp(args[0], "cpc") == 0)
		{
			copyCurrenttoNormal(args, fileName, entryList, numDirEntries, currentDirIndex);
		}
		else
		{
			printf("command not found.\n");
		}
	}

	printf("Exiting...\n");
	closePartitionSystem();
	free(fileName);

	return 0;
}

void listDir(int currentDirIndex, entry *entryList, int size)
{
	int entryCount = 0;
	for (int i = 0; i < size; i++)
	{
		if ((entryList + i)->parent == currentDirIndex && (entryList + i)->bitMap != ENTRYFLAG_UNUSED)
		{
			printf("%s ", (entryList + i)->name);
			entryCount++;
			if (entryCount % 4 == 0)
				printf("\n");
		}
	}
	printf("\n");
}

int changeDir(char *args[], int currentDirIndex, entry *entryList, uint64_t size)
{
	int result = -1;
	int prevResult = -1;

	//Note that "cd " returns args[1] = null
	if (args[1] == NULL)
	{
		args[1] = "Root";
	}

	for (int i = 0; i < size; i++)
	{
		if ((strcmp(args[1], (entryList + i)->name) == 0) && ((entryList + i)->bitMap == ENTRYFLAG_DIR))
		{
			//Check if the entered directory is the child of the current directory
			if ((entryList + i)->parent == (entryList + currentDirIndex)->index)
			{
				result = i;
				if (i == -1)
				{
					prevResult = -1;
				}
				else
				{
					prevResult = i - 1;
				}
			}
			//Check if the entered directory is the parent of the current directory
			if ((entryList + i)->index == (entryList + currentDirIndex)->parent)
			{
				result = i;
				if (i == -1)
				{
					prevResult = -1;
				}
				else
				{
					prevResult = i - 1;
				}
			}
		}
	}
	return result;
}

void previousDir(char *args[], int * currentDirIndex, entry *entryList, uint64_t size)
{
	if(*currentDirIndex != 0)
	{
		*currentDirIndex = ((entryList + *currentDirIndex)->parent);
	}
}

void makeDir(char *args[], int currentDirIndex, entry *entryList, char *bitMap)
{
	if (args[1] == NULL)
	{
		printf("Specify name of directory to add.");
	}

	writeDirectoryToVolume(args[1], currentDirIndex, ENTRYFLAG_DIR, entryList, bitMap);
}

void rmDir(char *args[], int currentDirIndex, entry *entryList, char *bitMap, uint64_t numDirEntries)
{
	if (args[1] == NULL)
	{
		printf("Specify name of file to delete.\n");
	}

	int index = -1;

	for (int i = 0; i < numDirEntries; i++)
	{
		if ((strcmp(args[1], (entryList + i)->name) == 0) && (((entryList + i)->parent) == currentDirIndex))
		{
			index = i;
		}
	}

	if (index == -1)
	{
		printf("Directory not found.\n");
	}
	else
	{
		for (int i = 0; i < numDirEntries; i++)
		{
			if ((entryList + i)->parent == (entryList + index)->index)
			{
				deleteFromVolume(i, entryList, bitMap);
			}
		}
		deleteDirectoryFromVolume(index, entryList, bitMap);
	}
}

void rmFile(char *args[], int currentDirIndex, entry *entryList, char *bitMap, uint64_t numDirEntries)
{
	if (args[1] == NULL)
	{
		printf("Specify name of file to delete.\n");
	}
	else
	{
		int index = -1;

		for (int i = 0; i < numDirEntries; i++)
		{
			if ((strcmp(args[1], (entryList + i)->name) == 0) && (((entryList + i)->parent) == currentDirIndex))
			{
				index = i;
			}
		}

		if (index == -1)
		{
			printf("File not found.\n");
		}
		else
		{
			deleteFromVolume(index, entryList, bitMap);
		}
	}
}

void addFile(char *args[], int currentDirIndex, entry *entryList, char *bitMap)
{

	if (args[1] == NULL)
	{
		printf("Specify name of file to add, followed by the text input for your file.\n");
	}
	else if (args[2] == NULL)
	{
		printf("Specify name of file to add, followed by the text input for your file.\n");
	}
	else
	{
		//Get initial characters before first space
		char *buf = malloc(strlen(args[2]+1));
		strcpy(buf, args[2]);
		
		//Get number of arguments for for loop
		uint64_t count = 0; 
		while(args[count]){
			count++;
		}
		
		// For loop to account for spaces in input
		for (int i=3; i < count; i++)
		{
			buf = realloc(buf, (strlen(buf) + strlen(args[i])));
			strcat(buf, " ");
			strcat(buf, args[i]);
		}
		//Malloc a new buffer since junk is written if we just realloc buf
		char * finalBuf = malloc((((strlen(buf) / BLOCK_SIZE) + 1) * BLOCK_SIZE));
		stpcpy(finalBuf, buf);
		
		uint64_t inputSize = strlen(finalBuf);
		
		writeToVolume(finalBuf, args[1], inputSize, currentDirIndex, ENTRYFLAG_FILE, entryList, bitMap);
		// Free the buffers
		free(buf);
		free(finalBuf);
	}
}

int cpFile(char *args[], int currentDirIndex, entry *entryList, char *bitMap, uint64_t numDirEntries)
{
	int fileInCurrentDir = 0;
	int dirExists = 0;
	int result = -2;
	int indexOfDestinationDir = -1;
	int writeDir = 0;

	if (args[1] == NULL)
	{
		printf("Error: must add file name to copy followed by the file to store copied data\n");
	}
	else if (args[2] == NULL)
	{
		printf("Error: must add file name to copy followed by the file to store copied data\n");
	}
	else
	{
		for (int i = 0; i < numDirEntries; i++)
		{
			if (((entryList + i)->bitMap == ENTRYFLAG_DIR) && ((entryList + i)->parent == (entryList + currentDirIndex)->index) && strcmp((entryList + i)->name, args[1]) == 0)
			{

				for (int j = 0; j < numDirEntries; j++)
				{
					if ((entryList + i)->index == (entryList + j)->parent)
					{
						printf("You can only use this command on a directory that is empty.\n");
						result = -2;
						return result;
					}
				}
				fileInCurrentDir = 1;
				writeDir = 1;
				result = i;
			}
			else if (((entryList + i)->parent == (entryList + currentDirIndex)->index) && strcmp((entryList + i)->name, args[1]) == 0)
			{
				fileInCurrentDir = 1;
				result = i;
				break;
			}
		}
		if (fileInCurrentDir == 0)
		{
			printf("There is no file in current directory with that name.\n");
		}

		for (int i = 0; i < numDirEntries; i++)
		{
			//check for directory specified
			if ((strcmp(args[2], (entryList + i)->name) == 0) && ((entryList + i)->bitMap == ENTRYFLAG_DIR))
			{
				dirExists = 1;
				indexOfDestinationDir = (entryList + i)->index;
				for (int j = 0; j < numDirEntries; j++)
				{
					if (((entryList + j)->parent == (entryList + i)->index) && strcmp((entryList + j)->name, args[1]) == 0)
					{
						printf("There's already a file in destination directory with the same name.\n");
						result = -2;
						break;
					}
				}
			}
		}
		if (dirExists == 0)
		{
			result = -2;
			printf("The destination directory specified does not exist.\n");
		}

		else if (result != -2)
		{
			int size = (((entryList + result)->count) * BLOCK_SIZE) - 1;
			char *name = (entryList + result)->name;

			//read in file specified
			void *buffer = malloc(((size / BLOCK_SIZE) + 1) * BLOCK_SIZE);
			buffer = readFromVolume((entryList + result)->name, entryList, numDirEntries, currentDirIndex);
			if (writeDir == 0)
			{
				//write a 'copy' to the destination directory
				writeToVolume(buffer, args[1], size, indexOfDestinationDir, ENTRYFLAG_FILE, entryList, bitMap);
			}
			else
			{
				writeDirectoryToVolume(args[1], indexOfDestinationDir, ENTRYFLAG_DIR, entryList, bitMap);
			}
			free(buffer);
		}

		//this is the index of the orginal file that move will used to delete the old file
		return result;
	}
}

void mvFile(char *args[], int currentDirIndex, entry *entryList, char *bitMap, uint64_t numDirEntries)
{
	int indexOfFileToDel = cpFile(args, currentDirIndex, entryList, bitMap, numDirEntries);

	if (indexOfFileToDel != -2)
	{
		//delete the previous entry
		deleteFromVolume((entryList + indexOfFileToDel)->index, entryList, bitMap);
	}
}

void copyNormaltoCurrent(char *args[], int currentDirIndex, entry *entryList, char *bitMap)
{
	uint64_t normalFd_p, fdSize, buffSize;

	normalFd_p = open(args[1], O_RDONLY);

	//checks if file to read and the file to write on exists
	if (normalFd_p < 0)
	{
		printf("Error opening file\n");
		if (args[2] == NULL)
		{
			printf("Error: argument invalid. file to write on unavailable.\n");
		}
	}
	else if (args[2] == NULL)
	{

		printf("Error: argument invalid. file to write on unavailable.\n");
	}
	else
	{
		buffSize = lseek(normalFd_p, 0, SEEK_END); //taking offset size from where normalfd ptr is (starting point) to end of file
		lseek(normalFd_p, 0, SEEK_SET); //bringing ptr back to begginging
		
		void * buffer = malloc(buffSize);

		fdSize = read(normalFd_p, buffer, buffSize);

		close(normalFd_p);

		writeToVolume(buffer, args[2], fdSize, currentDirIndex, ENTRYFLAG_FILE, entryList, bitMap);
	}
}

void copyCurrenttoNormal(char *args[], char fileName[], entry *entryList, uint64_t numDirEntries, int currentDirIndex)
{
	if (args[1] == NULL)
	{
		printf("Error: argument invalid. file to copy unavailable.\n");
	}
	else if (args[2] == NULL)
	{

		printf("Error: argument invalid. file to write on unavailable.\n");
	}
	else
	{
		int index = -1;

		for (int i = 0; i < numDirEntries; i++)
		{
			if ((strcmp(args[1], (entryList + i)->name) == 0) && (((entryList + i)->parent) == currentDirIndex))
			{
				index = i;
			}
		}

		if (index == -1)
		{
			printf("File not found.\n");
		}
		else
		{
			uint64_t fileSize = (((entryList + index)->count) * BLOCK_SIZE);

			void *currentBuff = readFromVolume(args[1], entryList, numDirEntries, currentDirIndex);

			FILE *fp;
			//Make sure you have /tmp directory available.
			char *tmpDir = "/tmp/";
			char fname[strlen(tmpDir) + strlen(args[2]) + 1]; //name of file added to normalfs
			sprintf(fname, "%s%s", tmpDir, args[2]);

			struct stat statBuffer;
			int exist = stat(fname, &statBuffer);

			if (exist == 0)
			{
				printf("%s file already exists. Choose a different name.\n", args[2]);
			}
			else
			{

				fp = fopen(fname, "w+");
				//    fprintf(fp, "This is testing for fprintf...\n");
				if (fwrite(currentBuff, fileSize, 1,fp) < 0)
				{
					printf("Error: Failed to  write on Normal File System");
				}

				fclose(fp);
			}
			free(currentBuff);
		}		
	}
}