#include <stdio.h>
#include <stdlib.h>
#include "display.h"

#define BOARD_HEIGHT 8
#define BOARD_WIDTH 7
#define MOVES_MAX 50
#define RECT_SIZE 70

enum movement {up, down, left, right}; 
typedef enum movement movement;

typedef struct Node {
    int board[BOARD_HEIGHT][BOARD_WIDTH];
    struct Node *parent;
    struct Node *next;
    struct Node *previous;
    int identifier;
} Node;

typedef struct move {
    int board[BOARD_HEIGHT][BOARD_WIDTH];
} move;

void populate_board (int board_array [BOARD_HEIGHT][BOARD_WIDTH]);
void print_board (int board_array[BOARD_HEIGHT][BOARD_WIDTH]);
void print_board_SDL (int board_array[BOARD_HEIGHT][BOARD_WIDTH], window *w, SDL_Rect *rectangle);
int valid_move (movement direction, int i, int j, int board_array [BOARD_HEIGHT][BOARD_WIDTH]);
void move_piece (movement direction, int i, int j, int board_array [BOARD_HEIGHT][BOARD_WIDTH]);
Node *allocate_start_node (int initial_board[BOARD_HEIGHT][BOARD_WIDTH]);
Node *allocate_new_node (Node *parent_node, Node *previous);
void copy_board (int copy[BOARD_HEIGHT][BOARD_WIDTH], int original[BOARD_HEIGHT][BOARD_WIDTH]);
int duplicate_board (int previous_board[BOARD_HEIGHT][BOARD_WIDTH], int board_to_check[BOARD_HEIGHT][BOARD_WIDTH]);
int duplicate_search (Node *list_element, int board_to_check [BOARD_HEIGHT][BOARD_WIDTH]);
int find_next_move (Node *start, int previous_board[BOARD_HEIGHT][BOARD_WIDTH], int new_board[BOARD_HEIGHT][BOARD_WIDTH]);
int check_move (movement direction, int i, int j, Node *start, int previous_board[BOARD_HEIGHT][BOARD_WIDTH], int new_board[BOARD_HEIGHT][BOARD_WIDTH]);
void print_list (Node *list_element);
void print_list_SDL (Node *list_element);
int populate_moves_array (Node *list_element, move *moves_array);
void print_moves (move *moves_array, int move_count);
void print_moves_SDL (move *moves_array, int move_count);
int check_for_target (int board[BOARD_HEIGHT][BOARD_WIDTH], int height_coordinate, int width_coordinate);
void free_list (Node *list_element);
move * create_move_array (int size);
void run (int height_coordinate, int width_coordinate);
int allocate_board_identifier (int board[BOARD_HEIGHT][BOARD_WIDTH]);

// To include:
    //free mallocs in linkedlist, messsage if solution not found, change how co-ordinates handled based on forum, SDL implementation,
    //parcel find next move function into smaller functions, work out how to steps backwards (Array?). Checks for coordinate input

    //speeded up version: check duplicates more efficiently. Search from parent node only. Hashing??? To show preferential boards?


int main (int argc, char const *argv[]){

    int width_coordinate, height_coordinate;
    //get target co-ordinates from argv
    sscanf (argv[1],"%d", &width_coordinate);
    sscanf (argv[2],"%d", &height_coordinate);
    
    run (height_coordinate, width_coordinate);

    return 0;
}

//sets up initial board
void populate_board (int board_array [BOARD_HEIGHT][BOARD_WIDTH]){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            if (i < 4){
                board_array[i][j] = 0;
            }
            else {
                board_array[i][j] = 1;
            }
        }
    }
}

void print_board (int board_array[BOARD_HEIGHT][BOARD_WIDTH]){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){  
            printf("%d", board_array[i][j]);            
        }
    printf("\n");
    }   
}

void print_board_SDL (int board_array[BOARD_HEIGHT][BOARD_WIDTH], window *w, SDL_Rect *rectangle){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            check_for_quit (w);
            if (w->finished == 1){
                SDL_Quit();
            }
            rectangle->x = j * RECT_SIZE;
            rectangle->y = i * RECT_SIZE;            
            if (board_array[i][j] == 1){
                SDL_SetRenderDrawColor(w->renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
            }
            else if (board_array[i][j] == 0){
                SDL_SetRenderDrawColor(w->renderer, 250, 250, 250, SDL_ALPHA_OPAQUE);
            }
            SDL_RenderFillRect(w->renderer, rectangle);
        }
    }
}

// checks a move from a specified co-ordinate in array to see whether it's valid. Returns a 1 if valid, 0 if not
int valid_move (movement direction, int i, int j, int board_array [BOARD_HEIGHT][BOARD_WIDTH]){

    switch (direction){
        case up:
            if (i != 0 && board_array[i-1][j] == 1 && board_array[i-2][j] == 0){
                return 1;
            }
            else {
                return 0;
            }
        case right:
            if (j <= (BOARD_WIDTH-3) && board_array[i][j+1] == 1 && board_array[i][j+2] == 0){
                return 1;
            }
            else {
                return 0;
            }
        case left:
            if (j != 1 && board_array[i][j-1] == 1 && board_array[i][j-2] == 0){
                return 1;
            }
            else {
                return 0;
            }
        case down:
            if (i <= (BOARD_HEIGHT-3) && board_array[i+1][j] == 1 && board_array[i+2][j] == 0){
                return 1;
            }
            else {
                return 0;
            }
        default:
            printf("Something wrong with check_valid_move switch statement\n");
    }
    return 0;
}

