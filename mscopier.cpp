#include <iostream>
#include <pthread.h>
#include <queue>
#include <string>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <cstdlib>

std::queue<std::string> lines;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;
long unsigned int threadCount;
bool finished = false;


void *reader(void *source){
    std::ifstream *file = static_cast<std::ifstream*>(source);
    std::string line;
    while(!finished){
        pthread_mutex_lock(&mutex); //enter the critical zone
        while(lines.size() == threadCount){
            pthread_cond_wait(&notFull, &mutex);
        }
        if(!std::getline(*file, line)){
            finished = true;
            pthread_cond_broadcast(&notEmpty);
            pthread_mutex_unlock(&mutex); //exit the critical zone
            break;
        }
        lines.push(line);
        pthread_cond_signal(&notEmpty);
        pthread_mutex_unlock(&mutex); // exit the critical zone
    }

    return NULL;
}

void *writer(void *destination){
    std::ofstream* writeFile = static_cast<std::ofstream*>(destination);
    while(true){
        pthread_mutex_lock(&mutex); //enter the critical zone
        while(lines.empty()){
            if(finished){
                pthread_mutex_unlock(&mutex); //exit the critical zone
                return NULL; //exit the infinite loop
            }
            pthread_cond_wait(&notEmpty, &mutex);
        }
        std::string writeLine= lines.front();
        lines.pop();
        *writeFile << writeLine << std::endl;
        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&mutex); //exit the critical zone
    }
    return NULL;
}



int main(int argc, char* argv[]){
    if (argc != 4){
        std::cout << "Please use the format: ./filename <number of thread> <source directory> <destination directory>";
        return 1;
    }
    int n = atoi(argv[1]);

    if (n < 2 || n > 10){
        std::cout << "Please choose a positive integer between 2 and 10";
        return 1;
    }
    threadCount = n;
    std::ifstream source(argv[2]);
    if(!source){
        std::cerr << "Error opening source";
        return 1;
    }
    
    std::ofstream destination(argv[3]);
    if(!destination){
        std::cerr << "Error opening destination";
        return 1;
    }

    std::vector<pthread_t> read(n);
    std::vector<pthread_t> write(n);

    for (int i = 0; i < n; i++){
        pthread_create(&read.at(i), NULL, reader, &source);
        pthread_create(&write.at(i), NULL, writer, &destination);
    }

    for (int i = 0; i < n; i++){
        pthread_join(read.at(i), NULL);
        pthread_join(write.at(i), NULL);
    }

    std::cout << "num arguments: " << argc << "\nnum threads: " << n << "\nsource file: " << argv[2] << "\ndestination file: " << argv[3];
    source.close();
    destination.close();
    return 0;
}