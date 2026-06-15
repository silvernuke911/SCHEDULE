#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 

int main()
{   
	char line[1024];
    char *token;
    
    printf("Enter input: ");
    fgets(line, sizeof(line), stdin);
    
    // Remove trailing newline
    line[strcspn(line, "\n")] = 0;
    
    // Split by spaces
    token = strtok(line, " ");
    while (token != NULL) {
        printf("Token: %s\n", token);
        token = strtok(NULL, " ");
    }
	
	return 0;
}
