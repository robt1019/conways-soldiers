//To quit SDL use cross in top left corner of window

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "display.h"

#define BOARD_HEIGHT 8
#define BOARD_WIDTH 7
#define MOVES_MAX 50
#define RECT_SIZE 70
#define HASH_TABLE_SIZE 500000
#define MAX_RANDOM 7500;
#define DELAY 750
#define ARGC_EXPECTED 3
#define BOARD_CENTRE 4
#define ZOBRIST_MULTIPLIER 3

typedef int Board[BOARD_HEIGHT][BOARD_WIDTH];

enum movement {up, down, left, right}; 
typedef enum movement movement;

enum cell_state {dead = 0, alive = 1};
typedef enum cell_state cell_state;

enum solution_type {target, mirror};
typedef enum solution_type solution_type;

typedef struct Node {
    Board board;
    int identifier;
    struct Node *parent;
    struct Node *next;
    struct Node *previous;
} Node;

typedef struct move {
    int board[BOARD_HEIGHT][BOARD_WIDTH];
} move;

typedef struct hash {
    Node *board_ref;
    struct hash *next;
} hash;

void run (int height_coordinate, int width_coordinate);

void get_target_coordinates (int argc, char const *argv[], int *width_coordinate, int *height_coordinate);
int array_height_coordinate (int height_coordinate);

void populate_board (Board board_array);
void print_board (Board board_array);
void print_board_SDL (Board board_array, window *w, SDL_Rect *rectangle);

//EXTENSION FUNCTION
void populate_zobrist_board (Board zobrist_board);

Node* find_solution (Node *start, int height_coordinate, int width_coordinate, int *target_type);
int find_next_move (hash *hash_array, Node *start, Node *latest_node, Board parent_board, Board new_board, Board zobrist_board);
int check_move (movement direction, int i, int j, hash *hash_array, Node *start, Node *latest_node, Board parent_board, Board new_board, Board zobrist_board);
int valid_move (movement direction, int i, int j, Board board_array);
void move_piece (movement direction, int i, int j, Board board_array);
int duplicate_board (Board previous_board, Board board_to_check);

//EXTENSION FUNCTIONS
hash *create_hash_table (int size);
void set_hash_table (hash *hash_array);
int insert_node (hash *hash_array, Node *current, int key, Board board_array);
hash *allocate_new_chain (Node *current);

Node *allocate_start_node (Board initial_board);
Node *allocate_new_node (Node *parent_node, Node *previous);
void copy_board (Board copy, Board original);

//EXTENSION FUNCTIONS
int allocate_board_address (Board board, Board zobrist_board);
void free_hash_table (hash *hash_array);

//EXTENSION FUNCTIONS
void mirror_board (Board board, Board mirrored_board);
int convert_mirror_width (int width_coordinate);

void print_list (Node *list_element);
void print_list_SDL (Node *list_element);
int populate_moves_array (Node *list_element, move *moves_array);

//EXTENSION FUNCTION
int populate_mirror_array (Node *list_element, move *moves_array);

void print_moves (move *moves_array, int move_count);
void print_moves_SDL (move *moves_array, int move_count);
int check_for_target (Board board, int height_coordinate, int width_coordinate);
void free_list (Node *list_element);
move * create_move_array (int size);

int main (int argc, char const *argv[]){

    int width_coordinate, height_coordinate;

    srand (time (NULL));

    get_target_coordinates (argc, argv, &width_coordinate, &height_coordinate);    
    
    run (height_coordinate, width_coordinate);
    
    return 0;
}

void run (int height_coordinate, int width_coordinate){

    move *moves_array = create_move_array (MOVES_MAX);
    Node *final_board, *start;

    int move_count = 0, target_found = 0, target_type = 0;
    int initial_board[BOARD_HEIGHT][BOARD_WIDTH];

    populate_board (initial_board);
    start = allocate_start_node (initial_board);
    target_found = check_for_target (initial_board, height_coordinate, width_coordinate);

    //checks if target is already on the board or not
    if (!target_found){
        final_board = find_solution (start, height_coordinate, width_coordinate, &target_type);
        if (target_type == target){
            move_count = populate_moves_array (final_board, moves_array);
        }
        if (target_type == mirror){
            move_count = populate_mirror_array (final_board, moves_array);
        }
        //to copy start board into last element of moves array for printing
        copy_board ((moves_array+(move_count-1))->board, start->board);
        print_moves_SDL (moves_array, move_count);
        // print_moves (moves_array, move_count);
    }
    else {
        print_list_SDL (start);
        printf("Target already on board\n");
    }
    free_list (start);
    free (moves_array);
}

