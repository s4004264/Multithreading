#include <iostream>
#include <pthread.h>
#include <string>
#include <dirent.h>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <cstring>
#include <cstdlib>
//just all imports

class paths { //effectively my own version of a tuple rather than using the c++ library as I am unfamiliar with it
public:
    std::string source;
    std::string destination;
};

void* copy(void* pathptr) { //function for copy threads to call
    paths* path = static_cast<paths*>(pathptr); //get the path and cast it to the proper type
    std::ifstream source(path->source.c_str(), std::ios::binary); //open the source destination for reading in binary mode
    if (!source) { //if there is no source file
        std::cerr << "Error opening source: " << path->source; //give an error message
        return (void*)1; //return 1
    }

    std::ofstream destination(path->destination.c_str(), std::ios::binary); //get the output directory in binary mode
    if (!destination) { //if there is no destination 
        std::cerr << "Error opening destination: " << path->destination; //give an error message
        return (void*)1; //return 1
    }

    try { //try catch for errors 
        destination << source.rdbuf(); //write the buffer to the destination
    }
    catch (const std::exception& e) { //catch an error
        std::cerr << "Error writing to destination: " << path->destination; //give an error message
        return (void*)1; //return 1
    }

    return (void*)0; //return 0
}

int main(int argc, char* argv[]) {
    if (argc != 4) { //give an error if the wrong argument structure is provided
        std::cout << "Please use the format: ./filename <number of thread> <source directory> <destination directory>";
        return 1;
    }

    int n = atoi(argv[1]); //convert the n from a string to an int

    if (n <= 0 || n > 10) { //range check for n
        std::cout << "Please choose a positive integer up to 10";
        return 1;
    }

    std::string source = argv[2]; //get the source directory
    std::string destination = argv[3]; //get the destination directory

    DIR* dir = opendir(source.c_str()); //open the source directory
    if (!dir) { //if there is no source
        std::cerr << "Please enter a valid source directory"; //give an error message
        return 1; //return 1
    }

    struct stat st; //create a file structure struct
    if (stat(destination.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) { //check if the destination exists
        std::cerr << "Please enter a valid destination directory"; // give an error message
        closedir(dir); //close the source
        return 1; //return 1
    }

    std::vector<pthread_t> threadList(n); //create a list of threads
    std::vector<paths> copyPaths(n); //create a list of paths

    struct dirent* entry; //create a directory for iteration
    int i = 0; //create a counter for iterating
    while ((entry = readdir(dir)) != NULL) { //loop through each file in source
        if (i >= n) { //if i is larger than the number of files end loop
            break;
        }
        if (entry->d_type == DT_REG) { //check if the current directory is a regular file
            copyPaths[i].source = source + "/" + entry->d_name; //create the source file path
            copyPaths[i].destination = destination + "/" + entry->d_name; //create the destination file path

            int thread = pthread_create(&threadList[i], NULL, copy, &copyPaths[i]); //create a thread for copying
            if (thread != 0) { //if thread creation fails
                std::cerr << "Thread creation failed"; //give an error message
                closedir(dir); //close the source directory
                return 1; //return 1
            }
            i++; //increment i
        }
    }

    closedir(dir); //close source

    for (int j = 0; j < i; j++) { //loop for each thread
        pthread_join(threadList[j], NULL); //join the given thread
    }

    std::cout << "num arguments: " << argc << "\nnum threads: " << n << "\nsource directory: " << source << "\ndestination directory: " << destination;
    return 0; //return 0
}
