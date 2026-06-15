#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    char buffer[256];
    char *token;
    int token_count = 0;
    
    // Clear the screen
    system("cls");
    
    // Print the prompt at the top
    printf("some-thing-or-other/> ");
    fflush(stdout);
    
    // Read user input
    fgets(buffer, sizeof(buffer), stdin);
    
    // Remove trailing newline
    buffer[strcspn(buffer, "\n")] = 0;
    
    // Print what was stored in buffer
    printf("\nYou entered: \"%s\"\n", buffer);
    
    // Parse by spaces
    printf("\nParsed tokens:\n");
    token = strtok(buffer, " ");
    while (token != NULL) {
        printf("  [%d] %s\n", token_count++, token);
        token = strtok(NULL, " ");
    }
    
    printf("\nTotal tokens: %d\n", token_count);
    
    return 0;
}
