//To quit SDL use cross in top left corner of window

#include <stdio.h>
#include <stdlib.h>
#include "display.h"

#define BOARD_HEIGHT 8
#define BOARD_WIDTH 7
#define MOVES_MAX 50
#define RECT_SIZE 70

typedef int Board[BOARD_HEIGHT][BOARD_WIDTH];

enum movement {up, down, left, right}; 
typedef enum movement movement;

typedef struct Node {
    Board board;
    struct Node *parent;
    struct Node *next;
    struct Node *previous;
} Node;

typedef struct move {
    int board[BOARD_HEIGHT][BOARD_WIDTH];
} move;

////////////////////////////////FUNCTIONS///////////////////////////////////////

//Run program
void run (int height_coordinate, int width_coordinate);

//Get target co-ordinates from argv
void get_target_coordinates (int argc, char const *argv[], int *width_coordinate, int *height_coordinate);
int array_height_coordinate (int height_coordinate);

//Set up initial boards and print boards
void populate_board (Board board_array);
void print_board (Board board_array);
void print_board_SDL (Board board_array, window *w, SDL_Rect *rectangle);

//Find solution
Node* find_solution (Node *start, int height_coordinate, int width_coordinate);
int check_move (movement direction, int i, int j, Node *start, Board parent_board, Board new_board);
int find_next_move (Node *start, Board parent_board, Board new_board);
int valid_move (movement direction, int i, int j, Board board_array);
void move_piece (movement direction, int i, int j, Board board_array);
int duplicate_board (Board previous_board, Board board_to_check);
int duplicate_search (Node *list_element, Board board_to_check);

//Add boards to linked list
Node * allocate_start_node (Board initial_board);
Node * allocate_new_node (Node *parent_node, Node *previous);
void copy_board (Board copy, Board original);


//Check for target and show solution
int populate_moves_array (Node *list_element, move *moves_array);
void print_list (Node *list_element);
void print_list_SDL (Node *list_element);
void print_moves (move *moves_array, int move_count);
void print_moves_SDL (move *moves_array, int move_count);
int check_for_target (Board board, int height_coordinate, int width_coordinate);
move * create_move_array (int size);
void free_list (Node *list_element);

///////////////////////////MAIN/////////////////////////////////////////////////

int main (int argc, char const *argv[]){

    int width_coordinate, height_coordinate;

    get_target_coordinates (argc, argv, &width_coordinate, &height_coordinate);
    
    run (height_coordinate, width_coordinate);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

void run (int height_coordinate, int width_coordinate){

    move *moves_array = create_move_array (MOVES_MAX);
    Node *final_board, *start;
    int move_count = 0, target_found = 0;
    Board initial_board;

    populate_board (initial_board);
    start = allocate_start_node (initial_board);
    target_found = check_for_target (initial_board, height_coordinate, width_coordinate);
    //If target not on initial board, then look for it
    if (!target_found){
        final_board = find_solution (start, height_coordinate, width_coordinate);
        move_count = populate_moves_array (final_board, moves_array);
        //Below is to copy start board into last element of moves array for printing
        copy_board ((moves_array+(move_count-1))->board, start->board);
        print_moves_SDL (moves_array, move_count);
    }
    //If target already on inital board
    else {
        print_list_SDL (start);
        printf("target already on board\n");
    }

    free_list (start);
    free (moves_array);
}

//takes width and height values entered at argv[1] and argv[2] and checks the inputs are sensible. Exits program if not
void get_target_coordinates (int argc, char const *argv[], int *width_coordinate, int *height_coordinate){
    if (argc < 3 || argc > 3){
        printf("you need to check how you're entering the target co-ordinates\n");
        printf("The program should be run in the format './program_name width height'\n");
        exit(1);
    }
    sscanf (argv[1],"%d", width_coordinate);
    sscanf (argv[2],"%d", height_coordinate);
    *height_coordinate = array_height_coordinate (*height_coordinate);
    if (*width_coordinate > 6 || *width_coordinate < 0){
        printf("width co-ordinate must be between 0 and 6\n");
        exit (1);
    }
    if (*height_coordinate > 7 || *height_coordinate < 0){
        printf("height co-ordinate must be between 0 and 7\n");
        exit (1);
    }
}

//converts user entered height co-ordinate into array height for use in rest of program
int array_height_coordinate (int height_coordinate){
    int array_height_coordinate;
    array_height_coordinate = ((0 - height_coordinate) + (BOARD_HEIGHT-1));
    return array_height_coordinate;
}

//sets up initial board
void populate_board (Board board_array){
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

void print_board (Board board_array){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){  
            printf("%d", board_array[i][j]);            
        }
    printf("\n");
    }   
}

