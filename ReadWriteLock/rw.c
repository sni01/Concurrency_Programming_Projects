/*
 * Read and Write function
 */
 #include <stdio.h>
 #include <stdlib.h>

 int main(int argc, char* argv[]){
 	/* declaration */
 	char *buffer;
 	FILE *input;
 	FILE *output;
 	size_t characters;
 	size_t bufsize;

 	if(argc != 3){
 		fprintf(stderr, "wrong args number"), exit(0);
 		exit(1);
 	}

 	/* initialization */
 	bufsize = 1000;
 	buffer = (char *)malloc(bufsize * sizeof(char));

 	input = fopen(argv[1], "r");
 	output = fopen(argv[2], "wr");

 	if(input == NULL || output == NULL){
 		fprintf(stderr, "open files error"), exit(0);
 	}

 	while((characters = getline(&buffer, &bufsize, input)) != -1){
 		fwrite(buffer, sizeof(char), characters, output);
 	}

 	fclose(input);
 	fclose(output);

 	return 0;
 }

