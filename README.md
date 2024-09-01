# Multithreading
to run the code first run make 

For mmcopier run ./mmcopier <number of threads> <source_dir> <destination_dir> (create these two directorys if they are not already created)
For mscopier run ./mscopier <number of threads> <source file> <destination file> (create the source file if it is not already)

locks and condition variables:
    
    mmcopier.cpp:
    
        no need as the threads do not interact with the same data
    
    mscopier.cpp:
    
    line 23: mutex lock (enter the critical zone)

    line 25: wait until not full condition (make sure not to run code when the queue is full)

    line 29: broadcast not empty (tell all writer threads that they are free to start writing)

    line 30: unlock mutex (allow other thread to enter the critical zone)
    
    line 34: signal not empty (tell one writer thread to start writing)
    
    line 35: unlock mutex (allow other thread to enter the critical zone)
    
    line 44: mutex lock (enter the critical zone)
    
    line 47: unlock mutex (exit the critical zone)
    
    line 50: wait until not empty condition (make sure not to attept to write when there are no lines in the queue)
    
    line 55: signal not full (tell reader thread they can start reading again)
    
    line 56: unlock mutex (allow other thread to enter the critical zone)