void print_board_SDL (Board board_array, window *w, SDL_Rect *rectangle){
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

//finds route to target location and returns a pointer to the board that found it
Node* find_solution (Node *start, int height_coordinate, int width_coordinate){
    Node *current, *previous, *parent;
    int target_found = 0;

    current = start;    
    parent = start;

    while (!target_found){
        previous = current;
        current->next = allocate_new_node (parent, previous);
        if (find_next_move (start, parent->board, current->next->board) == 0){

            if (parent->next == NULL){
                printf("Couldn't find a solution\n");
                exit (1);
            }
            parent = parent->next;
            current->next->parent = parent;
            copy_board (current->next->board, parent->board);
        }
        target_found = check_for_target (current->next->board, height_coordinate, width_coordinate);
        current = current->next;
    }
    return current;
}

//checks for next moves. If viable move found that doesn't lead to a duplicate board, new board shows this move and 1 is returned 
int find_next_move (Node *start, Board parent_board, Board new_board){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            
            if (new_board[i][j] == 1){

                if (check_move (up, i, j, start, parent_board, new_board)){
                    return 1;
                }
                if (check_move (right, i, j, start, parent_board, new_board)){
                    return 1;
                }
                if (check_move (left, i, j, start, parent_board, new_board)){
                    return 1;
                }
                if (check_move (down, i, j, start, parent_board, new_board)){
                    return 1;
                }
            }
        }
    }   
    return 0;
}

//checks a specific move from specific location on the board. If move viable and doesn't lead to duplicate board, 
//new board shows this move and 1 is returned. If no moves found, returns 0
int check_move (movement direction, int i, int j, Node *start, Board parent_board, Board new_board){
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

// checks a move from a specified co-ordinate in array to see whether it's valid. Returns a 1 if valid, 0 if not
int valid_move (movement direction, int i, int j, Board board_array){

    switch (direction){
        case up:
            if (i >= 2 && board_array[i-1][j] == 1 && board_array[i-2][j] == 0){
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
            if (j >= 2 && board_array[i][j-1] == 1 && board_array[i][j-2] == 0){
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

void move_piece (movement direction, int i, int j, Board board_array){

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

//checks if a particular board is the same as a specific board in the list. Returns 1 if duplicate, 0 if not
int duplicate_board (Board previous_board, Board board_to_check){
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
int duplicate_search (Node *list_element, Board board_to_check){
    while (list_element->next != NULL) {
        if (duplicate_board (list_element->board, board_to_check) == 1){
            return 1;
        }
        list_element = list_element->next;
    }
    return 0;
}

Node *allocate_start_node (Board initial_board){
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

void copy_board (Board copy, Board original){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            copy[i][j] = original[i][j];
        }
    }
}

void print_list (Node *list_element){
    while (list_element != NULL){
        print_board (list_element->board);
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
        SDL_Delay (500);
        list_element = list_element->next;
    }
}

//fills an array of structs with solution boards. Returns the position of the final board in the array
int populate_moves_array (Node *list_element, move *moves_array){
    //move_count initially set to 1 to leave room for start board
    int move_count = 1;
    while (list_element->previous != NULL){
        copy_board (moves_array->board, list_element->board);
        list_element = list_element->parent;
        move_count++;
        moves_array++;
    }
    return move_count;
}

//cycles through moves array backwards to print moves
void print_moves (move *moves_array, int move_count){
    for (int i = move_count-1; i >= 0; --i){
        print_board ((moves_array+i)->board);
        printf("\n");
    }
}

//cycles through moves array backwards to print moves to screen using SDL
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
int check_for_target (Board board, int height_coordinate, int width_coordinate){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            if (board[i][j] == 1 && i == height_coordinate && j == width_coordinate){
                return 1;
            }
        }
    }
    return 0;
}

//allocates space for array of move structs
move * create_move_array (int size){
    move *p_move_array = (move *) malloc (sizeof (move) * size);
    if (p_move_array == NULL){
        printf("problem with malloc in create_move_array\n");
        exit (1);
    }
    else return p_move_array;
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