//finds route to target location and returns a pointer to the board that found it
Node* find_solution (Node *start, int height_coordinate, int width_coordinate, int *target_type){
    hash *hash_array = create_hash_table (HASH_TABLE_SIZE);
    set_hash_table (hash_array);
    int zobrist_board[BOARD_HEIGHT][BOARD_WIDTH];
    populate_zobrist_board (zobrist_board);
    Node *current, *previous, *parent;
    int mirrored_width_coordinate = convert_mirror_width (width_coordinate);

    int target_found = 0;

    current = start;    
    parent = start;

    while (!target_found){
        previous = current;
        current->next = allocate_new_node (parent, previous);
        //if no next move possible from current parent node, move parent node to next node
        if (find_next_move (hash_array, start, current->next, parent->board, current->next->board, zobrist_board) == 0){
            if (parent->next == NULL){
                printf("Couldn't find a solution\n");
                exit (1);
            }
            parent = parent->next;
            current->next->parent = parent;
            copy_board (current->next->board, parent->board);
        }
        //checks for target
        target_found = check_for_target (current->next->board, height_coordinate, width_coordinate);
        if (target_found){
            *target_type = target;
        }
        //checks for mirrored target
        if (!target_found){
            target_found = check_for_target (current->next->board, height_coordinate, mirrored_width_coordinate);
            if (target_found){
                *target_type = mirror;
            }
        }
        current = current->next;
    }
    free_hash_table (hash_array);
    free (hash_array);
    return current;
}

//takes width and height values entered at argv[1] and argv[2] and checks the inputs are sensible. Exits program if not
//height_coordinate and width_coordinate are updated in main
void get_target_coordinates (int argc, char const *argv[], int *width_coordinate, int *height_coordinate){
    if (argc < ARGC_EXPECTED || argc > ARGC_EXPECTED){
        printf("you need to check how you're entering the target co-ordinates\n");
        printf("The program should be run in the format './program_name width height'\n");
        exit(1);
    }
    sscanf (argv[1],"%d", width_coordinate);
    sscanf (argv[2],"%d", height_coordinate);
    *height_coordinate = array_height_coordinate (*height_coordinate);
    if (*width_coordinate >= BOARD_WIDTH || *width_coordinate < 0){
        printf("width co-ordinate must be between 0 and 6\n");
        exit (1);
    }
    if (*height_coordinate >= BOARD_HEIGHT || *height_coordinate < 0){
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
            if (i < BOARD_CENTRE){
                board_array[i][j] = dead;
            }
            else {
                board_array[i][j] = alive;
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

void populate_zobrist_board (Board zobrist_board){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            zobrist_board[i][j] = rand () % MAX_RANDOM;
        }
    }
}

//checks for next moves. If viable move found that doesn't lead to a duplicate board, new board shows this move and 1 is returned 
int find_next_move (hash *hash_array, Node *start, Node *latest_node, Board parent_board, Board new_board, Board zobrist_board){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            
            if (new_board[i][j] == alive){

                if (check_move (up, i, j, hash_array, start, latest_node, parent_board, new_board, zobrist_board)){
                    return 1;
                }
                if (check_move (right, i, j, hash_array, start, latest_node, parent_board, new_board, zobrist_board)){
                    return 1;
                }
                if (check_move (left, i, j, hash_array, start, latest_node, parent_board, new_board, zobrist_board)){
                    return 1;
                }
            }
        }
    }   
    return 0;
}

