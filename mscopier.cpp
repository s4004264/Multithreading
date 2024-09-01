#include <iostream>
#include <pthread.h>
#include <queue>
#include <string>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <cstdlib>
//just all imports

std::queue<std::string> lines; //create a queue of lines to write
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //initialise the mutex
pthread_cond_t notFull = PTHREAD_COND_INITIALIZER; //initialise the notFull condition
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER; //initialise the notEmpty condition
long unsigned int threadCount; //initialise a count for the total number of threads
bool finished = false; //initialise the finished flag to false


void *reader(void *source){ //create a function for the reader threads
    std::ifstream *file = static_cast<std::ifstream*>(source); //cast the parsed source to the correct type
    std::string line; //create a temporary line variable
    while(!finished){ //while the finished flag is false
        pthread_mutex_lock(&mutex); //enter the critical zone
        while(lines.size() == threadCount){ //while the size of the lines queue is the number of threads
            pthread_cond_wait(&notFull, &mutex); //wait
        }
        if(!std::getline(*file, line)){ //get a line, if there is not a line to get
            finished = true; //set the finished flag to true
            pthread_cond_broadcast(&notEmpty); //broadcast the notEmpty condition
            pthread_mutex_unlock(&mutex); //exit the critical zone
            break;//break out of the look
        }
        lines.push(line); //push the line to the list of lines
        pthread_cond_signal(&notEmpty); //signal the notEmpty condition for one thread to start attempting to write
        pthread_mutex_unlock(&mutex); // exit the critical zone
    }

    return NULL;
}

void *writer(void *destination){
    std::ofstream* writeFile = static_cast<std::ofstream*>(destination); //cast the parsed destination to the correct type
    while(true){ //forever loop
        pthread_mutex_lock(&mutex); //enter the critical zone
        while(lines.empty()){ //loop as long as the global lines queue is empty
            if(finished){ //check if the finished flag is set
                pthread_mutex_unlock(&mutex); //exit the critical zone
                return NULL; //exit the infinite loop
            }
            pthread_cond_wait(&notEmpty, &mutex); //wait until the not empty signal
        }
        std::string writeLine= lines.front(); //get the line from the front of the queue
        lines.pop(); //remove that line from the queue
        *writeFile << writeLine << std::endl; //write the line to the file
        pthread_cond_signal(&notFull); //signal the not full condition
        pthread_mutex_unlock(&mutex); //exit the critical zone
    }
    return NULL; //return
}



int main(int argc, char* argv[]){
    if (argc != 4){ //check if the number of arguments is correct
        std::cout << "Please use the format: ./filename <number of thread> <source directory> <destination directory>";
        return 1;
    }
    int n = atoi(argv[1]); //convert the number of threads from a string to an int

    if (n < 2 || n > 10){ //check the range of n
        std::cout << "Please choose a positive integer between 2 and 10";
        return 1;
    }
    threadCount = n; //set the threadcount to n if all checks pass
    std::ifstream source(argv[2]); //create an input stream from the source
    if(!source){ //if there is no source
        std::cerr << "Error opening source"; //give an error message
        return 1; //return 1
    }
    
    std::ofstream destination(argv[3]); //create an output stream
    if(!destination){ //if there is a problem 
        std::cerr << "Error opening destination"; //give an error message
        return 1; //return 1
    }

    std::vector<pthread_t> read(n); //create a list of reader threads
    std::vector<pthread_t> write(n); //create a list of writer threads

    for (int i = 0; i < n; i++){ //loop for the number of threads given in args
        pthread_create(&read.at(i), NULL, reader, &source); //create reader
        pthread_create(&write.at(i), NULL, writer, &destination); //create writer
    }

    for (int i = 0; i < n; i++){ //loop for number of threads given in args
        pthread_join(read.at(i), NULL); //join reader
        pthread_join(write.at(i), NULL); //join writer
    }

    std::cout << "num arguments: " << argc << "\nnum threads: " << n << "\nsource file: " << argv[2] << "\ndestination file: " << argv[3];
    source.close(); //close source
    destination.close(); //close destination
    return 0;
}