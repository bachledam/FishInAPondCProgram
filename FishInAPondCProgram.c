#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "leak_detector_c.h"

typedef struct failfish{
int sequence_number;
struct failfish *next;
struct failfish *prev;
} failfish;

typedef struct pond {
int g;
char *name;
int n;
int e;
int th;
failfish *head;
} pond;
int is_empty(failfish *fish){ //checks if the place you want to insert is empty
    if (fish == NULL){
        return 1;
    }
    else {
        return 0;
    }
}

failfish *dequeue (failfish *remove, failfish *head) //takes the circular linked list apart into an array and removes fish from the front for when there is no nodes, one node, two nodes, more than two nodes
{
    if (is_empty(head)){ //empty list
        return head;
   }
    else { //one node
        failfish *temp = remove;
        if (is_empty(head->next)){
            free(head);
            return NULL;
        }
        else if (head->next->next == head){ //two nodes
            if (remove == head){
                head = head->next;
                remove->next->next = NULL;
                remove->prev->next = NULL;
                free(remove);
                return head;
            }
            else {
                head->next = NULL;
                head->prev = NULL;
                free(remove);
                return head;
            }
        }
        else { //more than two nodes
            if (remove == head){
                head = head->next;
            }
            remove->prev->next = remove->next;
            remove->next->prev = remove->prev;
            remove = remove->next;
            free(temp);
            return head;
        }
   }
}

failfish *enqueue (failfish *fish, failfish *head) //places fish into the list in the back for when there is no nodes, one node, more than two nodes
{
    if(is_empty(head)) //empty list
    {

        head = fish;
    }
    else{ //one node
        if (is_empty(head->prev)){

            fish->next = head;
            fish->prev = head;
            head->prev = fish;
            head->next = fish;
        }
        else{ //more than one node
            fish->prev = head->prev;
            head->prev->next = fish;
            fish->next = head;
            head->prev = fish;
        }
    }
    return head;
}

void destructor (pond **pond_list, int ponds_num){ //Frees pondlist name, elements, and whole list
    for (int i = 0; i < ponds_num; i++)
    {
        free(pond_list[i]->name);
        free(pond_list[i]);
    }
    free(pond_list);
}
failfish *create_failfish(int sequence_number){ //creates one failfish circular list
    failfish *fish;
    fish = malloc(sizeof(failfish));
    fish->sequence_number = sequence_number;
    fish->next = NULL;
    fish->prev = NULL;

    return fish;
}

void first_cycle (pond *pond, FILE *ofp){ //created temp value to keep track of head and then uses th and e to delete the appropriate node
    failfish *temp = pond->head;
    failfish *remove;
    for (int i = pond->n; i > (pond->th); i--){ // removes until threshold met
        remove = temp;

        for (int j = 0; j < (pond->e) - 1; j++){ //counts how many to skip before it removes
            remove = remove->next;
        }

        temp = remove->next; //removes the failfish
        fprintf(ofp, "Failfish %d eaten\n", remove->sequence_number);
        pond->head = dequeue(remove, pond->head);

        failfish *current = pond->head;
    }
}
void second_cycle (pond **pond_list, int pond_num, FILE *ofp){ //unwinds circles into queues and then searches each pond for smallest fish and deletes it
    int max = -1;
    int index;
    int total = 0;
    failfish *temp;

    for (int k = 0; k < pond_num; k++){ //continues for each pond
        total = total + pond_list[k]->th;
    }
    fprintf(ofp, "\nSecond Course\n\n");
    while (total > 1){
        for (int i = 0; i < pond_num; i++){ //finds the right fish
            if (!is_empty(pond_list[i]->head)){
                if (pond_list[i]->head->sequence_number > max){
                    max = pond_list[i]->head->sequence_number;
                    index = i;
                }
            }
        }
        fprintf(ofp, "Eaten: Failfish %d from pond %d\n", pond_list[index]->head->sequence_number, pond_list[index]->g); //removes the fish from pond
        pond_list[index]->head = dequeue(pond_list[index]->head, pond_list[index]->head);
        total = total - 1;
        max = -1;
    }


    fprintf(ofp, " \n");
    for (int m = 0; m < pond_num; m++){
        if (pond_list[m]->head != NULL){
            fprintf(ofp, "Failfish %d from pond %d remains\n", pond_list[m]->head->sequence_number, pond_list[m]->g); //prints remaining fish
            free(pond_list[m]->head);
            break;
        }
    }
}