void move_piece (movement direction, int i, int j, int board_array [BOARD_HEIGHT][BOARD_WIDTH]){

        switch (direction){
        case up:
            board_array[i-2][j] = 1;
            board_array[i-1][j] = 0;
            board_array[i][j] = 0;
            break;
        case right:
            board_array[i][j+2] = 1;
            board_array[i][j+1] = 0;
            board_array[i][j] = 0;
            break;
        case left:
            board_array[i][j-2] = 1;
            board_array[i][j-1] = 0;
            board_array[i][j] = 0;
            break;
        case down:
            board_array[i+2][j] = 1;
            board_array[i+1][j-1] = 0;
            board_array[i][j] = 0;
            break;
        default:            
            printf("Something wrong with move_piece switch statement\n");
    }
}

Node *allocate_start_node (int initial_board[BOARD_HEIGHT][BOARD_WIDTH]){
    Node *p_node = (Node *) malloc (sizeof (Node));
    if (p_node == NULL){
        printf("Malloc in allocate_node didn't work\n");
        exit (1);
    }
    p_node->parent = NULL;
    p_node->next = NULL;
    p_node->previous = NULL;
    copy_board (p_node->board, initial_board);
    return p_node;
}

//Allocates space for a new node, assigns it a previous and parent pointer
Node *allocate_new_node (Node *parent_node, Node *previous){
    Node *p_node = (Node *) malloc (sizeof (Node));
    if (p_node == NULL){
        printf("Malloc in allocate_node didn't work\n");
        exit (1);
    }
    p_node->parent = parent_node;
    p_node->next = NULL;
    p_node->previous = previous;
    copy_board (p_node->board, parent_node->board);
    return p_node;
}

void copy_board (int copy[BOARD_HEIGHT][BOARD_WIDTH], int original[BOARD_HEIGHT][BOARD_WIDTH]){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            copy[i][j] = original[i][j];
        }
    }
}

//checks if a particular board is the same as a specific board in the list. Returns 1 if duplicate, 0 if not
int duplicate_board (int previous_board[BOARD_HEIGHT][BOARD_WIDTH], int board_to_check[BOARD_HEIGHT][BOARD_WIDTH]){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            if (board_to_check[i][j] != previous_board[i][j]){
                return 0;
            }
        }
    }
    return 1;
}

//checks existing list for duplicate boards. Returns 1 if duplicate found, 0 if not
int duplicate_search (Node *list_element, int board_to_check [BOARD_HEIGHT][BOARD_WIDTH]){
    while (list_element->next != NULL) {
        if (duplicate_board (list_element->board, board_to_check) == 1){
            return 1;
        }
        list_element = list_element->next;
    }
    return 0;
}

//checks for next moves. If viable move found that doesn't lead to a duplicate board, new board shows this move and 1 is returned 
int find_next_move (Node *start, int previous_board[BOARD_HEIGHT][BOARD_WIDTH], int new_board[BOARD_HEIGHT][BOARD_WIDTH]){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            
            if (new_board[i][j] == 1){

                if (check_move (up, i, j, start, previous_board, new_board)){
                    return 1;
                }
                if (check_move (right, i, j, start, previous_board, new_board)){
                    return 1;
                }
                if (check_move (left, i, j, start, previous_board, new_board)){
                    return 1;
                }
                if (check_move (down, i, j, start, previous_board, new_board)){
                    return 1;
                }
            }
        }
    }   
    return 0;
}

//checks a specific move from specific location on the board. If move viable and doesn't lead to duplicate board, 
//new board shows this move and 1 is returned. If no moves found, returns 0
int check_move (movement direction, int i, int j, Node *start, int parent_board [BOARD_HEIGHT][BOARD_WIDTH], int new_board[BOARD_HEIGHT][BOARD_WIDTH]){
    if (valid_move (direction, i, j, new_board)){
        move_piece (direction, i, j, new_board);
        if (duplicate_search (start, new_board) == 1){
            copy_board (new_board, parent_board);
        }
        else{
            return 1;
        }
    }
    return 0;
}

void print_list (Node *list_element){
    while (list_element != NULL){
        print_board (list_element->board);
        printf("Hash number = %d\n", list_element->identifier);
        printf("\n");
        list_element = list_element->next;
    }
}

void print_list_SDL (Node *list_element){
    window w;
    setup_SDL (&w, (BOARD_HEIGHT * RECT_SIZE), (BOARD_WIDTH * RECT_SIZE));
    SDL_Rect rectangle;
    rectangle.w = RECT_SIZE;
    rectangle.h = RECT_SIZE;
    while (list_element != NULL){
        print_board_SDL (list_element->board, &w, &rectangle);
        SDL_RenderPresent(w.renderer);
        SDL_UpdateWindowSurface(w.win);
        SDL_Delay (250);
        list_element = list_element->next;
    }
}

