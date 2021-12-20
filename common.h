struct capList {
    char fname[20];  // object

    // operation
    int read;
    int write;
};

struct fileInfo {
    char fname[20];  // file name
    char owner[20];  // owner name
    int rcount;      // read lock
    int wcount;      // write lock
};

#define portNum 7894