//checks a specific move from specific location on the board. If move viable and doesn't lead to duplicate board, 
//new board shows this move and 1 is returned. If no moves found, returns 0
int check_move (movement direction, int i, int j, hash *hash_array, Node *start, Node *latest_node, Board parent_board, Board new_board, Board zobrist_board){
    if (valid_move (direction, i, j, new_board)){
        move_piece (direction, i, j, new_board);
        latest_node->identifier = allocate_board_address (new_board, zobrist_board);
        // If a duplicate is found, insert node returns 1, otherwise returns a 0 and inserts board into hash table
        if (insert_node (hash_array, latest_node, latest_node->identifier, new_board) == 1){
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
            if (i >= 2 && board_array[i-1][j] == alive && board_array[i-2][j] == dead){
                return 1;
            }
            else {
                return 0;
            }
        case right:
            if (j <= (BOARD_WIDTH-3) && board_array[i][j+1] == alive && board_array[i][j+2] == dead){
                return 1;
            }
            else {
                return 0;
            }
        case left:
            if (j >= 2 && board_array[i][j-1] == alive && board_array[i][j-2] == dead){
                return 1;
            }
            else {
                return 0;
            }
        case down:
            if (i <= (BOARD_HEIGHT-3) && board_array[i+1][j] == alive && board_array[i+2][j] == dead){
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
            board_array[i-2][j] = alive;
            board_array[i-1][j] = dead;
            board_array[i][j] = dead;
            break;
        case right:
            board_array[i][j+2] = alive;
            board_array[i][j+1] = dead;
            board_array[i][j] = dead;
            break;
        case left:
            board_array[i][j-2] = alive;
            board_array[i][j-1] = dead;
            board_array[i][j] = dead;
            break;
        case down:
            board_array[i+2][j] = alive;
            board_array[i+1][j] = dead;
            board_array[i][j] = dead;
            break;
        default:            
            printf("Something wrong with move_piece switch statement\n");
    }
}

//sets up space for an array of hash structs to use in a chained hash table
hash *create_hash_table (int size){
    hash *p_hash = (hash *) malloc (sizeof (hash) * size);
    if (p_hash == NULL){
        printf("malloc for creating hash table didn't work\n");
        exit (1);
    }
    return p_hash;
}

//sets all next pointers in chained hash table to NULL;
void set_hash_table (hash *hash_array){
    for (int i = 0; i < HASH_TABLE_SIZE; ++i){
        (hash_array+i)->next = NULL;
        (hash_array+i)->board_ref = NULL;
    }
}

//Tries to insert a node at a certain point in hash table. If full, checks if duplicate, if not, puts in next available chain position
//returns a 1 if duplicate found, 0 if not.
int insert_node (hash *hash_array, Node *current, int key, Board new_board){
    hash *chain = (hash_array + key);
    
    if (chain->board_ref == NULL){
        chain->board_ref = current;
        return 0;
    }
    else if (duplicate_board (chain->board_ref->board, new_board)){
        return 1;
    }
    else while (chain != NULL){
        if (chain->next == NULL){
            chain->next = allocate_new_chain (current);
            return 0;
        }
        if (duplicate_board (chain->board_ref->board, new_board)){
            return 1;
        }
        chain = chain->next;
    }
    printf("error with insert_node function\n");
    exit (1);
}

//returns pointers to space for new chain off hash table
hash *allocate_new_chain (Node *current){
    hash *p_chain = (hash *) malloc (sizeof (hash));
    if (p_chain == NULL){
        printf("malloc failed when allocating new chain\n");
        exit (1);
    }
    p_chain->next = NULL;
    p_chain->board_ref = current;
    return p_chain;
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

//generates random address number for each unique board to use in hash table
int allocate_board_address (Board board, Board zobrist_board){
    int board_value = 1;
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            if (board[i][j] == alive){
                board_value += (zobrist_board[i][j] * j+ZOBRIST_MULTIPLIER);
            }
        }
    }
    return board_value;
}

// Frees hash table chained linked list
void free_hash_table (hash *hash_array){
    hash *array_element = NULL;
    hash *temp = NULL;
    for (int i = 0; i < MOVES_MAX; ++i){
        array_element = (hash_array + i); 
        while (array_element->next != NULL){
            temp = array_element->next;
            array_element = array_element->next;
            free (temp);
        }
    }
}

//takes a board and copies a mirror of it into another board
void mirror_board (Board board, Board mirrored_board){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            mirrored_board [i][BOARD_WIDTH - j - 1] = board[i][j];
        }
    }
}

//converts a width co-ordinate into its vertical mirror image on the board
int convert_mirror_width (int width_coordinate){
    int mirror_width;
    mirror_width = (BOARD_WIDTH - width_coordinate - 1);
    return mirror_width;
}

void print_list (Node *list_element){
    while (list_element != NULL){
        print_board (list_element->board);
        printf("\n");
        printf("hash no = %d\n", list_element->identifier);
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
        SDL_Delay (DELAY);
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

//array where mirrored symetrical solution is stored for printing
int populate_mirror_array (Node *list_element, move *moves_array){
    //move_count initially set to 1 to leave room for start board
    int move_count = 1;
    Board mirrored_board;
    while (list_element->previous != NULL){
        mirror_board (list_element->board, mirrored_board);
        copy_board (moves_array->board, mirrored_board);
        list_element = list_element->parent;
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
        SDL_Delay (DELAY);
    }
}

//checks to see if board has a peg at target location. Returns 1 if target has peg, otherwise 0
int check_for_target (Board board, int height_coordinate, int width_coordinate){
    for (int i = 0; i < BOARD_HEIGHT; ++i){
        for (int j = 0; j < BOARD_WIDTH; ++j){
            if (board[i][j] == alive && i == height_coordinate && j == width_coordinate){
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
