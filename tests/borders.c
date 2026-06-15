#include <windows.h>
#include <stdio.h>

void get_terminal_size(int *width, int *height) {

    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(
        GetStdHandle(STD_OUTPUT_HANDLE),
        &csbi
    );

    *width =
        csbi.srWindow.Right -
        csbi.srWindow.Left + 1;

    *height =
        csbi.srWindow.Bottom -
        csbi.srWindow.Top + 1;
}

void draw_border(int width, int height, int sep_bar) {

    for (int y = 0; y < height; y++) {

        for (int x = 0; x < width; x++) {

			if (x == 0 && y == 0) {
				printf("╔");
			}
			else if (x == width - 1 && y == 0) {
				printf("╗");
			}
			else if (x == 0 && y == height - 1) {
				printf("╚");
			}
			else if (x == width - 1 && y == height - 1) {
				printf("╝");
			}

			// separator intersections
			else if (x == 0 && y == height - sep_bar) {
				printf("╠");
			}
			else if (x == width - 1 && y == height - sep_bar) {
				printf("╣");
			}

			// separator line
			else if (y == height - sep_bar) {
				printf("═");
			}

			// top/bottom borders
			else if (y == 0 || y == height - 1) {
				printf("═");
			}

			// side borders
			else if (x == 0 || x == width - 1) {
				printf("║");
			}

			else {
				printf(" ");
			}
        }
    }
}

int main() {
	SetConsoleOutputCP(CP_UTF8);
    int width = 0;
    int height = 0;

    int old_width = -1;
    int old_height = -1;
	
	int sep_bar = 4;
    while (1) {

        get_terminal_size(&width, &height);

        if (width != old_width ||
            height != old_height)
        {
            system("cls");

            draw_border(width, height, sep_bar);

            old_width = width;
            old_height = height;
        }

        Sleep(50);
    }

    return 0;
}
