#include <stdio.h>
#include <time.h>
#include <windows.h>

int main() {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[100];
    
    while (1) {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        // Format nicely
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %I:%M:%S %p", timeinfo);
        
        printf("\r%s", buffer);
        fflush(stdout);
        
        Sleep(1000);
    }
    
    return 0;
}