//fills an array of structs with solution boards. Returns the position of the final board in the array
int populate_moves_array (Node *list_element, move *moves_array){
    //move_count initially set to 1 to leave room for start board in array
    int move_count = 1;
    while (list_element->previous != NULL){
        copy_board (moves_array->board, list_element->board);
        if (list_element->parent == list_element->previous->parent){
            list_element = list_element->previous;
        }
        else {
            list_element = list_element->parent;
        }
        move_count++;
        moves_array++;
    }
    return move_count;
}

void print_moves (move *moves_array, int move_count){
    for (int i = move_count-1; i >= 0; --i){
        print_board ((moves_array+i)->board);
        printf("\n");
    }
}

void print_moves_SDL (move *moves_array, int move_count){
    window w;
    setup_SDL (&w, (BOARD_HEIGHT * RECT_SIZE), (BOARD_WIDTH * RECT_SIZE));
    SDL_Rect rectangle;
    rectangle.w = RECT_SIZE;
    rectangle.h = RECT_SIZE;
    for (int i = move_count-1; i >= 0; --i){   
        print_board_SDL ((moves_array+i)->board, &w, &rectangle);
        SDL_RenderPresent(w.renderer);
        SDL_UpdateWindowSurface(w.win);
        SDL_Delay (250);
    }
}

//checks to see if board has a peg at target location. Returns 1 if target has peg, otherwise 0
int check_for_target (int board[BOARD_HEIGHT][BOARD_WIDTH], int height_coordinate, int width_coordinate){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            if (board[i][j] == 1 && i == height_coordinate && j == width_coordinate){
                return 1;
            }
        }
    }
    return 0;
}

//frees memory allocated to linked list
void free_list (Node *list_element){
    Node *temp;
    while (list_element != NULL){
        temp = list_element;
        list_element = list_element->next;
        free (temp);
        temp = NULL;
    }
}

move * create_move_array (int size){
    move *p_move_array = (move *) malloc (sizeof (move) * size);
    if (p_move_array == NULL){
        printf("problem with malloc in create_move_array\n");
        exit (1);
    }
    else return p_move_array;
}

//finds route to target location and returns a pointer to the board that found it
Node* find_solution (Node *start, int height_coordinate, int width_coordinate){
    Node *current, *previous, *parent;
    int target_found = 0;

    current = start;    
    parent = start;

    // window w;
    // setup_SDL (&w, (BOARD_HEIGHT * RECT_SIZE), (BOARD_WIDTH * RECT_SIZE));
    // SDL_Rect rectangle;
    // rectangle.w = RECT_SIZE;
    // rectangle.h = RECT_SIZE;

    while (!target_found){
        previous = current;
        current = current->next;
        current = allocate_new_node (parent, previous);
        if (find_next_move (start, previous->board, current->board) == 0){
            if (parent->next == NULL){
                printf("Couldn't find a solution\n");
                exit (1);
            }

            // print_board_SDL (current->board, &w, &rectangle);
            // SDL_RenderPresent(w.renderer);
            // SDL_UpdateWindowSurface(w.win);
            parent = parent->next;
            current->parent = parent;
            copy_board (current->board, parent->board);
    
        }
        current->identifier = allocate_board_identifier (current->board);
        target_found = check_for_target (current->board, height_coordinate, width_coordinate);
        current = current->next;
    }
    return previous;
}

void run (int height_coordinate, int width_coordinate){

    move *moves_array = create_move_array (MOVES_MAX);
    Node *final_board, *start;
    int move_count = 0, target_found = 0;
    int initial_board [BOARD_HEIGHT][BOARD_WIDTH];

    populate_board (initial_board);
    start = allocate_start_node (initial_board);
    target_found = check_for_target (initial_board, height_coordinate, width_coordinate);

    if (!target_found){
        final_board = find_solution (start, height_coordinate, width_coordinate);
        move_count = populate_moves_array (final_board, moves_array);
        //to copy start board into last element of moves array for printing
        copy_board ((moves_array+(move_count-1))->board, start->board);
        // print_moves_SDL (moves_array, move_count);
        // print_moves (moves_array, move_count);
    }

    else {
        print_list (start);
    }

    print_list (start);

    // print_list_SDL (start);

    free_list (start);
    free (moves_array);
}

//generates random number for each unique board and returns it
int allocate_board_identifier (int board[BOARD_HEIGHT][BOARD_WIDTH]){
    int x_val = 1;
    int y_val = 1;
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            if (board[i][j] == 1){
                x_val += ((i+5) * (j+3) * 2789);
            }
            else {
                y_val += ((i+7) * (j+11) * 3229);
            }
        }
    }
    // printf("x_val = %d y_val = %d\n", x_val, y_val);
    // printf("hash = %d\n", (x_val * y_val));
    return (x_val * y_val);
}