void sort_ponds_list (int ponds_num, pond **pond_list){ //sorts the ponds based off pond number (g)
    pond *temp = NULL;
    for (int i = 0; i < ponds_num - 1; i++){
        for (int j = i + 1; j < ponds_num; j++){
            if (pond_list[i]->g > pond_list[j]->g){
                temp = pond_list[i];
                pond_list[i] = pond_list[j];
                pond_list[j] = temp;
            }
        }
    }
}

void print_end_stats (pond **pond_list, int ponds_num, FILE *ofp){ //prints the end of course pond status with all nodes in each pond that remain
    failfish *temp;
    fprintf(ofp, "End of Course Pond Status\n");
    for (int i = 0; i < ponds_num; i++){
        fprintf(ofp, "%d %s ", pond_list[i]->g, pond_list[i]->name);
        temp = pond_list[i]->head;
        for (int k = 0; k < pond_list[i]->th; k++){
            fprintf(ofp, "%d ", temp->sequence_number);
            temp = temp->next;
        }
        fprintf(ofp, " \n");
    }
}
void print_initial_stats (pond **pond_list, int ponds_num, FILE *ofp){ //prints the initial pond with how many fish are in each pond
    failfish *temp;
    fprintf(ofp, "Initial Pond Status\n");
    for (int i = 0; i < ponds_num; i++){
        fprintf(ofp, "%d %s ", pond_list[i]->g, pond_list[i]->name);
        temp = pond_list[i]->head;
        for (int k = 0; k < pond_list[i]->n; k++){
            fprintf(ofp, "%d ", temp->sequence_number);
            temp = temp->next;
        }
        fprintf(ofp, " \n");
    }
}


void read_ponds(FILE *ifp, pond **pond_list, int pond_num){ //scans the input file and stores each pond and contents in its own list
    int g;
    char nameTemp[25]; //have to consider how long names could be
    int n, e, th; //consider how long element name could be
    failfish *fishTemp;

    //populate the head node
    for(int i = 0; i < pond_num; i++){

        //goes through each monster line
        pond_list[i] = malloc(sizeof(pond));
        fscanf(ifp, "%d %s %d %d %d\n", &g, nameTemp, &n, &e, &th); //SCANS LINES OF FILE
        pond_list[i]->g = g;
        pond_list[i]->name = malloc((strlen(nameTemp) + 1) * sizeof(char)); //MAKES SPACE FOR THE NAME TO BE STORED
        strcpy(pond_list[i]->name, nameTemp); //COPIES THE NAME TO THE LIST
        pond_list[i]->n = n;
        pond_list[i]->e = e;
        pond_list[i]->th = th;
        pond_list[i]->head = NULL;

        for (int j = 1; j <= pond_list[i]->n; j++){
                fishTemp = create_failfish(j);
                pond_list[i]->head = enqueue (fishTemp, pond_list[i]->head);

        }
    }
}
int main()
{
    atexit(report_mem_leak); //memory leak report
    FILE *ifp = fopen("cop3502-as2-input.txt", "r");
    FILE *ofp = fopen("cop3502-as2-output-bachleda-michal.txt", "w");

    //error check: file read correctly
    if (ifp == NULL){ //DONE
        fprintf(ofp, "failed reading file\n");
        return 0;
    }
    //create pond list
    pond **pond_list;
    int ponds_num, g; //first number read to store
    char trash[10];

    fscanf(ifp, "%d\n", &ponds_num);
    pond_list = malloc(sizeof(pond *) * ponds_num);
    read_ponds(ifp, pond_list, ponds_num);

    sort_ponds_list(ponds_num, pond_list);

    print_initial_stats (pond_list, ponds_num, ofp);

    fprintf(ofp, "\nFirst Course\n\n");
    for (int i = 0; i < ponds_num; i++){
        fprintf(ofp, "Pond %d: %s\n", pond_list[i]->g, pond_list[i]->name);
        first_cycle (pond_list[i], ofp);
        fprintf(ofp, " \n");
    }
    print_end_stats (pond_list,ponds_num, ofp);

    second_cycle (pond_list, ponds_num, ofp);

    destructor (pond_list, ponds_num);

    fclose(ofp);
    fclose(ifp);
    return 0;
}
