typedef struct {
    int NZ; //number of clients
    int NU; //number of workers
    int TZ; //Time in ms where client waiting to go for post
    int TU; //worker rest, in ms
    int F; //in ms, time after that post office will be closed  
}data_t;
typedef enum {OPEN, CLOSED} PostStatus;