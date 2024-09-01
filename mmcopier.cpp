#include <iostream>
#include <pthread.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>

class paths{ //effectively a tuple replacement containing a source and a destination
    public:
        std::filesystem::path source;
        std::filesystem::path destination;
};

void* copy(void* pathptr){
    paths* path = static_cast<paths*>(pathptr);
    std::ifstream source(path->source);
    if(!source){
        std::cerr << "Error opening source: " << path->source;
        return (void*)1;
    }

    std::ofstream destination(path->destination);
    if(!destination){
        std::cerr << "Error opening destination: " << path->destination;
        return (void*)1;
    }

    try
    {      
        destination << source.rdbuf();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error writing to destination: " << path->destination;
        return (void*)1;
    }

    return (void*)0;
}



int main(int argc, char* argv[]){
    if (argc != 4){
        std::cout << "Please use the format: ./filename <number of thread> <source directory> <destination directory>";
        return 1;
    }
    int n = atoi(argv[1]);

    if (n < 0 || n > 10){
        std::cout << "Please choose a positive integer up to 10";
        return 1;
    }
    std::filesystem::path source = argv[2];
    std::filesystem::path destination = argv[3];

    if(!(std::filesystem::exists(source) && std::filesystem::is_directory(source))){
        std::cout << "Please enter a valid source directory";
        return 1;
    }

    if(!std::filesystem::exists(destination)){
        std::cout << "Please enter a valid destination directory";
        return 1;
    }


    std::vector <pthread_t> threadList(n);
    std::vector <paths> copyPaths(n);

    int i = 0;
        for(const std::filesystem::__cxx11::directory_entry& file : std::filesystem::directory_iterator(source)){
            if (i >= n){
                break;
            }
            if(file.is_regular_file()){
                copyPaths.at(i).source = file.path();
                copyPaths.at(i).destination = destination / file.path().filename();

                int thread = pthread_create(&threadList.at(i), nullptr, copy, &copyPaths.at(i));
                if (thread != 0){
                    std::cerr << "Thread failed";
                    return 1;
                }
            }
            i++;
        }

    for(int j = 0; j < i; j++){
        pthread_join(threadList.at(j), nullptr);
    }

    std::cout << "num arguments: " << argc << "\nnum threads: " << n << "\nsource directory: " << source << "\ndestination directory: " << destination;
    return 0;
}

