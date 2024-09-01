#include <iostream>
#include <pthread.h>
#include <string>
#include <dirent.h>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <cstring>
#include <cstdlib>

class paths {
public:
    std::string source;
    std::string destination;
};

void* copy(void* pathptr) {
    paths* path = static_cast<paths*>(pathptr);
    std::ifstream source(path->source.c_str(), std::ios::binary);
    if (!source) {
        std::cerr << "Error opening source: " << path->source;
        return (void*)1;
    }

    std::ofstream destination(path->destination.c_str(), std::ios::binary);
    if (!destination) {
        std::cerr << "Error opening destination: " << path->destination;
        return (void*)1;
    }

    try {
        destination << source.rdbuf();
    }
    catch (const std::exception& e) {
        std::cerr << "Error writing to destination: " << path->destination;
        return (void*)1;
    }

    return (void*)0;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Please use the format: ./filename <number of thread> <source directory> <destination directory>";
        return 1;
    }

    int n = atoi(argv[1]);

    if (n <= 0 || n > 10) {
        std::cout << "Please choose a positive integer up to 10";
        return 1;
    }

    std::string source = argv[2];
    std::string destination = argv[3];

    DIR* dir = opendir(source.c_str());
    if (!dir) {
        std::cerr << "Please enter a valid source directory";
        return 1;
    }

    struct stat st;
    if (stat(destination.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        std::cerr << "Please enter a valid destination directory";
        closedir(dir);
        return 1;
    }

    std::vector<pthread_t> threadList(n);
    std::vector<paths> copyPaths(n);

    struct dirent* entry;
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (i >= n) {
            break;
        }
        if (entry->d_type == DT_REG) {
            copyPaths[i].source = source + "/" + entry->d_name;
            copyPaths[i].destination = destination + "/" + entry->d_name;

            int thread = pthread_create(&threadList[i], NULL, copy, &copyPaths[i]);
            if (thread != 0) {
                std::cerr << "Thread creation failed";
                closedir(dir);
                return 1;
            }
            i++;
        }
    }

    closedir(dir);

    for (int j = 0; j < i; j++) {
        pthread_join(threadList[j], NULL);
    }

    std::cout << "num arguments: " << argc << "\nnum threads: " << n << "\nsource directory: " << source << "\ndestination directory: " << destination;
    return 0;
}
